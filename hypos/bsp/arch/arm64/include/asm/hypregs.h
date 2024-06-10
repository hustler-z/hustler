/**
 * Hustler's Project
 *
 * File:  hypregs.h
 * Date:  2024/05/23
 * Usage:
 */

#ifndef _ARCH_HYP_REGS_H
#define _ARCH_HYP_REGS_H
// --------------------------------------------------------------
#ifndef __ASSEMBLY__

#include <generic/type.h>

struct hyp_regs {
    u64 x0;
    u64 x1;
    u64 x2;
    u64 x3;
    u64 x4;
    u64 x5;
    u64 x6;
    u64 x7;
    u64 x8;
    u64 x9;
    u64 x10;
    u64 x11;
    u64 x12;
    u64 x13;
    u64 x14;
    u64 x15;
    u64 x16;
    u64 x17;
    u64 x18;
    u64 x19;
    u64 x20;
    u64 x21;
    u64 x22;
    u64 x23;
    u64 x24;
    u64 x25;
    u64 x26;
    u64 x27;
    u64 x28;

    u64 fp;      /* x29 */
    u64 lr;      /* x30 */

    u64 sp;

    u64 elr;     /* ELR_EL2 */
    u64 cpsr;    /* SPSR_EL2 */
    u64 esr;     /* ESR_EL2 */

    /* The kernel frame should be 16-byte aligned. */
    u64 pad0;

    /* Outer guest frame only from here on... */
    u64 spsr_el1;

    u32 spsr_fiq, spsr_irq, spsr_und, spsr_abt;

    /* AArch64 guests only */
    u64 sp_el0;
    u64 sp_el1, elr_el1;
};

#endif /* !__ASSEMBLY__ */

#define ARCH_REGS_SPSR_EL1   0x0
#define ARCH_REGS_LR         0x0
#define ARCH_REGS_PC         0x0
#define ARCH_REGS_CPSR       0x0
// --------------------------------------------------------------
#endif /* _ARCH_HYP_REGS_H */
