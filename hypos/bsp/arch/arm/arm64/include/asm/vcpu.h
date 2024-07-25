/**
 * Hustler's Project
 *
 * File:  vcpu.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ASM_VCPU_H
#define _ASM_VCPU_H
// --------------------------------------------------------------
#include <org/gic.h>
#include <org/vgic.h>
#include <bsp/compiler.h>

/* Architecture Virtual CPU Struct for AArch64
 * --------------------------------------------------------------
 * (a) Basic Context
 * (b) Stack Address
 * (c) hypos CPU Information
 * (d) Basic Architectural Registers
 * (e) GIC Data
 * (f) VGIC Data
 * --------------------------------------------------------------
 *  org/vgic.h  ◀---+   XXX: error: field '*' has incomplete type
 *     |            |   One possible way to avoid this error is
 *  asm/vcpu.h      :   to take the header file which contains all
 *     \            |   necessary data structures.
 *   org/vcpu.h ---▶+
 * --------------------------------------------------------------
 */
struct arch_vcpu {
    struct {
        register_t x19;
        register_t x20;
        register_t x21;
        register_t x22;
        register_t x23;
        register_t x24;
        register_t x25;
        register_t x26;
        register_t x27;
        register_t x28;

        register_t fp;
        register_t sp;
        register_t pc;
    } saved_context;

    void *stack;

    struct hcpu *hcpu_info;

    register_t far;
    register_t esr;

    register_t vbar;
    register_t ttbcr;
    register_t ttbr0, ttbr1;
    register_t par;

    register_t mair;
    register_t amair;

    register_t sctlr;
    register_t actlr;
    register_t cpacr;

    register_t tpidr_el0;
    register_t tpidr_el1;
    register_t tpidrro_el0;

    register_t zcr_el1;
    register_t zcr_el2;

    register_t cptr_el2;
    register_t hcr_el2;
    register_t mdcr_el2;

    /* CP 15 */
    u32 csselr;
    register_t vmpidr;

    /* XXX: Physical GIC Data */
    union gic_state_data gic;
    u64        lr_mask;

    /* XXX: Virtual GIC Data */
    struct vgic_cpu vgic;
    register_t cntkctl;

} __cacheline_aligned;

// --------------------------------------------------------------
#endif /* _ASM_VCPU_H */
