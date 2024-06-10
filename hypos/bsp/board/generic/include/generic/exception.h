/**
 * Hustler's Project
 *
 * File:  exception.h
 * Date:  2024/05/20
 * Usage: handle hypos exceptions.
 */

#ifndef _GENERIC_EXCEPTION_H
#define _GENERIC_EXCEPTION_H
// --------------------------------------------------------------
#include <asm/hypregs.h>

void show_regs(struct hyp_regs *regs);

void do_bad_sync(struct hyp_regs *regs);
void do_bad_irq(struct hyp_regs *regs);
void do_bad_fiq(struct hyp_regs *regs);
void do_bad_error(struct hyp_regs *regs);

void do_sync(struct hyp_regs *regs);
void do_irq(struct hyp_regs *regs);
void do_fiq(struct hyp_regs *regs);
void do_error(struct hyp_regs *regs);

// --------------------------------------------------------------
#endif /* _GENERIC_EXCEPTION_H */
