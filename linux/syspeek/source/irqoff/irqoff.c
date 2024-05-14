#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "pidComm.h"
#include "sched_jit.h"
#include "irqoff.h"
#include "irqoff.skel.h"

struct env {
	__u64 sample_period;
	time_t duration;
	bool verbose, summary;
	__u64 threshold;
	struct sched_jit_summary *sump;
	char *shm_p;
} env = {
	.duration = 0,
	.threshold = 10*1000*1000,	/* 10ms */
	.summary = false,
	.shm_p = NULL,
};

static int nr_cpus;
FILE *filep = NULL;
char filename[256] = {0};
struct sched_jit_summary summary, *percpu_summary;
char log_dir[] = "/var/log/sysak/irqoff";
char defaultfile[] = "/var/log/sysak/irqoff/irqoff.log";

static struct ksym *ksyms;
static int stackmp_fd;
volatile sig_atomic_t exiting = 0;

void print_stack(int fd, __u32 ret, struct ksym *syms);
int load_kallsyms(struct ksym **pksyms);

const char *argp_program_version = "irqoff 0.1";
const char argp_program_doc[] =
"Catch the irq-off time more than threshold.\n"
"\n"
"USAGE: irqoff [--help] [-t THRESH(ms)] [-S SHM] [-f LOGFILE] [duration(s)]\n"
"\n"
"EXAMPLES:\n"
"    irqoff                # run forever, and detect irqoff more than 10ms(default)\n"
"    irqoff -S shmkey      # record summary log to share memory shmkey\n"
"    irqoff -t 15          # detect irqoff with threshold 15ms (default 10ms)\n"
"    irqoff -f a.log       # record result to a.log (default to ~sysak/irqoff/irqoff.log)\n";

static const struct argp_option opts[] = {
	{ "threshold", 't', "THRESH", 0, "Threshold to detect, default 10ms"},
	{ "logfile", 'f', "LOGFILE", 0, "logfile for result"},
	{ "summary", 'S', "SHMKEY", 0, "Record summary log to share memory"},
	{ "verbose", 'v', NULL, 0, "Verbose debug output"},
	{ NULL, 'h', NULL, OPTION_HIDDEN, "Show the full help"},
	{},
};

