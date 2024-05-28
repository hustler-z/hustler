/**
 * Hustler's Project
 *
 * File:  arch_sync.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_SYNCHRONIZATION_H
#define _ARCH_SYNCHRONIZATION_H
// ---------------------------------------------------------


#define isb()           asm volatile("isb" : : : "memory")
#define dsb(scope)      asm volatile("dsb " #scope : : : "memory")
#define dmb(scope)      asm volatile("dmb " #scope : : : "memory")

#define sev()           asm volatile("sev" : : : "memory")
#define wfe()           asm volatile("wfe" : : : "memory")
#define wfi()           asm volatile("wfi" : : : "memory")

#define mb()            dsb(sy)
#define rmb()           dsb(ld)
#define wmb()           dsb(st)

#define smp_mb()        dmb(ish)
#define smp_rmb()       dmb(ishld)
#define smp_wmb()       dmb(ishst)

// ---------------------------------------------------------
#endif /* _ARCH_SYNCHRONIZATION_H */
