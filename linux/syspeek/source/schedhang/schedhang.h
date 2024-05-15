#define BPF_ANY	0
#define MAX_MONI_NR	1024

/* latency thresh:10ms*/
#define LAT_THRESH_NS	(10*1000*1000)
#define TASK_COMM_LEN	16
#define PERF_MAX_STACK_DEPTH	32

#ifdef __x86_64__
#define	TIF_NEED_RESCHED	3
#elif defined (__aarch64__)
#define TIF_NEED_RESCHED	1
#endif

struct args {
	int flag;
	__u64 thresh;
};

struct ksym {
	long addr;
	char *name;
};

struct key_t {
	__u32 ret;
};

struct latinfo {
	__u64 last_seen_need_resched_ns;
	__u64 last_perf_event;
	int recorded, ticks_without_resched;
};

