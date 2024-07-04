/**
 * Hustler's Project
 *
 * File:  exception.h
 * Date:  2024/05/20
 * Usage: handle hcpu exceptions.
 */

#ifndef _COMMON_EXCEPTION_H
#define _COMMON_EXCEPTION_H
// --------------------------------------------------------------
#include <asm/hcpu.h>

void do_bad_sync(struct hcpu_regs *regs);
void do_bad_irq(struct hcpu_regs *regs);
void do_bad_fiq(struct hcpu_regs *regs);
void do_bad_error(struct hcpu_regs *regs);

void do_sync(struct hcpu_regs *regs);
void do_irq(struct hcpu_regs *regs);
void do_fiq(struct hcpu_regs *regs);
void do_error(struct hcpu_regs *regs);
void panic_par(paddr_t par);

// --------------------------------------------------------------
#endif /* _COMMON_EXCEPTION_H */
