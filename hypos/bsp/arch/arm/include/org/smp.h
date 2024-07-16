/**
 * Hustler's Project
 *
 * File:  smp.h
 * Date:  2024/06/27
 * Usage:
 */

#ifndef _ORG_SMP_H
#define _ORG_SMP_H
// --------------------------------------------------------------

#include <asm/bitops.h>
#include <bsp/compiler.h>
#include <bsp/calculate.h>
#include <bsp/percpu.h>
#include <bsp/panic.h>
#include <bsp/cpu.h>
#include <bsp/type.h>
#include <lib/bitmap.h>
#include <lib/bitops.h>

typedef struct cpumask {
    DECLARE_BITMAP(bits, NR_CPUS);
} cpumask_t;

#define nr_cpumask_bits (BITS_TO_LONGS(NR_CPUS) * BITS_PER_LONG)

extern unsigned int nr_cpu_ids;

extern void smpboot_setup(void);

// --------------------------------------------------------------
static inline unsigned int cpumask_check(unsigned int cpu)
{
    ASSERT(cpu < nr_cpu_ids);

    return cpu;
}

static inline void cpumask_set_cpu(int cpu,
        volatile cpumask_t *dstp)
{
    set_bit(cpumask_check(cpu), dstp->bits);
}

static inline void cpumask_setall(cpumask_t *dstp)
{
    bitmap_fill(dstp->bits, nr_cpumask_bits);
}

static inline void cpumask_clear_cpu(int cpu,
        volatile cpumask_t *dstp)
{
    clear_bit(cpumask_check(cpu), dstp->bits);
}

static inline bool cpumask_test_cpu(unsigned int cpu,
        const cpumask_t *src)
{
    return test_bit(cpumask_check(cpu), src->bits);
}

static inline int cpumask_weight(const cpumask_t *srcp)
{
    return bitmap_weight(srcp->bits, nr_cpu_ids);
}

static inline int cpumask_first(const cpumask_t *srcp)
{
    return min_t(int, nr_cpu_ids,
            find_first_bit(srcp->bits, nr_cpu_ids));
}

static inline int cpumask_next(int n, const cpumask_t *srcp)
{
    /* -1 is a legal arg here. */
    if (n != -1)
        cpumask_check(n);

    return min_t(int, nr_cpu_ids,
            find_next_bit(srcp->bits, nr_cpu_ids, n + 1));
}

static inline void cpumask_and(cpumask_t *dstp,
                               const cpumask_t *src1p,
                               const cpumask_t *src2p)
{
    bitmap_and(dstp->bits, src1p->bits, src2p->bits,
            nr_cpumask_bits);
}

static inline void cpumask_or(cpumask_t *dstp,
                              const cpumask_t *src1p,
                              const cpumask_t *src2p)
    {
    bitmap_or(dstp->bits, src1p->bits, src2p->bits,
            nr_cpumask_bits);
}

static inline void cpumask_xor(cpumask_t *dstp,
                               const cpumask_t *src1p,
                               const cpumask_t *src2p)
{
    bitmap_xor(dstp->bits, src1p->bits, src2p->bits,
            nr_cpumask_bits);
}

static inline int cpumask_subset(const cpumask_t *src1p,
                                 const cpumask_t *src2p)
{
	return bitmap_subset(src1p->bits, src2p->bits,
            nr_cpu_ids);
}

static inline void cpumask_andnot(cpumask_t *dstp,
                                  const cpumask_t *src1p,
                                  const cpumask_t *src2p)
{
    bitmap_andnot(dstp->bits, src1p->bits, src2p->bits,
            nr_cpumask_bits);
}

static inline void cpumask_complement(cpumask_t *dstp,
                                      const cpumask_t *srcp)
{
    bitmap_complement(dstp->bits, srcp->bits,
            nr_cpumask_bits);
}

static inline int cpumask_equal(const cpumask_t *src1p,
				const cpumask_t *src2p)
{
    return bitmap_equal(src1p->bits, src2p->bits,
            nr_cpu_ids);
}

static inline void cpumask_clear(cpumask_t *dstp)
{
	bitmap_zero(dstp->bits, nr_cpumask_bits);
}

static inline int cpumask_last(const cpumask_t *srcp)
{
    int cpu, pcpu = nr_cpu_ids;

    for (cpu = cpumask_first(srcp);
         cpu < nr_cpu_ids;
         cpu = cpumask_next(cpu, srcp))
        pcpu = cpu;
    return pcpu;
}

static inline int cpumask_cycle(int n,
                                const cpumask_t *srcp)
{
    int nxt = cpumask_next(n, srcp);

    if (nxt == nr_cpu_ids)
        nxt = cpumask_first(srcp);
    return nxt;
}

static inline int cpumask_empty(const cpumask_t *srcp)
{
	return bitmap_empty(srcp->bits, nr_cpu_ids);
}

static inline void cpumask_copy(cpumask_t *dstp,
                                const cpumask_t *srcp)
{
	bitmap_copy(dstp->bits, srcp->bits, nr_cpumask_bits);
}

extern const unsigned long
	cpu_bit_bitmap[BITS_PER_LONG+1][BITS_TO_LONGS(NR_CPUS)];

