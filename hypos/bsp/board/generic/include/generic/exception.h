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

void show_regs(struct hypos_regs *regs);

void do_bad_sync(struct hypos_regs *regs);
void do_bad_irq(struct hypos_regs *regs);
void do_bad_fiq(struct hypos_regs *regs);
void do_bad_error(struct hypos_regs *regs);

void do_sync(struct hypos_regs *regs);
void do_irq(struct hypos_regs *regs);
void do_fiq(struct hypos_regs *regs);
void do_error(struct hypos_regs *regs);

// --------------------------------------------------------------
#endif /* _GENERIC_EXCEPTION_H */
