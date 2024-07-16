/**
 * Hustler's Project
 *
 * File:  traps.c
 * Date:  2024/05/20
 * Usage: exception handler implementation in C
 */

#include <asm/sysregs.h>
#include <asm/debug.h>
#include <bsp/traps.h>

// --------------------------------------------------------------
void hyp_syn_el2t_handler(struct hcpu_regs *regs)
{
    do_bad_sync(regs);
}

void hyp_irq_el2t_handler(struct hcpu_regs *regs)
{
    do_bad_irq(regs);
}

void hyp_fiq_el2t_handler(struct hcpu_regs *regs)
{
    do_bad_fiq(regs);
}

void hyp_serror_el2t_handler(struct hcpu_regs *regs)
{
    do_bad_error(regs);
}

// --------------------------------------------------------------
void hyp_syn_el2h_handler(struct hcpu_regs *regs)
{
    do_sync(regs);
}

void hyp_irq_el2h_handler(struct hcpu_regs *regs)
{
    do_irq(regs);
}

void hyp_fiq_el2h_handler(struct hcpu_regs *regs)
{
    do_fiq(regs);
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

void guest_fiq_handler(struct hcpu_regs *regs)
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

void guest_fiq_compact_handler(struct hcpu_regs *regs)
{
}

void guest_serror_compact_handler(struct hcpu_regs *regs)
{
}
// --------------------------------------------------------------