static inline const cpumask_t *cpumask_of(unsigned int cpu)
{
	const unsigned long *p = cpu_bit_bitmap[1 +
                    cpumask_check(cpu) % BITS_PER_LONG];

	return (const cpumask_t *)(p - cpu / BITS_PER_LONG);
}

static inline unsigned int cpumask_any(const cpumask_t *srcp)
{
    unsigned int cpu = cpumask_first(srcp);
    unsigned int w = cpumask_weight(srcp);

    if (w > 1 && cpu < nr_cpu_ids)
        for (w = get_random() % w; w--;) {
            unsigned int next = cpumask_next(cpu, srcp);

            if (next >= nr_cpu_ids)
                break;
            cpu = next;
        }

    return cpu;
}

extern cpumask_t cpu_possible_map;
extern cpumask_t cpu_online_map;
extern cpumask_t cpu_present_map;
extern register_t __cpu_logical_map[];
#define cpu_logical_map(cpu)    __cpu_logical_map[(cpu)]
// --------------------------------------------------------------
typedef cpumask_t cpumask_var_t[1];

static inline bool alloc_cpumask_var(cpumask_var_t *mask)
{
	return true;
}
#define cond_alloc_cpumask_var alloc_cpumask_var

static inline bool zalloc_cpumask_var(cpumask_var_t *mask)
{
	cpumask_clear(*mask);
	return true;
}
#define cond_zalloc_cpumask_var zalloc_cpumask_var

static inline void free_cpumask_var(cpumask_var_t mask)
{
}
// --------------------------------------------------------------
#define num_online_cpus()	cpumask_weight(&cpu_online_map)
#define num_possible_cpus()	cpumask_weight(&cpu_possible_map)
#define num_present_cpus()	cpumask_weight(&cpu_present_map)
#define cpu_online(cpu)	    cpumask_test_cpu(cpu, &cpu_online_map)
#define cpu_possible(cpu)   cpumask_test_cpu(cpu, &cpu_possible_map)
#define cpu_present(cpu)    cpumask_test_cpu(cpu, &cpu_present_map)

#define cpu_is_offline(cpu) unlikely(!cpu_online(cpu))

#define for_each_cpu(cpu, mask)			\
    for ((cpu) = cpumask_first(mask);	\
         (cpu) < nr_cpu_ids;		    \
         (cpu) = cpumask_next(cpu, mask))

#define for_each_possible_cpu(cpu) for_each_cpu(cpu, &cpu_possible_map)
#define for_each_online_cpu(cpu)   for_each_cpu(cpu, &cpu_online_map)
#define for_each_present_cpu(cpu)  for_each_cpu(cpu, &cpu_present_map)
// --------------------------------------------------------------
int halt_run(int (*fn)(void *data), void *data, unsigned int cpu);

void smp_send_call_function_mask(const cpumask_t *mask);

void smp_send_event_check_mask(const cpumask_t *mask);
#define smp_send_event_check_cpu(cpu) \
    smp_send_event_check_mask(cpumask_of(cpu))

void on_selected_cpus(
    const cpumask_t *selected,
    void (*func) (void *info),
    void *info,
    int wait);

void smp_call_function(
    void (*func) (void *info),
    void *info,
    int wait);

void smp_call_function_interrupt(void);
// --------------------------------------------------------------
void put_cpu_maps(void);
bool get_cpu_maps(void);

void cpu_hotplug_begin(void);
void cpu_hotplug_done(void);

/* CPU_UP_PREPARE: Preparing to bring CPU online. */
#define CPU_UP_PREPARE    (0x0001 | NOTIFY_FORWARD)
/* CPU_UP_CANCELED: CPU is no longer being brought online. */
#define CPU_UP_CANCELED   (0x0002 | NOTIFY_REVERSE)
/* CPU_STARTING: CPU nearly online. Runs on new CPU,
 * irqs still disabled. */
#define CPU_STARTING      (0x0003 | NOTIFY_FORWARD)
/* CPU_ONLINE: CPU is up. */
#define CPU_ONLINE        (0x0004 | NOTIFY_FORWARD)
/* CPU_DOWN_PREPARE: CPU is going down. */
#define CPU_DOWN_PREPARE  (0x0005 | NOTIFY_REVERSE)
/* CPU_DOWN_FAILED: CPU is no longer going down. */
#define CPU_DOWN_FAILED   (0x0006 | NOTIFY_FORWARD)
/* CPU_DYING: CPU is nearly dead (in stop_machine context). */
#define CPU_DYING         (0x0007 | NOTIFY_REVERSE)
/* CPU_DEAD: CPU is dead. */
#define CPU_DEAD          (0x0008 | NOTIFY_REVERSE)
/* CPU_REMOVE: CPU was removed. */
#define CPU_REMOVE        (0x0009 | NOTIFY_REVERSE)
/* CPU_RESUME_FAILED: CPU failed to come up in resume,
 * all other CPUs up. */
#define CPU_RESUME_FAILED (0x000a | NOTIFY_REVERSE)

int cpu_down(unsigned int cpu);
int cpu_up(unsigned int cpu);

void register_cpu_notifier(struct notifier_block *nb);
// --------------------------------------------------------------
/* Pre-secondary cpu setup
 */
int pre_secondary_cpu_setup(void);
int post_secondary_cpu_setup(void);
int prep_smp_mm(int cpu);
// --------------------------------------------------------------

#endif /* _ORG_SMP_H */
