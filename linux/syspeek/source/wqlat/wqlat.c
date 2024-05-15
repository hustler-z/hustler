#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <getopt.h>
#include "wqlat.skel.h"
#include "wqlat.h"

//__u64 period = DEF_TIME;
__u64 g_thresh = LAT_THRESH_NS;
bool verbose = false;
FILE *latency_fd = NULL;
FILE *runtime_fd = NULL;
static struct ksym syms[MAX_SYMS];
static int sym_cnt;
static volatile sig_atomic_t exiting;


char *log_dir = "/var/log/syspeek/wqlat";
char *runtime_data = "/var/log/syspeek/wqlat/runtime.log";
char *latency_data = "/var/log/syspeek/wqlat/latency.log";

struct option longopts[] = {
    //{ "time", no_argument, NULL, 't' },
	{ "threshold", no_argument, NULL, 'l' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0},
};

static void usage(void)
{
	fprintf(stdout,
	        "Usage: wqlat [options] [args]\n"
            "Options:\n"
			"    --threshold/-l		specify the threshold time(ms), default=10ms\n"
            "    --help/-h			help info\n"
            "example: wqlat -t 10  #trace work runtime and latency statistics\n");
	exit(EXIT_FAILURE);
}

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	if (verbose)
		return vfprintf(stderr, format, args);
	else
		return 0;
}

static void sig_int(int signo)
{
	exiting = 1;
}

static void sig_alarm(int signo)
{
	exiting = 1;
}

static void bump_memlock_rlimit(void)
{
	struct rlimit rlim_new = {
		.rlim_cur	= RLIM_INFINITY,
		.rlim_max	= RLIM_INFINITY,
	};

	if (setrlimit(RLIMIT_MEMLOCK, &rlim_new)) {
		fprintf(stderr, "Failed to increase RLIMIT_MEMLOCK limit!\n");
		exit(1);
	}
}

static int ksym_cmp(const void *p1, const void *p2)
{
	return ((struct ksym *)p1)->addr - ((struct ksym *)p2)->addr;
}

int load_kallsyms(void)
{
	FILE *f = fopen("/proc/kallsyms", "r");
	char func[MAX_BUF], buf[MAX_BUF];
	char symbol;
	void *addr;
	int i = 0;

	if (!f)
		return -ENOENT;

	while (!feof(f)) {
		if (!fgets(buf, sizeof(buf), f))
			break;
		if (sscanf(buf, "%p %c %s", &addr, &symbol, func) != 3)
			break;
		if (!addr)
			continue;
		syms[i].addr = (long) addr;
		syms[i].name = strdup(func);
		i++;
	}
	fclose(f);
	sym_cnt = i;
	if (sym_cnt == 0) {
		printf("/proc/kallsyms has no valid address\n");
		return -ENOENT;
	}
	qsort(syms, sym_cnt, sizeof(struct ksym), ksym_cmp);
	return 0;
}

struct ksym *ksym_search(long key)
{
	int start = 0, end = sym_cnt;
	int result;

	/* kallsyms not loaded. return NULL */
	if (sym_cnt <= 0)
		return NULL;

	while (start < end) {
		size_t mid = start + (end - start) / 2;

		result = key - syms[mid].addr;
		if (result < 0)
			end = mid;
		else if (result > 0)
			start = mid + 1;
		else
			return &syms[mid];
	}

	if (start >= 1 && syms[start - 1].addr < key &&
	    key < syms[start].addr)
		/* valid ksym */
		return &syms[start - 1];

	/* out of range. return _stext */
	return &syms[0];
}

static void print_ksym(__u64 addr, char work_name[])
{
	struct ksym *sym;

	if (!addr)
		return;

	sym = ksym_search(addr);
	if (sym)
		snprintf(work_name, MAX_SYMS, "<0x%llx> %s",
             		addr, sym->name);
	else
		snprintf(work_name, MAX_SYMS, "<0x%llx>: missing symbols\n", addr);
}

