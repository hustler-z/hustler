/**
 * Hustler's Project
 *
 * File:  percpu.c
 * Date:  2024/06/06
 * Usage:
 */

#include <org/section.h>
#include <asm/debug.h>
#include <bsp/percpu.h>
#include <bsp/debug.h>
#include <bsp/cpu.h>
#include <bsp/panic.h>

// --------------------------------------------------------------
DEFINE_PERCPU(unsigned int, cpu_id);

unsigned long __percpu_offset[NR_CPUS];

extern char __percpu_start[], __percpu_end[];

#define INVALID_PERCPU_AREA      (-(long)__percpu_start)

int __bootfunc percpu_setup(void)
{
    unsigned int cpu;

    set_processor_id(0);

    early_debug("[hypos] Boot CPU start kicking\n");

    for (cpu = 1; cpu < NR_CPUS; cpu++)
        __percpu_offset[cpu] = INVALID_PERCPU_AREA;

    return 0;
}
// --------------------------------------------------------------
