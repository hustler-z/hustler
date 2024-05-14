#define CPU_ARRY_LEN	4
#define TASK_COMM_LEN	16
#define CONID_LEN	13

struct event {
	__u32 ret, pid, cpu;
	__u64 delay, stamp, enter, exit;
	char comm[TASK_COMM_LEN];
};

struct jit_maxN {
	__u64 delay;
	__u64 stamp;
	int cpu, pid;
	char comm[TASK_COMM_LEN];
};

struct jit_lastN {
	int cpu;
	__u64 delay;
	char con[CONID_LEN];
};

struct sched_jit_summary {
	int min_idx;
	unsigned long num;
	__u64 total, topNmin;
	struct jit_maxN maxN_array[CPU_ARRY_LEN]; 
	struct jit_lastN lastN_array[CPU_ARRY_LEN]; 
	unsigned long less10ms, less50ms, less100ms, less500ms, less1s,plus1s;
};
