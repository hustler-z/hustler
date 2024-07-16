/**
 * Hustler's Project
 *
 * File:  percpu.h
 * Date:  2024/05/21
 * Usage:
 */

#ifndef _BSP_PERCPU_H
#define _BSP_PERCPU_H
// --------------------------------------------------------------
#include <asm/sysregs.h>
#include <bsp/compiler.h>

#define DECLARE_PERCPU(type, name) \
    extern __typeof__(type) percpu_ ## name

#define __DEFINE_PERCPU(attr, type, name) \
    attr __typeof__(type) percpu_ ## name

#define DEFINE_PERCPU(type, name) \
    __DEFINE_PERCPU(__section(".bss.percpu"), type, name)

#define DEFINE_PERCPU_RO(type, name) \
    __DEFINE_PERCPU(__section(".bss.percpu.ro"), type, name)

#define get_percpu_offset()  READ_SYSREG(TPIDR_EL2)

#define this_cpu(var) \
    (*RELOC_HIDE(&percpu_##var, get_percpu_offset()))

// --------------------------------------------------------------

extern unsigned long __percpu_offset[];

#define percpu(var, cpu) \
    (*RELOC_HIDE(&percpu_##var, __percpu_offset[(cpu)]))

#define percpu_ptr(var, cpu)  \
    (*RELOC_HIDE(var, __percpu_offset[(cpu)]))
#define this_cpu_ptr(var) \
    (*RELOC_HIDE(var, get_percpu_offset()))

DECLARE_PERCPU(unsigned int, cpu_id);

#define set_processor_id(id)                        \
do {                                                \
    WRITE_SYSREG(__percpu_offset[(id)], TPIDR_EL2); \
    this_cpu(cpu_id) = (id);                        \
} while(0)

#define smp_processor_id()  this_cpu(cpu_id)

int percpu_setup(void);
// --------------------------------------------------------------
#endif /* _BSP_PERCPU_H */
