#ifndef __OFFCPUTIME_H
#define __OFFCPUTIME_H

#define TASK_COMM_LEN		16

struct bpfarg {
	bool kernel_threads_only;
	bool user_threads_only;
	__u64 max_block_ns;
	__u64 min_block_ns;
	pid_t targ_tgid;
	pid_t targ_pid;
	long state;
};

struct key_t {
	__u32 pid;
	__u32 tgid;
	int user_stack_id;
	int kern_stack_id;
};

struct val_t {
	__u64 delta;
	char comm[TASK_COMM_LEN];
};

#endif /* __OFFCPUTIME_H */
