#ifndef __WORKQLATENCY_H
#define __WORKQLATENCY_H

#define DEF_TIME	10 /* s */
#define MAX_DATE	128
#define MAX_BUF		256
#define MAX_SYMS	300000
#define MAX_KWORKNAME	128
#define SEC_TO_NS   (1000*1000*1000)
#define SEC_TO_MS   (1000*1000)
#define LAT_THRESH_NS	(10*1000*1000)
#define NR_THREADS 	2

enum kwork_class_type {
	KWORK_CLASS_IRQ,
	KWORK_CLASS_SOFTIRQ,
	KWORK_CLASS_WORKQUEUE,
	KWORK_CLASS_MAX,
};

struct args {
	__u64 thresh;
};

enum trace_class_type {
	TRACE_RUNTIME,
	TRACE_LATENCY,
	RACE_CLASS_MAX,
};

struct ksym {
	long addr;
	char *name;
};

struct work_key {
	__u32 type;
	__u32 cpu;
	__u64 id;
};

struct report_data {
	__u64 nr;
	__u64 cpuid;
	__u64 total_time;
	__u64 max_time;
	__u64 max_time_start;
	__u64 max_time_end;
	__u64 name_addr;
};

struct thread_args {
	enum kwork_class_type type;
	void *skel;
};
#endif