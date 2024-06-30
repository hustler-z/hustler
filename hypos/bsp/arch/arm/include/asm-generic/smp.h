/**
 * Hustler's Project
 *
 * File:  smp.h
 * Date:  2024/06/27
 * Usage:
 */

#ifndef _ASM_GENERIC_SMP_H
#define _ASM_GENERIC_SMP_H
// --------------------------------------------------------------

#include <asm/bitops.h>
#include <bsp/percpu.h>
#include <bsp/check.h>
#include <bsp/cpu.h>
#include <common/type.h>
#include <lib/bitmap.h>
#include <lib/bitops.h>

typedef struct cpumask{
    DECLARE_BITMAP(bits, NR_CPUS);
} cpumask_t;

extern unsigned int nr_cpu_ids;

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

extern cpumask_t cpu_possible_map;
extern cpumask_t cpu_online_map;
extern cpumask_t cpu_present_map;

#define num_online_cpus()	cpumask_weight(&cpu_online_map)
#define num_possible_cpus()	cpumask_weight(&cpu_possible_map)
#define num_present_cpus()	cpumask_weight(&cpu_present_map)
#define cpu_online(cpu)	    cpumask_test_cpu(cpu, &cpu_online_map)
#define cpu_possible(cpu)   cpumask_test_cpu(cpu, &cpu_possible_map)
#define cpu_present(cpu)    cpumask_test_cpu(cpu, &cpu_present_map)

#define for_each_cpu(cpu, mask)			\
    for ((cpu) = cpumask_first(mask);	\
         (cpu) < nr_cpu_ids;		    \
         (cpu) = cpumask_next(cpu, mask))

#define for_each_possible_cpu(cpu) for_each_cpu(cpu, &cpu_possible_map)
#define for_each_online_cpu(cpu)   for_each_cpu(cpu, &cpu_online_map)
#define for_each_present_cpu(cpu)  for_each_cpu(cpu, &cpu_present_map)

int smp_setup(void);
// --------------------------------------------------------------
#endif /* _ASM_GENERIC_SMP_H */