static error_t parse_arg(int key, char *arg, struct argp_state *state)
{
	char *p;
	int shm_fd, ret = errno;
	static int pos_args;

	switch (key) {
	case 'h':
		argp_state_help(state, stderr, ARGP_HELP_STD_HELP);
		break;
	case 'S':
		percpu_summary = malloc(nr_cpus*sizeof(struct sched_jit_summary));
		if (!percpu_summary)
			return -ENOMEM;
		shm_fd = shm_open(arg, O_RDWR, 0666);
		if (shm_fd < 0) {
			/* we do not use summary, use detail instead. */
			fprintf(stdout, "shm_open %s: %s", arg, strerror(errno));
			break;
		}

		p  = mmap(NULL, sizeof(struct sched_jit_summary)+32,
			PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
		if (!p) {
			fprintf(stdout, "mmap %s: %s", arg, strerror(errno));
			break;
		}
		env.summary = true;
		env.sump = (struct sched_jit_summary *)p;
		break;
	case 'v':
		env.verbose = true;
		break;
	case 't':
		errno = 0;
		__u64 thresh;
		thresh = strtoull(arg, NULL, 10);
		if (errno) {
			fprintf(stderr, "invalid threshold\n");
			argp_usage(state);
			break;
		} else if (thresh < 5) {
			fprintf(stderr, "threshold must >5ms, set to default 10ms\n");
			break;
		}
		env.threshold = thresh * 1000*1000;
		break;
	case 'f':
		if (strlen(arg) < 2)
			strncpy(filename, defaultfile, sizeof(filename));
		else
			strncpy(filename, arg, sizeof(filename));
		filep = fopen(filename, "w+");
		if (!filep) {
			ret = errno;
			fprintf(stderr, "%s :fopen %s\n",
				strerror(errno), filename);
			return ret;
		}
		break;
	case ARGP_KEY_ARG:
		if (pos_args++) {
			fprintf(stderr,
				"unrecognized positional argument: %s\n", arg);
			argp_usage(state);
		}
		errno = 0;
		env.duration = strtol(arg, NULL, 10);
		if (errno) {
			ret = errno;
			fprintf(stderr, "invalid duration\n");
			argp_usage(state);
			return ret;
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	/* refer to watchdog.c:set_sample_period, sample_period set to thres*2/5. */
	env.sample_period = env.threshold*2/5;

	return 0;
}

static void bump_memlock_rlimit(void)
{
	struct rlimit rlim_new = {
		.rlim_cur = RLIM_INFINITY,
		.rlim_max = RLIM_INFINITY,
	};

	if (setrlimit(RLIMIT_MEMLOCK, &rlim_new)) {
		fprintf(stderr, "Failed to increase RLIMIT_MEMLOCK limit!\n");
		exit(1);
	}
}

static int
open_and_attach_perf_event(struct perf_event_attr *attr,
			   struct bpf_program *prog,
			   struct bpf_link *links[])
{
	int i, fd;

	for (i = 0; i < nr_cpus; i++) {
		fd = syscall(__NR_perf_event_open, attr, -1, i, -1, 0);
		if (fd < 0) {
			/* Ignore CPU that is offline */
			if (errno == ENODEV)
				continue;
			fprintf(stderr, "failed to init perf sampling: %s\n",
				strerror(errno));
			return -1;
		}
		links[i] = bpf_program__attach_perf_event(prog, fd);
		if (!links[i]) {
			fprintf(stderr, "failed to attach perf event on cpu: %d\n", i);
			close(fd);
			return -1;
		}
	}
	return 0;
}

/* surprise! return 0 if failed! */
static int attach_prog_to_perf(struct irqoff_bpf *obj,
		struct bpf_link **sw_mlinks,struct bpf_link **hw_mlinks)
{
	int ret = 0;

	struct perf_event_attr attr_hw = {
		.type = PERF_TYPE_HARDWARE,
		.freq = 0,
		.sample_period = env.sample_period*2,	/* refer to watchdog_update_hrtimer_threshold() */
		.config = PERF_COUNT_HW_CPU_CYCLES,
	};

	struct perf_event_attr attr_sw = {
		.type = PERF_TYPE_SOFTWARE,
		.freq = 0,
		.sample_period = env.sample_period,
		.config = PERF_COUNT_SW_CPU_CLOCK,
	};

	if (!open_and_attach_perf_event(&attr_hw, obj->progs.hw_irqoff_event, hw_mlinks)) {
		ret = 1<<PERF_TYPE_SOFTWARE;
		if (!open_and_attach_perf_event(&attr_sw, obj->progs.sw_irqoff_event1, sw_mlinks))
			ret = ret | 1<<PERF_TYPE_SOFTWARE;

	} else {
		if (!open_and_attach_perf_event(&attr_sw, obj->progs.sw_irqoff_event2, sw_mlinks))
			ret = 1<<PERF_TYPE_SOFTWARE;
	}
	return ret;
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
	if (get_container(buf, e->pid))
		strncpy(summary->lastN_array[ridx].con, "000000000000", sizeof(summary->lastN_array[ridx].con));
	else
		strncpy(summary->lastN_array[ridx].con, buf, sizeof(summary->lastN_array[ridx].con));

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

void handle_event(void *ctx, int cpu, void *data, __u32 data_sz)
{
	struct event ev, *e;
	const struct event *ep = data;
	struct tm *tm;
	char ts[64];
	time_t t;

	ev = *ep;
	e = &ev;
	time(&t);
	tm = localtime(&t);
	strftime(ts, sizeof(ts), "%F %H:%M:%S", tm);
	e->delay = e->delay/(1000*1000);
	if (env.summary) {
		if (e->cpu > nr_cpus - 1)
			return;

		update_summary(env.sump, e);
	} else {
		fprintf(filep, "%-21s %-5d %-15s %-8d %-10llu %-18.6f\n",
			ts, e->cpu, e->comm, e->pid, e->delay, ((double)e->stamp/1000000000));
		print_stack(stackmp_fd, e->ret, ksyms);
	}
}

void irqoff_handler(int poll_fd, int map_fd, struct irqoff_bpf *obj,
		struct bpf_link **sw_mlinks, struct bpf_link **hw_mlinks)
{
	int arg_key = 0, err = 0;
	struct arg_info arg_info = {};
	struct perf_buffer *pb = NULL;
	struct perf_buffer_opts pb_opts = {};

	if (!env.summary) {
		fprintf(filep, "%-21s %-5s %-15s %-8s %-10s %-18s\n",
			"TIME(irqoff)", "CPU", "COMM", "TID", "LAT(ms)", "STAMP");
	} else {
		memset(env.sump, 0, sizeof(struct sched_jit_summary));
	}

	pb_opts.sample_cb = handle_event;
	pb = perf_buffer__new(poll_fd, 64, &pb_opts);
	if (!pb) {
		err = -errno;
		fprintf(stderr, "failed to open perf buffer: %d\n", err);
		goto clean_irqoff;
	}

	arg_info.thresh = env.threshold;
	err = bpf_map_update_elem(map_fd, &arg_key, &arg_info, 0);
	if (err) {
		fprintf(stderr, "Failed to update arg_map\n");
		goto clean_irqoff;
	}

	if (!attach_prog_to_perf(obj, sw_mlinks, hw_mlinks))
		goto clean_irqoff;

	while (!exiting) {
		err = perf_buffer__poll(pb, 100);
		if (err < 0 && err != -EINTR) {
			fprintf(stderr, "error polling perf buffer: %s\n", strerror(-err));
			goto clean_irqoff;
		}
		/* reset err to return 0 if exiting */
		err = 0;
	}

clean_irqoff:
	perf_buffer__free(pb);
}

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	if (level == LIBBPF_DEBUG && !env.verbose)
		return 0;
	return vfprintf(stderr, format, args);
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

static void sig_alarm(int signo)
{
	exiting = 1;
}

static void sig_int(int sig)
{
	exiting = 1;
}

int main(int argc, char **argv)
{
	int err, i, ent_fd, arg_fd;
	struct irqoff_bpf *obj;
	struct bpf_link **sw_mlinks, **hw_mlinks= NULL;
	static const struct argp argp = {
		.options = opts,
		.parser = parse_arg,
		.doc = argp_program_doc,
	};

	err = prepare_directory(log_dir);
	if (err) {
		fprintf(stderr, "prepare_directory %s fail\n", log_dir);
		return err;
	}
	ksyms = NULL;

	nr_cpus = libbpf_num_possible_cpus();
	if (nr_cpus < 0) {
		fprintf(stderr, "failed to get # of possible cpus: '%s'!\n",
			strerror(-nr_cpus));
		return 1;
	}

	err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (err) {
		fprintf(stderr, "argp_parse fail\n");
		return err;
	}
	if (!filep) {
		filep = fopen(defaultfile, "w+");
		if (!filep) {
			err = errno;
			fprintf(stderr, "%s :fopen %s\n",
				strerror(errno), defaultfile);
			return err;
		}
	}
	libbpf_set_print(libbpf_print_fn);

	bump_memlock_rlimit();
	err = load_kallsyms(&ksyms);
	if (err) {
		fprintf(stderr, "Failed to load kallsyms\n");
		return err;
	}

	sw_mlinks = calloc(nr_cpus, sizeof(*sw_mlinks));
	if (!sw_mlinks) {
		fprintf(stderr, "failed to alloc sw_mlinks or rlinks\n");
		return 1;
	}
	hw_mlinks = calloc(nr_cpus, sizeof(*hw_mlinks));
	if (!hw_mlinks) {
		fprintf(stderr, "failed to alloc hw_mlinks or rlinks\n");
		free(sw_mlinks);
		return 1;
	}

	obj = irqoff_bpf__open_and_load();
	if (!obj) {
		fprintf(stderr, "failed to open and/or load BPF object\n");
		goto cleanup;
	}

	arg_fd = bpf_map__fd(obj->maps.arg_map);
	ent_fd = bpf_map__fd(obj->maps.events);
	stackmp_fd = bpf_map__fd(obj->maps.stackmap);

	if (signal(SIGINT, sig_int) == SIG_ERR ||
		signal(SIGALRM, sig_alarm) == SIG_ERR) {
		fprintf(stderr, "can't set signal handler: %s\n", strerror(errno));
		err = 1;
		goto cleanup;
	}

	if (env.duration)
		alarm(env.duration);

	irqoff_handler(ent_fd, arg_fd, obj, sw_mlinks, hw_mlinks);

cleanup:
	for (i = 0; i < nr_cpus; i++) {
		bpf_link__destroy(sw_mlinks[i]);
		bpf_link__destroy(hw_mlinks[i]);
	}
	free(sw_mlinks);
	free(hw_mlinks);
	if (ksyms)
		free(ksyms);
	irqoff_bpf__destroy(obj);

	if (env.sump) {
		munmap(env.sump, sizeof(struct sched_jit_summary)+32);
	}

	return err != 0;
}
