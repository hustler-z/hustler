#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>	/* bpf_obj_pin */
#include <getopt.h>
// #include "pidComm.h"
#include "sched_jit.h"
#include "schedhang.skel.h"
#include "schedhang.h"

#define MAX_SYMS 300000
#define SIZE_512MB	(512*1024*1024)
unsigned int nr_cpus;
FILE *filep = NULL;
char filename[256] = {0};
char log_dir[] = "/var/log/syspeek/schedhang";
char defaultfile[] = "/var/log/syspeek/schedhang/schedhang.log";

static struct ksym syms[MAX_SYMS];
static volatile sig_atomic_t exiting;
static int sym_cnt,  stackmp_fd;
char *help_str = "schedhang";
struct evn {
	__u64 thresh;
	bool summary;
	bool verbose;
	struct sched_jit_summary *sump;
	char *shm_p;
} env = {
	.thresh = LAT_THRESH_NS,
	.summary = false,
	.verbose = false,
	.shm_p = NULL,
};

static void usage(char *prog)
{
	const char *str =
	"  Usage: %s [OPTIONS]\n"
	"  Options:\n"
	"  -t THRESH_TIME       specify the threshold time(ms), default=10ms\n"
	"  -f result.log        result file, default is /var/log/syspeek/schedhang.log\n"
	"  -s TIME              specify how long to run \n"
	"  -S shmkey            record summary log to share memory\n"
	;

	fprintf(stderr, str, prog);
	exit(EXIT_FAILURE);
}

static int ksym_cmp(const void *p1, const void *p2)
{
	return ((struct ksym *)p1)->addr - ((struct ksym *)p2)->addr;
}

int load_kallsyms(void)
{
	FILE *f = fopen("/proc/kallsyms", "r");
	char func[256], buf[256];
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

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	if (env.verbose)
		return vfprintf(stderr, format, args);
	else
		return 0;
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

static void print_ksym(__u64 addr)
{
	struct ksym *sym;

	if (!addr)
		return;

	sym = ksym_search(addr);
	if (sym)
		fprintf(filep, "<0x%llx> %s\n", addr, sym->name);
	else
		fprintf(filep, "<0x%llx>: missing symbols\n", addr);
}

static void print_stack(int fd, __u32 ret)
{
	int i;
	__u64 ip[PERF_MAX_STACK_DEPTH] = {};

	if (bpf_map_lookup_elem(fd, &ret, &ip) == 0) {
		for (i = 7; i < PERF_MAX_STACK_DEPTH - 1; i++)
			print_ksym(ip[i]);
	} else {
		if ((int)(ret) < 0)
		fprintf(filep, "<0x0000000000000000>:error=%d\n", (int)(ret));
	}
}

static int prepare_directory(char *path)
{
	int ret;

	ret = mkdir(path, 0777);
	if (ret < 0 && errno != EEXIST)
		return errno;
	else
		return 0;
}

static void fill_maxN(struct jit_maxN *maxN, const struct event *e)
{
	maxN->delay = e->delay;
	maxN->cpu = e->cpu;
	maxN->pid = e->pid;
	maxN->stamp = e->stamp;
	strncpy(maxN->comm, e->comm, 16);
}

static void update_summary(struct sched_jit_summary* summary, const struct event *e)
{
	int i, ridx;
	char buf[CONID_LEN] = {0};

	summary->num++;
	summary->total += e->delay;

	if (e->delay < 10) {
		summary->less10ms++;
	} else if (e->delay < 50) {
		summary->less50ms++;
	} else if (e->delay < 100) {
		summary->less100ms++;
	} else if (e->delay < 500) {
		summary->less500ms++;
	} else if (e->delay < 1000) {
		summary->less1s++;
	} else {
		summary->plus1s++;
	}

	ridx = summary->num % CPU_ARRY_LEN;
	summary->lastN_array[ridx].cpu = e->cpu;
	summary->lastN_array[ridx].delay = e->delay;

#if 0
	strncpy(summary->lastN_array[ridx].con, "000000000000", sizeof(summary->lastN_array[ridx].con));
#else
	strncpy(summary->lastN_array[ridx].con, buf, sizeof(summary->lastN_array[ridx].con));
#endif

	if (e->delay > summary->topNmin) {
		__u64 tmp;
		int idx;
		struct jit_maxN *maxi;

		idx = 0;
		tmp = summary->maxN_array[0].delay;
		/* sort: user insert sort */
		for (i = 1; i < CPU_ARRY_LEN; i++) {
			maxi = &summary->maxN_array[i];
			if (tmp > maxi->delay) {
				tmp = maxi->delay;
				idx = i;
			}
 		}
		summary->topNmin = tmp;
		summary->min_idx = idx;
		fill_maxN(&summary->maxN_array[idx], e);
	}
}

void check_rewind_file(const char *path, FILE *fp)
{
	struct stat buf;

	stat(path, &buf);
	if (buf.st_size > SIZE_512MB)
		rewind(fp);
}

void handle_event(void *ctx, int cpu, void *data, __u32 data_sz)
{
	struct event tmpe, *e;
	const struct event *ep = data;
	struct tm *tm;
	char ts[64];
	time_t t;

	tmpe = *ep;
	e = &tmpe;
	time(&t);
	tm = localtime(&t);
	strftime(ts, sizeof(ts), "%F %H:%M:%S", tm);
	e->delay = e->delay/(1000*1000);
	if (env.summary) {
		if (e->cpu > nr_cpus - 1)
			return;
		if (e->exit != 0)
			update_summary(env.sump, e);
	} else {
		fprintf(filep, "%-18.6f %-5d %-15s %-8d %-10llu %-21s\n",
			((double)e->stamp/1000000000), e->cpu, e->comm,
			e->pid, e->delay, (e->exit==e->stamp)?"(EOF)":ts);
		if (e->exit == 0)
			print_stack(stackmp_fd, e->ret);
		fflush(filep);
	}
	check_rewind_file(filename, filep);
}

void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt)
{
	printf("Lost %llu events on CPU #%d!\n", lost_cnt, cpu);
}

