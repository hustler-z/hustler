/**
 * Hustler's Project
 *
 * File:  cache.h
 * Date:  2024/06/27
 * Usage:
 */

#ifndef _ASM_CACHE_H
#define _ASM_CACHE_H
// --------------------------------------------------------------

#ifndef __ASSEMBLY__
#include <asm/alternative.h>

#define icache_clear(op) \
    asm volatile ("ic "#op"\n" \
                  "isb\n" : : : "memory")

#define ARM64_WORKAROUND_CLEAN_CACHE            0

#define __invalidate_dcache_one(R) "dc ivac, %" #R ";"

#define __clean_dcache_one(R)                   \
    ALTERNATIVE("dc cvac, %" #R ";",            \
                "dc civac, %" #R ";",           \
                ARM64_WORKAROUND_CLEAN_CACHE)

#define __clean_and_invalidate_dcache_one(R) \
    "dc  civac, %" #R ";"

/* Invalidate all instruction caches in Inner
 * Shareable domain to PoU.
 */
static inline void invalidate_icache(void)
{
    asm volatile ("ic ialluis");
    dsb(ish); /* Ensure completion of the flush I-cache */
    isb();
}

/* Invalidate all instruction caches on the local
 * processor to PoU.
 */
static inline void invalidate_icache_local(void)
{
    asm volatile ("ic iallu");
    dsb(nsh); /* Ensure completion of the I-cache flush */
    isb();
}

#endif /* !__ASSEMBLY__ */

// --------------------------------------------------------------
#endif /* _ASM_CACHE_H */
