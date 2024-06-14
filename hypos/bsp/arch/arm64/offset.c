/**
 * Hustler's Project
 *
 * File:  offset.c
 * Date:  2024/06/14
 * Usage:
 */

#include <asm/hypregs.h>
#include <asm/setup.h>
#include <lib/define.h>
#include <hyp/sched.h>

#define DEFINE(sym, val) \
    asm volatile("\n.ascii \"->" #sym " %0 " #val "\"" : : "i" (val))

#define BLANK() asm volatile("\n.ascii \"->\"" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(struct str, mem))

#define COMMENT(x) \
    asm volatile("\n.ascii \"->#" x "\"")

void __offset__(void)
{
#if 0 /* Unaligned Opcode */
    OFFSET(HYPOS_X0, hypos_regs, x0);
    OFFSET(HYPOS_X1, hypos_regs, x1);
    OFFSET(HYPOS_LR, hypos_regs, lr);

    OFFSET(HYPOS_SP, hypos_regs, sp);
    OFFSET(HYPOS_PC, hypos_regs, pc);
    OFFSET(HYPOS_CPSR, hypos_regs, cpsr);
    OFFSET(HYPOS_ESR, hypos_regs, esr);

    OFFSET(HYPOS_SPSR_EL1, hypos_regs, spsr_el1);

    OFFSET(HYPOS_SPSR_FIQ, hypos_regs, spsr_fiq);
    OFFSET(HYPOS_SPSR_IRQ, hypos_regs, spsr_irq);
    OFFSET(HYPOS_SPSR_UND, hypos_regs, spsr_und);
    OFFSET(HYPOS_SPSR_ABT, hypos_regs, spsr_abt);

    OFFSET(HYPOS_SP_EL0, hypos_regs, sp_el0);
    OFFSET(HYPOS_SP_EL1, hypos_regs, sp_el1);
    OFFSET(HYPOS_ELR_EL1, hypos_regs, elr_el1);

    OFFSET(HYPOS_CORE_SIZE, hypos_regs, spsr_el1);
    BLANK();

    DEFINE(HYPOS_CPU_SIZE, sizeof(struct hypos_cpu));
    OFFSET(HYPOS_CPU_FLAGS, hypos_cpu, flags);
    BLANK();

    OFFSET(VCPU_SAVED_CONTEXT, vcpu, arch.saved_context);
    BLANK();

    OFFSET(BOOT_STACK, arch_stack, stack);
    BLANK();
#endif
}