int schedhang_handler(int poll_fd, struct schedhang_bpf *skel)
{
	int err = 0;
	struct perf_buffer *pb = NULL;
	struct perf_buffer_opts pb_opts = {};

	if (!env.summary) {
		fprintf(filep, "%-18s %-5s %-15s %-8s %-10s %-21s\n",
			"TIME(schedhang)", "CPU", "COMM", "TID", "LAT(ms)", "DATE");
	} else {
		memset(env.sump, 0, sizeof(struct sched_jit_summary));
	}

	pb = perf_buffer__new(poll_fd, 64, handle_event, handle_lost_events, NULL, &pb_opts);
	if (!pb) {
		err = -errno;
		fprintf(stderr, "failed to open perf buffer: %d\n", err);
		goto clean_schedhang;
	}

	/* Attach tracepoint handler */
	err = schedhang_bpf__attach(skel);
	if (err) {
		fprintf(stderr, "Failed to attach BPF skeleton\n");
		return err;
	}
	while (!exiting) {
		err = perf_buffer__poll(pb, 100);
		if (err < 0 && err != -EINTR) {
			fprintf(stderr, "error polling perf buffer: %s\n", strerror(-err));
			goto clean_schedhang;
		}
		/* reset err to return 0 if exiting */
		err = 0;
	}
clean_schedhang:
	perf_buffer__free(pb);
	return err;
}

static void sig_int(int signo)
{
	exiting = 1;
}

static void sig_alarm(int signo)
{
	exiting = 1;
}

