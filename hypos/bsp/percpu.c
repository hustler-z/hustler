/**
 * Hustler's Project
 *
 * File:  percpu.c
 * Date:  2024/06/06
 * Usage:
 */

#include <asm-generic/section.h>
#include <bsp/percpu.h>
#include <bsp/debug.h>
#include <bsp/cpu.h>

DEFINE_PERCPU(unsigned int, cpu_id);

unsigned long __percpu_offset[NR_CPUS];

int __bootfunc percpu_setup(void)
{
    MSGH("Percpu Varible Setup\n");

    set_processor_id(0);

    return 0;
}
