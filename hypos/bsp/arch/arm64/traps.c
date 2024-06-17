/**
 * Hustler's Project
 *
 * File:  exception.c
 * Date:  2024/05/20
 * Usage: exception handler implementation in C
 */

#include <asm/sysregs.h>
#include <asm/debug.h>
#include <generic/exception.h>

// --------------------------------------------------------------
int hyp_syn_el2t_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] hyp_syn_el2t happened\n");
    do_bad_sync(regs);

    return 0;
}

int hyp_irq_el2t_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] hyp_irq_el2t happened\n");
    do_bad_irq(regs);

    return 0;
}

int hyp_fiq_el2t_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] hyp_fiq_el2t happened\n");
    do_bad_fiq(regs);

    return 0;
}

int hyp_serror_el2t_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] hyp_serror_el2t happened\n");
    do_bad_error(regs);

    return 0;
}

// --------------------------------------------------------------
int hyp_syn_el2h_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] hyp_syn_el2h happened\n");
    return 0;
}

int hyp_irq_el2h_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] hyp_irq_el2h happened\n");
    return 0;
}

int hyp_fiq_el2h_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] hyp_fiq_el2h happened\n");
    return 0;
}

int hyp_serror_el2h_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] hyp_serror_el2h happened\n");
    return 0;
}

// --------------------------------------------------------------
int guest_syn_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] guest_syn_el1 happened\n");
    return 0;
}

int guest_irq_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] guest_irq_el1 happened\n");
    return 0;
}

int guest_fiq_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] guest_fiq_el1 happened\n");
    return 0;
}

int guest_serror_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] guest_serror_el1 happened\n");
    return 0;
}

// --------------------------------------------------------------
int guest_syn_compact_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] guest_syn_el1_compact happened\n");
    return 0;
}

int guest_irq_compact_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] guest_irq_el1_compact happened\n");
    return 0;
}

int guest_fiq_compact_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] guest_fiq_el1_compact happened\n");
    return 0;
}

int guest_serror_compact_handler(struct hcpu_regs *regs)
{
    early_debug("[exception] guest_serror_el1_compact happened\n");
    return 0;
}
// --------------------------------------------------------------
