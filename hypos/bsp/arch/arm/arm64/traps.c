/**
 * Hustler's Project
 *
 * File:  traps.c
 * Date:  2024/05/20
 * Usage: exception handler implementation in C
 */

#include <asm/sysregs.h>
#include <asm/barrier.h>
#include <asm/define.h>
#include <asm/debug.h>
#include <org/traps.h>

// --------------------------------------------------------------
void do_bad_trap(struct hcpu_regs *regs, unsigned int reason)
{
    switch (reason) {
    case BAD_SYNC:
        do_bad_sync(regs);
        break;
    case BAD_IRQ:
        do_bad_irq(regs);
        break;
    case BAD_FIQ:
        do_bad_fiq(regs);
        break;
    case BAD_ERROR:
        do_bad_error(regs);
        break;
    default:
        break;
    }
}
// --------------------------------------------------------------
void hyp_syn_el2h_handler(struct hcpu_regs *regs)
{
    unsigned long spsr = READ_SYSREG(SPSR_EL2);
    spsr &= (PSR_DBG_MASK | PSR_ABT_MASK | PSR_IRQ_MASK | PSR_FIQ_MASK);
    hypos_daif_set(spsr);

    do_sync(regs);
}

void hyp_irq_el2h_handler(struct hcpu_regs *regs)
{
    unsigned long spsr = READ_SYSREG(SPSR_EL2);
    unsigned long daif = PSR_DBG_MASK | PSR_ABT_MASK | PSR_FIQ_MASK;
    spsr &= daif;
    spsr |= PSR_IRQ_MASK;
    hypos_daif_set(spsr);

    do_irq(regs);
}

void hyp_serror_el2h_handler(struct hcpu_regs *regs)
{
    do_error(regs);
}

// --------------------------------------------------------------
void guest_syn_handler(struct hcpu_regs *regs)
{
}

void guest_irq_handler(struct hcpu_regs *regs)
{
}

void guest_serror_handler(struct hcpu_regs *regs)
{
}

// --------------------------------------------------------------
void guest_syn_compact_handler(struct hcpu_regs *regs)
{
}

void guest_irq_compact_handler(struct hcpu_regs *regs)
{
}

void guest_serror_compact_handler(struct hcpu_regs *regs)
{
}
// --------------------------------------------------------------