void stamp_to_date(__u64 stamp, char dt[], int len)
{
	time_t t, diff, last;
	struct tm *tm;
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	last = time(&t);
	if (stamp) {
		diff = ts.tv_sec * SEC_TO_NS + ts.tv_nsec - stamp;
		diff = diff / SEC_TO_NS;
		last = t - diff;
	}
	tm = localtime(&last);
	strftime(dt, len, "%F %H:%M:%S", tm);
}

static int prepare_dictory(char *path)
{
	int ret = 0;

	if (access(path,0) != 0)
        ret = mkdir(path, 0777);

	return ret;
}

void handle_latency_event(void *ctx, int cpu, void *data, __u32 data_sz)
{
	const struct report_data *dp = data;
	char date_start[MAX_DATE], date_end[MAX_DATE];
	char workname[MAX_SYMS] = {0};

	stamp_to_date(dp->max_time_start, date_start,MAX_DATE);
	stamp_to_date(dp->max_time_end, date_end, MAX_DATE);
	print_ksym(dp->name_addr,workname);
	fprintf(latency_fd, "%-48s\t%-10llu\t%-10llu\t%-12llu\t%-12llu\t%s(%-16llu)\t%s(%-16llu)\n", workname, dp->cpuid,
		dp->total_time/dp->nr, dp->nr, dp->max_time, date_start, dp->max_time_start, date_end, dp->max_time_end);
}

void handle_runtime_event(void *ctx, int cpu, void *data, __u32 data_sz)
{
	const struct report_data *dp = data;
	char date_start[MAX_DATE], date_end[MAX_DATE];
	char workname[MAX_SYMS] = {0};

	stamp_to_date(dp->max_time_start, date_start,MAX_DATE);
	stamp_to_date(dp->max_time_end, date_end, MAX_DATE);
	print_ksym(dp->name_addr,workname);
	fprintf(runtime_fd, "%-48s\t%-10llu\t%-10llu\t%-12llu\t%-12llu\t%s(%-16llu)\t%s(%-16llu)\n", workname, dp->cpuid,
		dp->total_time/dp->nr, dp->nr, dp->max_time, date_start, dp->max_time_start, date_end, dp->max_time_end);
}

void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt)
{
	printf("Lost %llu events on CPU #%d!\n", lost_cnt, cpu);
}

int report_handler(int poll_fd, struct wqlat_bpf *skel, FILE *filep, enum trace_class_type type)
{
	int err = 0;
	struct perf_buffer *pb = NULL;
	struct perf_buffer_opts pb_opts = {};

	if (type == TRACE_RUNTIME)
        pb = perf_buffer__new(poll_fd, 64, handle_runtime_event, handle_lost_events, NULL, &pb_opts);
	else
		pb = perf_buffer__new(poll_fd, 64, handle_latency_event, handle_lost_events, NULL, &pb_opts);

	if (!pb) {
		err = -errno;
		fprintf(stderr, "failed to open perf buffer: %d\n", err);
		goto clean_nosched;
	}

	while (!exiting) {
		err = perf_buffer__poll(pb, 100);
		if (err < 0 && err != -EINTR) {
			fprintf(stderr, "error polling perf buffer: %s\n", strerror(-err));
			goto clean_nosched;
		}
		/* reset err to return 0 if exiting */
		err = 0;
	}
clean_nosched:
	perf_buffer__free(pb);
	return err;
}

