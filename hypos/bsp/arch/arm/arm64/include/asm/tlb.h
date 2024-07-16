/**
 * Hustler's Project
 *
 * File:  tlb.h
 * Date:  2024/06/19
 * Usage:
 */

#ifndef _ASM_SPINLOCK_H
#define _ASM_SPINLOCK_H
// --------------------------------------------------------------

#ifndef __ASSEMBLY__
#include <org/cpu.h>
#include <asm/alternative.h>
#include <asm/page.h>
#include <bsp/type.h>

#define TLB_HELPER(name, tlbop, sh)              \
static inline void name(void)                    \
{                                                \
    asm volatile(                                \
        "dsb  "  # sh  "st;"                     \
        "tlbi "  # tlbop  ";"                    \
        ALTERNATIVE(                             \
            "nop; nop;",                         \
            "dsb  ish;"                          \
            "tlbi "  # tlbop  ";",               \
            ARM64_WORKAROUND_REPEAT_TLBI)        \
        "dsb  "  # sh  ";"                       \
        "isb;"                                   \
        : : : "memory");                         \
}

#define TLB_HELPER_VA(name, tlbop)               \
static inline void name(vaddr_t va)              \
{                                                \
    asm volatile(                                \
        "tlbi "  # tlbop  ", %0;"                \
        ALTERNATIVE(                             \
            "nop; nop;",                         \
            "dsb  ish;"                          \
            "tlbi "  # tlbop  ", %0;",           \
            ARM64_WORKAROUND_REPEAT_TLBI)        \
        : : "r" (va >> PAGE_SHIFT) : "memory");  \
}

/* Flush local TLBs, current VMID only. */
TLB_HELPER(flush_guest_tlb_local, vmalls12e1, nsh)

/* Flush innershareable TLBs, current VMID only */
TLB_HELPER(flush_guest_tlb, vmalls12e1is, ish)

/* Flush local TLBs, all VMIDs, non-hypervisor mode */
TLB_HELPER(flush_all_guests_tlb_local, alle1, nsh)

/* Flush innershareable TLBs, all VMIDs, non-hypervisor mode */
TLB_HELPER(flush_all_guests_tlb, alle1is, ish)

/* Flush all hypervisor mappings from the TLB of the local
 * processor.
 */
TLB_HELPER(flush_tlb_local, alle2, nsh)

/* Flush TLB of local processor for address va. */
TLB_HELPER_VA(__flush_tlb_one_local, vae2)

/* Flush TLB of all processors in the inner-shareable domain
 * for address va.
 */
TLB_HELPER_VA(__flush_tlb_one, vae2is)

void flush_tlb_range_va_local(vaddr_t va, unsigned long size);
void flush_tlb_range_va(vaddr_t va, unsigned long size);

#endif /* !__ASSEMBLY__ */

// --------------------------------------------------------------
#endif /* _ASM_SPINLOCK_H */