int main(int argc, char **argv)
{
	char *p;
	struct schedhang_bpf *skel;
	struct args args;
	int c, option_index, args_key, shm_fd;
	unsigned long span = 0;
	int err, map_fd0, map_fd1, map_fd2;

	err = prepare_directory(log_dir);
	if (err)
		return err;

	for (;;) {
		c = getopt_long(argc, argv, "t:f:s:S:vh", NULL, &option_index);
		if (c == -1)
			break;

		switch (c) {
        case 't':
            env.thresh = (int)strtoul(optarg, NULL, 10);
            if ((errno == ERANGE && (env.thresh == LONG_MAX || env.thresh == LONG_MIN))
                || (errno != 0 && env.thresh == 0)) {
                fprintf(stderr, "%s :strtoul %s\n", strerror(errno), optarg);
                return errno;
            }
            env.thresh = env.thresh*1000*1000;
            break;
        case 'f':
            if (strlen(optarg) < 2)
                strncpy(filename, defaultfile, sizeof(filename));
            else
                strncpy(filename, optarg, sizeof(filename));
            filep = fopen(filename, "w+");
            if (!filep) {
                int ret = errno;
                fprintf(stderr, "%s :fopen %s\n",
                strerror(errno), filename);
                return ret;
            }
            break;
        case 's':
            span = (int)strtoul(optarg, NULL, 10);
            if ((errno == ERANGE && (span == LONG_MAX || span == LONG_MIN))
                || (errno != 0 && span == 0)) {
                perror("strtoul");
                return errno;
            }
            break;
        case 'S':
            shm_fd = shm_open(optarg, O_RDWR, 0666);
            if (shm_fd < 0) {
                /* we do not use summary, use detail instead. */
                fprintf(stdout, "shm_open %s: %s", optarg, strerror(errno));
                break;
            }

            p  = mmap(NULL, sizeof(struct sched_jit_summary)+32,
            PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
            if (!p) {
                fprintf(stdout, "mmap %s: %s", optarg, strerror(errno));
                break;
            }
            env.summary = true;
            env.sump = (struct sched_jit_summary *)p;
            break;
        case 'v':
            env.verbose = true;
            break;
        case 'h':
            usage(help_str);
            break;
        default:
            usage(help_str);
		}
	}
	if (!filep) {
		filep = fopen(defaultfile, "w+");
		if (!filep) {
			int ret = errno;
			fprintf(stderr, "%s :fopen %s\n",
				strerror(errno), defaultfile);
			return ret;
		}
	}
	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);
	nr_cpus = libbpf_num_possible_cpus();
	/* Bump RLIMIT_MEMLOCK to allow BPF sub-system to do anything */
	bump_memlock_rlimit();
	err = load_kallsyms();
	if (err) {
		fprintf(stderr, "Failed to load kallsyms\n");
		return err;
	}
	/* Open load and verify BPF application */
	skel = schedhang_bpf__open();
	if (!skel) {
		fprintf(stderr, "Failed to open BPF skeleton\n");
		return 1;
	}

	err = schedhang_bpf__load(skel);
	if (err) {
		fprintf(stderr, "Failed to load BPF skeleton\n");
		return 1;
	}

	map_fd0 = bpf_map__fd(skel->maps.args_map);
	map_fd1 = bpf_map__fd(skel->maps.events);
	map_fd2 = bpf_map__fd(skel->maps.stackmap);
	if (signal(SIGINT, sig_int) == SIG_ERR ||
		signal(SIGALRM, sig_alarm) == SIG_ERR) {
		fprintf(stderr, "can't set signal handler: %s\n", strerror(errno));
		goto cleanup;
	}

	stackmp_fd = map_fd2;
	args_key = 0;
	args.flag = TIF_NEED_RESCHED;
	args.thresh = env.thresh;
	err = bpf_map_update_elem(map_fd0, &args_key, &args, 0);
	if (err) {
		fprintf(stderr, "Failed to update flag map\n");
		goto cleanup;
	}

	if (span)
		alarm(span);

	err = schedhang_handler(map_fd1, skel);

cleanup:
	schedhang_bpf__destroy(skel);
	if (env.sump) {
		munmap(env.sump, sizeof(struct sched_jit_summary)+32);
	}
	return -err;
}
