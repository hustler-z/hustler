/**
 * Hustler's Project
 *
 * File:  exception.c
 * Date:  2024/05/20
 * Usage: exception handler implementation in C
 */

#include <asm/sysregs.h>
#include <bsp/stdio.h>
#include <generic/exception.h>

// --------------------------------------------------------------
int hyp_syn_el2t_handler(void)
{
    hyp_dbg("- hyp_syn_el2t -\n");
    return 0;
}

int hyp_irq_el2t_handler(void)
{
    hyp_dbg("- hyp_irq_el2t -\n");
    return 0;
}

int hyp_fiq_el2t_handler(void)
{
    hyp_dbg("- hyp_fiq_el2t -\n");
    return 0;
}

int hyp_serror_el2t_handler(void)
{
    hyp_dbg("- hyp_serror_el2t -\n");
    return 0;
}
// --------------------------------------------------------------
int hyp_syn_el2h_handler(void)
{
    hyp_dbg("- hyp_syn_el2h -\n");
    return 0;
}

int hyp_irq_el2h_handler(void)
{
    hyp_dbg("- hyp_irq_el2h -\n");
    return 0;
}

int hyp_fiq_el2h_handler(void)
{
    hyp_dbg("- hyp_fiq_el2h -\n");
    return 0;
}

int hyp_serror_el2h_handler(void)
{
    hyp_dbg("- hyp_serror_el2h -\n");
    return 0;
}
// --------------------------------------------------------------
int guest_syn_handler(void)
{
    hyp_dbg("- guest_syn_el1 -\n");
    return 0;
}

int guest_irq_handler(void)
{
    hyp_dbg("- guest_irq_el1 -\n");
    return 0;
}

int guest_fiq_handler(void)
{
    hyp_dbg("- guest_fiq_el1 -\n");
    return 0;
}

int guest_serror_handler(void)
{
    hyp_dbg("- guest_serror_el1 -\n");
    return 0;
}
// --------------------------------------------------------------
int guest_syn_compat_handler(void)
{
    return 0;
}

int guest_irq_compat_handler(void)
{
    return 0;
}

int guest_fiq_compat_handler(void)
{
    return 0;
}

int guest_serror_compat_handler(void)
{
    return 0;
}
// --------------------------------------------------------------
