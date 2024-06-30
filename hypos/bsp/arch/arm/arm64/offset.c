/**
 * Hustler's Project
 *
 * File:  offset.c
 * Date:  2024/06/14
 * Usage:
 */

#include <asm/hcpu.h>
#include <asm/setup.h>
#include <lib/define.h>
#include <core/vcpu.h>

// --------------------------------------------------------------
#define DEFINE(sym, val) \
	asm volatile("\n.ascii \"->" #sym " %0 " #val "\"" : : "i" (val))
#define BLANK() asm volatile("\n.ascii \"->\"" : : )
#define OFFSET(sym, str, mem) \
	DEFINE(sym, offsetof(struct str, mem))

/* Error: unaligned opcodes detected in executable segment!!!
 * --------------------------------------------------------------
 * XXX: How do we solve the problem above?
 *      Obviously make those codes aligned.
 * --------------------------------------------------------------
 */
void __offset__(void)
{
    /* Hypervisor Registers
     */
    OFFSET(HCPU_X0, hcpu_regs, x0);
    OFFSET(HCPU_X1, hcpu_regs, x1);
    OFFSET(HCPU_LR, hcpu_regs, lr);

    OFFSET(HCPU_SP, hcpu_regs, sp);
    OFFSET(HCPU_PC, hcpu_regs, pc);
    OFFSET(HCPU_CPSR, hcpu_regs, cpsr);
    OFFSET(HCPU_ESR, hcpu_regs, esr);

    /* Guest Registers
     */
    OFFSET(HCPU_SPSR_EL1, hcpu_regs, spsr_el1);
    OFFSET(HCPU_SP_EL0, hcpu_regs, sp_el0);
    OFFSET(HCPU_SP_EL1, hcpu_regs, sp_el1);
    OFFSET(HCPU_ELR_EL1, hcpu_regs, elr_el1);

    /* Hypervisor Registers Offset
     */
    OFFSET(HYPERVISOR_OFFSET, hcpu_regs, spsr_el1);
    BLANK();

    /* Hypervisor CPU Setups
     */
    DEFINE(HCPU_SIZE, sizeof(struct hcpu));
    OFFSET(HCPU_FLAG, hcpu, flags);
    BLANK();

    /* Hypervisor Stack
     */
    OFFSET(HCPU_STACK, boot_setup, stack);
    BLANK();

    /* Virtual CPU Context
     */
    OFFSET(VCPU_CONTEXT, vcpu, arch.saved_context);
    BLANK();
}
// --------------------------------------------------------------