//static int report_data(struct wqlat_bpf *skel, enum trace_class_type type)
static void *report_data(void *arg)
{
	int map_fd;
	struct thread_args *arg_info;
	struct wqlat_bpf *skel;
	enum trace_class_type type;

	arg_info = (struct thread_args *)arg;
	type = arg_info->type;
	skel = (struct wqlat_bpf *)arg_info->skel;
	if (type == TRACE_RUNTIME){
		if (access(runtime_data,0) != 0){
			runtime_fd = fopen(runtime_data, "a+");
			fprintf(runtime_fd, "%-48s\t%-10s\t%-10s\t%-12s\t%-12s\t%-32s\t%-16s\n", "Kwork Name", "Cpu",
				"Avg runtime(ns)", "Count", "Max runtime(ns)", "Max runtime start(s)", "Max runtime end(s)");
		} else {
			runtime_fd = fopen(runtime_data, "a+");
		}

		map_fd = bpf_map__fd(skel->maps.runtime_events);
		report_handler(map_fd, skel, runtime_fd, type);
		fclose(runtime_fd);
	}else{
		if (access(latency_data,0) != 0){
			latency_fd = fopen(latency_data, "a+");
			fprintf(latency_fd, "%-48s\t%-10s\t%-10s\t%-12s\t%-12s\t%-32s\t%-16s\n", "Kwork Name", "Cpu",
				"Avg delay(ns)", "Count", "Max delay(ns)", "Max delay start(s)", "Max delay end(s)");
		} else {
			latency_fd = fopen(latency_data, "a+");
		}

		map_fd = bpf_map__fd(skel->maps.latency_events);
		report_handler(map_fd, skel, latency_fd, type);
		fclose(latency_fd);

	}
	return 0;
}

int main(int argc, char *argv[])
{
	int opt, i;
	int err = 0, map_fd, args_key = 0;
	pthread_t tid[NR_THREADS];
	struct wqlat_bpf *skel;
	struct args args;
	struct thread_args *targs[NR_THREADS];

	while ((opt = getopt_long(argc, argv, "t:l:hv", longopts, NULL)) != -1) {
		switch (opt) {
		case 'l':
            if (optarg)
                g_thresh = (int)strtoul(optarg, NULL, 10)*1000*1000;
            break;
		case 'h':
            usage();
            break;
		case 'v':
            verbose = true;
            break;
		default:
            printf("must have parameter\n");
            usage();
            break;
        }
	}

	libbpf_set_print(libbpf_print_fn);
	bump_memlock_rlimit();

	prepare_dictory(log_dir);

	err = load_kallsyms();
	if (err) {
		fprintf(stderr, "Failed to load kallsyms\n");
		return err;
	}

	skel = wqlat_bpf__open();
	if (!skel) {
		fprintf(stderr, "Failed to open BPF skeleton\n");
		return 1;
	}

	err = wqlat_bpf__load(skel);
	if (err) {
		fprintf(stderr, "Failed to load BPF skeleton, errno:%d\n",err);
		return 1;
	}
	if (signal(SIGINT, sig_int) == SIG_ERR ||
		signal(SIGALRM, sig_alarm) == SIG_ERR) {
		fprintf(stderr, "can't set signal handler: %s\n", strerror(errno));
		goto cleanup;
	}

	map_fd = bpf_map__fd(skel->maps.args_map);
	args.thresh = g_thresh;
	err = bpf_map_update_elem(map_fd, &args_key, &args, 0);
	if (err) {
		fprintf(stderr, "Failed to update flag map\n");
		goto cleanup;
	}

	printf("Starting trace, Can hit <Ctrl+C> to abort and report\n");
	for (i = 0; i < NR_THREADS; i++){
		targs[i] = (struct thread_args *)malloc(sizeof(struct thread_args));
		targs[i]->type = i;
		targs[i]->skel = skel;
		//err = report_data(skel,	TRACE_LATENCY);
		pthread_create(&tid[i],NULL,report_data,(void *)targs[i]);
	}

	/* Attach tracepoint handler */
	err = wqlat_bpf__attach(skel);
	if (err) {
		fprintf(stderr, "Failed to attach BPF skeleton\n");
		return err;
	}
	pthread_exit(NULL);

cleanup:
	wqlat_bpf__destroy(skel);
	return err;
}
