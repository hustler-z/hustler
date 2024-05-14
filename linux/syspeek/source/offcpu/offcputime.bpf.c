#define BPF_NO_GLOBAL_DATA
#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>
#include "offcputime.h"

//#define BPF_F_FAST_STACK_CMP    (1ULL << 9)
#define KERN_STACKID_FLAGS      (0 | BPF_F_FAST_STACK_CMP)
#define PF_KTHREAD		0x00200000	/* I am a kernel thread */
#define MAX_ENTRIES		10240

struct internal_key {
	u64 start_ts;
	struct key_t key;
};

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__uint(max_entries, 1);
	__type(key, u32);
	__type(value, struct bpfarg);
} argmap SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key, u32);
	__type(value, struct internal_key);
	__uint(max_entries, MAX_ENTRIES);
} start SEC(".maps");

#define PERF_MAX_STACK_DEPTH 127
#if 0
struct {
	__uint(type, BPF_MAP_TYPE_STACK_TRACE);
	__uint(key_size, sizeof(u32));
	__uint(value_size, PERF_MAX_STACK_DEPTH * sizeof(u64));
	__uint(max_entries, MAX_ENTRIES);
} stackmap SEC(".maps");
#endif
struct bpf_map_def SEC("maps") stackmap = {
        .type = BPF_MAP_TYPE_STACK_TRACE,
        .key_size = sizeof(u32),
        .value_size = PERF_MAX_STACK_DEPTH * sizeof(u64),
        .max_entries = 10000,
};


struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key, struct key_t);
	__type(value, struct val_t);
	__uint(max_entries, MAX_ENTRIES);
} info SEC(".maps");

#if 0
static __s64 get_task_state(void *task)
{
	struct task_struct___x *t = task;

	if (bpf_core_field_exists(t->__state))
		return t->__state;
	return ((struct task_struct *)task)->state;
}
#endif

#define _(P) ({typeof(P) val = 0; bpf_probe_read(&val, sizeof(val), &P); val;})
/*
 * the return value type can only be assigned to 0,
 * so it can be int ,long , long long and the unsinged version
 * */
#define GETARG_FROM_ARRYMAP(map,argp,type,member,oldv)({	\
	type retval = (type)oldv;			\
	int i = 0;				\
	argp = bpf_map_lookup_elem(&map, &i);	\
	if (argp) {				\
		retval = _(argp->member);		\
	}					\
	retval;					\
	})


static bool allow_record(struct task_struct *t)
{
	bool kernel_threads_only;
	bool user_threads_only;
	pid_t targ_tgid;
	pid_t targ_pid;
	long state;
	struct bpfarg *argp;

	targ_tgid = GETARG_FROM_ARRYMAP(argmap, argp, pid_t, targ_tgid, -1);
	targ_pid = GETARG_FROM_ARRYMAP(argmap, argp, pid_t, targ_pid, -1);
	user_threads_only = GETARG_FROM_ARRYMAP(argmap, argp, bool, user_threads_only, false);
	kernel_threads_only = GETARG_FROM_ARRYMAP(argmap, argp, bool, kernel_threads_only, false);
	state = GETARG_FROM_ARRYMAP(argmap, argp, long, state, -1);

	if (targ_tgid != -1 && targ_tgid != _(t->tgid))
		return false;
	if (targ_pid != -1 && targ_pid != _(t->pid))
		return false;
	if (user_threads_only && _(t->flags) & PF_KTHREAD)
		return false;
	else if (kernel_threads_only && !(_(t->flags) & PF_KTHREAD))
		return false;
	//if (state != -1 && bpf_core_task_struct_state(t) != state)
	//if (state != -1 && (_(t->__state) != state))
	//	return false;
	return true;
}

struct sched_switch_tp_args {
	struct trace_entry ent;
	char prev_comm[16];
	pid_t prev_pid;
	int prev_prio;
	long int prev_state;
	char next_comm[16];
	pid_t next_pid;
	int next_prio;
	char __data[0];
};

//SEC("raw_tracepoint/sched_switch")
//int raw_tp__sched_switch(struct bpf_raw_tracepoint_args *ctx)
SEC("tp/sched/sched_switch")
int tp__sched_switch(struct sched_switch_tp_args *ctx)
{
	struct task_struct *prev = (void *)bpf_get_current_task();
	//struct task_struct *next = (void *)ctx->args[2];
	struct internal_key *i_keyp, i_key;
	struct val_t *valp, val;
	s64 delta;
	u32 pid;
	__u64 max_block_ns = -1;
	__u64 min_block_ns = 1;
	struct bpfarg *argp;

	max_block_ns = GETARG_FROM_ARRYMAP(argmap, argp, __u64, max_block_ns, -1);
	min_block_ns = GETARG_FROM_ARRYMAP(argmap, argp, __u64, min_block_ns, 1);

	if (allow_record(prev)) {
		pid = _(prev->pid);
		/* To distinguish idle threads of different cores */
		if (pid == 0)
			pid = bpf_get_smp_processor_id();
		i_key.key.pid = pid;
		i_key.key.tgid = _(prev->tgid);
		i_key.start_ts = bpf_ktime_get_ns();
		if (_(prev->flags) & PF_KTHREAD)
			i_key.key.user_stack_id = -1;
		else
			i_key.key.user_stack_id =
				bpf_get_stackid(ctx, &stackmap,
						BPF_F_USER_STACK);
		/* There have be a bug in linux-4.19 for bpf_get_stackid in raw_tracepoint */
		i_key.key.kern_stack_id = bpf_get_stackid(ctx, &stackmap, KERN_STACKID_FLAGS);
		bpf_map_update_elem(&start, &pid, &i_key, 0);
		__builtin_memset(&val, 0, sizeof(val));
		bpf_probe_read_str(&val.comm, sizeof(prev->comm), prev->comm);
		val.delta = 0;
		bpf_map_update_elem(&info, &i_key.key, &val, BPF_NOEXIST);
	}

	//pid = _(next->pid);
	pid = ctx->next_pid;
	i_keyp = bpf_map_lookup_elem(&start, &pid);
	if (!i_keyp)
		return 0;
	delta = (s64)(bpf_ktime_get_ns() - i_keyp->start_ts);
	if (delta < 0)
		goto cleanup;
	delta = (u64)delta/1000U;
	if (delta < min_block_ns || delta > max_block_ns)
		goto cleanup;
	valp = bpf_map_lookup_elem(&info, &i_keyp->key);
	if (!valp)
		goto cleanup;
	__sync_fetch_and_add(&valp->delta, delta);

cleanup:
	bpf_map_delete_elem(&start, &pid);
	return 0;
}

char LICENSE[] SEC("license") = "GPL";
