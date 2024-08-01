/**
 * Hustler's Project
 *
 * File:  traps.h
 * Date:  2024/05/20
 * Usage: handle hcpu exceptions.
 */

#ifndef _ORG_TRAPS_H
#define _ORG_TRAPS_H
// --------------------------------------------------------------
#include <asm/hcpu.h>

// --------------------------------------------------------------
void do_bad_sync(struct hcpu_regs *regs);
void do_bad_irq(struct hcpu_regs *regs);
void do_bad_fiq(struct hcpu_regs *regs);
void do_bad_error(struct hcpu_regs *regs);

void do_sync(struct hcpu_regs *regs);
void do_irq(struct hcpu_regs *regs);
void do_fiq(struct hcpu_regs *regs);
void do_error(struct hcpu_regs *regs);
void panic_par(hpa_t par);

register_t get_hcpu_reg(struct hcpu_regs *regs, int reg);
void set_hcpu_reg(struct hcpu_regs *regs, int reg,
                  register_t value);
// --------------------------------------------------------------
#endif /* _ORG_TRAPS_H */
