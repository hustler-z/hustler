/**
 * Hustler's Project
 *
 * File:  hypregs.h
 * Date:  2024/05/23
 * Usage:
 */

#ifndef _ASM_HYP_REGS_H
#define _ASM_HYP_REGS_H
// --------------------------------------------------------------
#ifndef __ASSEMBLY__

#include <bsp/type.h>

struct hcpu_regs {
    register_t x0;
    register_t x1;
    register_t x2;
    register_t x3;
    register_t x4;
    register_t x5;
    register_t x6;
    register_t x7;
    register_t x8;
    register_t x9;
    register_t x10;
    register_t x11;
    register_t x12;
    register_t x13;
    register_t x14;
    register_t x15;
    register_t x16;
    register_t x17;
    register_t x18;
    register_t x19;
    register_t x20;
    register_t x21;
    register_t x22;
    register_t x23;
    register_t x24;
    register_t x25;
    register_t x26;
    register_t x27;
    register_t x28;

    register_t fp;      /* x29 */
    register_t lr;      /* x30 */

    register_t sp;
    register_t pc;      /* ELR_EL2 */

    register_t cpsr;    /* SPSR_EL2 */
    register_t esr;     /* ESR_EL2 */

    register_t pad0;

    /* For AArch64 Guests */
    register_t spsr_el1;
    register_t sp_el0;
    register_t sp_el1, elr_el1;
};

struct hcpu {
    struct hcpu_regs hcpu_regs;
    unsigned int flags;
    unsigned int cpuid;

    /* XXX: Remain openning */
};

int hcpu_setup(void);

#endif /* !__ASSEMBLY__ */

// --------------------------------------------------------------
#endif /* _ASM_HYP_REGS_H */
