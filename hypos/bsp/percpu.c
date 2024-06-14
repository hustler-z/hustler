/**
 * Hustler's Project
 *
 * File:  percpu.c
 * Date:  2024/06/06
 * Usage:
 */

#include <bsp/percpu.h>
#include <bsp/cpu.h>

unsigned long __percpu_offset[NR_CPUS];
