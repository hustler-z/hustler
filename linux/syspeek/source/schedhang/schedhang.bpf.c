#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "sched_jit.h"
#include "schedhang.h"

#define BPF_F_FAST_STACK_CMP	(1ULL << 9)
#define KERN_STACKID_FLAGS	    (0 | BPF_F_FAST_STACK_CMP)

#define BIT_WORD(nr)	((nr) / BITS_PER_LONG)
#define BITS_PER_LONG	64
#define _(P) ({typeof(P) val = 0; bpf_probe_read(&val, sizeof(val), &P); val;})

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(key_size, sizeof(int));
    __uint(value_size, sizeof(struct args));
    __uint(max_entries, 1);
} args_map SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_STACK_TRACE);
	__uint(key_size, sizeof(u32));
	__uint(value_size, PERF_MAX_STACK_DEPTH * sizeof(u64));
	__uint(max_entries, 1000);
} stackmap SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_PERCPU_HASH);
	__uint(max_entries, MAX_MONI_NR);
	__type(key, u64);
	__type(value, struct latinfo);
} info_map SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
	__uint(key_size, sizeof(u32));
	__uint(value_size, sizeof(u32));
} events SEC(".maps");

char LICENSE[] SEC("license") = "Dual BSD/GPL";

static inline int test_bit(int nr, const volatile unsigned long *addr)
{
        return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
}

static inline int test_ti_thread_flag(struct thread_info *ti, int nr)
{
	int result;
	unsigned long *addr;
	unsigned long tmp = _(ti->flags);

	addr = &tmp;
	result = 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
	return result;
}

static inline int test_tsk_thread_flag_low(struct task_struct *tsk, int flag)
{
	struct thread_info *tfp;

	tfp = (struct thread_info *)(BPF_CORE_READ(tsk, stack));
	return test_ti_thread_flag(tfp, flag);
}

/*
 * Note: This is based on
 *   1) ->thread_info is always be the first element of task_struct if CONFIG_THREAD_INFO_IN_TASK=y
 *   2) ->state now is the most nearly begin of task_struct except ->thread_info if it has.
 * return ture if struct thread_info is in task_struct */
static bool test_THREAD_INFO_IN_TASK(struct task_struct *p)
{
#if 0
	volatile long *pstate;
	size_t len;

	pstate = &(p->state);

	len = (u64)pstate - (u64)p;
	return (len == sizeof(struct thread_info));
	return bpf_core_task_struct_thread_info_exist(p);
#endif
    return true;
}

static inline int test_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	struct thread_info *tfp;

	tfp = (struct thread_info *)tsk;
	return test_ti_thread_flag(tfp, flag);
}

static inline int test_tsk_need_resched(struct task_struct *tsk, int flag)
{
	if (test_THREAD_INFO_IN_TASK(tsk))
		return test_tsk_thread_flag(tsk, flag);
	else
		return test_tsk_thread_flag_low(tsk, flag);
}

SEC("kprobe/account_process_tick")
int BPF_KPROBE(account_process_tick, struct task_struct *p, int user_tick)
{
	int args_key;
	u64 cpuid;
	u64 resched_latency, now;
	struct latinfo lati, *latp;
	struct args args, *argsp;

	__builtin_memset(&args_key, 0, sizeof(int));
	argsp = bpf_map_lookup_elem(&args_map, &args_key);
	if (!argsp)
		return 0;

	if (_(p->pid) == 0)
		return 0;

	if(!test_tsk_need_resched(p, _(argsp->flag)))
		return 0;

	now = bpf_ktime_get_ns();
	__builtin_memset(&cpuid, 0, sizeof(u64));
	cpuid = bpf_get_smp_processor_id();
	latp = bpf_map_lookup_elem(&info_map, &cpuid);
	if (latp) {
		if (!latp->last_seen_need_resched_ns) {
			latp->last_seen_need_resched_ns = now;
			latp->ticks_without_resched = 0;
			latp->last_perf_event = now;
		} else {
			latp->ticks_without_resched++;
			resched_latency = now - latp->last_perf_event;
			if (resched_latency > _(argsp->thresh)) {
				struct event event = {0};
				event.stamp = latp->last_seen_need_resched_ns;
				event.cpu = cpuid;
				event.delay = now - latp->last_seen_need_resched_ns;
				event.pid = bpf_get_current_pid_tgid();
				bpf_get_current_comm(&event.comm, sizeof(event.comm));
				event.ret = bpf_get_stackid(ctx, &stackmap, KERN_STACKID_FLAGS);
				latp->last_perf_event = now;
				if (latp->recorded == 0) {
					bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
							&event, sizeof(event));
					latp->recorded = 1;
				}
			}
		}
	} else {
		__builtin_memset(&lati, 0, sizeof(struct latinfo));
		lati.last_seen_need_resched_ns = now;
		lati.last_perf_event = now;
		bpf_map_update_elem(&info_map, &cpuid, &lati, BPF_ANY);
	}

	return 0;
}

/*
struct trace_event_raw_sched_switch {
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
 */
SEC("tp/sched/sched_switch")
int handle_switch(struct trace_event_raw_sched_switch *ctx)
{
	int args_key;
	u64 cpuid;
	struct latinfo lati, *latp;
	struct args *argp;

	__builtin_memset(&args_key, 0, sizeof(int));
	argp = bpf_map_lookup_elem(&args_map, &args_key);
	if (!argp)
		return 0;

	cpuid = bpf_get_smp_processor_id();
	latp = bpf_map_lookup_elem(&info_map, &cpuid);
	if (latp) {
		u64 now;
		struct event event = {0};

		now = bpf_ktime_get_ns();
		event.enter = latp->last_seen_need_resched_ns;
		if (argp->thresh && event.enter &&
			(now - event.enter > argp->thresh)) {
			event.stamp = now;
			event.exit = now;
			event.cpu = cpuid;
			event.delay = now - latp->last_seen_need_resched_ns;
			latp->last_perf_event = now;
			event.pid = bpf_get_current_pid_tgid();
			bpf_get_current_comm(&event.comm, sizeof(event.comm));
			event.ret = bpf_get_stackid(ctx, &stackmap, KERN_STACKID_FLAGS);
			bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
					&event, sizeof(event));
		}
		latp->last_seen_need_resched_ns = 0;
	}

	return 0;
}
