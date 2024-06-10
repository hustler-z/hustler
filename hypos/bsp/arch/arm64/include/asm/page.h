/**
 * Hustler's Project
 *
 * File:  page.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_PAGE_H
#define _ARCH_PAGE_H
// --------------------------------------------------------------
#include <asm-generic/bitops.h>

/*
 * Granularity | PAGE_SHIFT | LEVELS OF LOOKUP
 * -------------------------------------------
 * 4K          | 12         | 4
 * 16K         | 14         | 4
 * 64K         | 16         | 3
 */
#define PAGE_SHIFT          12
#define PAGE_SIZE           (_AC(1, L) << PAGE_SHIFT) /* 4KB */
#define PAGE_MASK           (~(PAGE_SIZE-1)) /* 0xFFFFFFFF FFFFF000 */
#define PAGE_OFFSET(ptr)    ((unsigned long)(ptr) & ~PAGE_MASK)
#define PAGE_ALIGN(x)       (((x) + PAGE_SIZE - 1) & PAGE_MASK)

#define PADDR_BITS          (48)
#define VADDR_BITS          (48)
#define PADDR_MASK          ((_AC(1,ULL) << PADDR_BITS) - 1)
#define VADDR_MASK          (~_AC(0,UL) >> (BITS_PER_LONG - VADDR_BITS))

#define STACK_ORDER         (3)
#define STACK_SIZE          (PAGE_SIZE << STACK_ORDER)
// --------------------------------------------------------------
#ifndef __ASSEMBLY__
#include <generic/type.h>
#include <asm/sysregs.h>
#include <asm/barrier.h>

#define GV2P_READ        (0U << 0)
#define GV2P_WRITE       (1U << 0)
#define GV2P_EXEC        (1U << 1)

/* at => Address Translate
 * S1E1R
 * S1E1W
 * S1E0R
 * S1E0W
 * S1E1RP
 * S1E1WP
 * S1E1A
 * S1E2R
 * S1E2W
 * S12E1R
 * S12E1W
 * S12E0R
 * S12E0W
 * S1E2A
 * S1E3R
 * S1E3W
 * S1E3A
 */
static inline u64 __va_to_pa(vaddr_t va)
{
    u64 par, tmp = read_sysreg_par();
    asm volatile ("at s1e2r, %0;" : : "r"(va));
    isb();
    par = read_sysreg_par();
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
}

static inline u64 __gva_to_pa(vaddr_t va, unsigned int flags)
{
    u64 par, tmp = read_sysreg_par();
    if (!flags & GV2P_WRITE)
        asm volatile ("at s12e1r, %0;" : : "r"(va));
    else
        asm volatile ("at s12e1w, %0;" : : "r"(va));
    isb();
    par = read_sysreg_par();
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
}

static inline u64 __gva_to_ipa(vaddr_t va, unsigned int flags)
{
    u64 par, tmp = read_sysreg_par();
    if (!flags & GV2P_WRITE)
        asm volatile ("at s1e1r, %0;" : : "r"(va));
    else
        asm volatile ("at s1e1w, %0;" : : "r"(va));
    isb();
    par = read_sysreg_par();
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
}
#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#endif /* _ARCH_PAGE_H */
