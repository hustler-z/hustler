/**
 * Hustler's Project
 *
 * File:  cache.h
 * Date:  2024/07/12
 * Usage:
 */

#ifndef _ORG_CACHE_H
#define _ORG_CACHE_H
// --------------------------------------------------------------
#include <asm/sysregs.h>
#include <asm/barrier.h>
#include <asm/cache.h>
#include <bsp/panic.h>
#include <bsp/type.h>

#define MIN_CACHELINE_BYTES 16

extern size_t dcache_line_bytes;

static inline size_t read_dcache_line_bytes(void)
{
    register_t ctr;

    /* Read CTR */
    ctr = READ_SYSREG(CTR_EL0);

    /* Bits 16-19 are the log2 number of words in the cacheline. */
    return (size_t) (4 << ((ctr >> 16) & 0xf));
}
static inline int invalidate_dcache_va_range(const void *p,
                                             unsigned long size)
{
    size_t cacheline_mask = dcache_line_bytes - 1;
    unsigned long idx = 0;

    if (!size)
        return 0;

    /* Passing a region that wraps around is illegal */
    ASSERT(((ap_t)p + size - 1) >= (ap_t)p);

    dsb(sy);           /* So the CPU issues all writes to the range */

    if ((ap_t)p & cacheline_mask) {
        size -= dcache_line_bytes - ((ap_t)p & cacheline_mask);
        p = (void *)((ap_t)p & ~cacheline_mask);
        asm volatile (__clean_and_invalidate_dcache_one(0)
                : : "r" (p));
        p += dcache_line_bytes;
    }

    for ( ; size >= dcache_line_bytes;
            idx += dcache_line_bytes, size -= dcache_line_bytes)
        asm volatile (__invalidate_dcache_one(0) : : "r" (p + idx));

    if (size > 0)
        asm volatile (__clean_and_invalidate_dcache_one(0)
                : : "r" (p + idx));

    dsb(sy);

    return 0;
}

static inline int clean_dcache_va_range(const void *p,
                                        unsigned long size)
{
    size_t cacheline_mask = dcache_line_bytes - 1;
    unsigned long idx = 0;

    if (!size)
        return 0;

    /* Passing a region that wraps around is illegal */
    ASSERT(((ap_t)p + size - 1) >= (ap_t)p);

    dsb(sy);           /* So the CPU issues all writes to the range */
    size += (ap_t)p & cacheline_mask;
    size = (size + cacheline_mask) & ~cacheline_mask;
    p = (void *)((ap_t)p & ~cacheline_mask);
    for ( ; size >= dcache_line_bytes;
            idx += dcache_line_bytes, size -= dcache_line_bytes)
        asm volatile (__clean_dcache_one(0) : : "r" (p + idx));
    dsb(sy);
    /* ARM callers assume that dcache_* functions cannot fail. */
    return 0;
}

static inline int clean_and_invalidate_dcache_va_range
    (const void *p, unsigned long size)
{
    size_t cacheline_mask = dcache_line_bytes - 1;
    unsigned long idx = 0;

    if (!size)
        return 0;

    /* Passing a region that wraps around is illegal */
    ASSERT(((ap_t)p + size - 1) >= (ap_t)p);

    dsb(sy);         /* So the CPU issues all writes to the range */
    size += (ap_t)p & cacheline_mask;
    size = (size + cacheline_mask) & ~cacheline_mask;
    p = (void *)((ap_t)p & ~cacheline_mask);
    for ( ; size >= dcache_line_bytes;
            idx += dcache_line_bytes, size -= dcache_line_bytes)
        asm volatile (__clean_and_invalidate_dcache_one(0)
                      : : "r" (p + idx));
    dsb(sy); /* So we know the flushes happen before continuing */
    /* ARM callers assume that dcache_* functions cannot fail. */
    return 0;
}

#define clean_dcache(x) do {                                        \
    typeof(x) *_p = &(x);                                           \
    if (sizeof(x) > MIN_CACHELINE_BYTES || sizeof(x) > alignof(x))  \
        clean_dcache_va_range(_p, sizeof(x));                       \
    else                                                            \
        asm volatile (                                              \
            "dsb sy;"   /* Finish all earlier writes */             \
            __clean_dcache_one(0)                                   \
            "dsb sy;"   /* Finish flush before continuing */        \
            : : "r" (_p), "m" (*_p));                               \
} while (0)

#define clean_and_invalidate_dcache(x) do {                         \
    typeof(x) *_p = &(x);                                           \
    if (sizeof(x) > MIN_CACHELINE_BYTES || sizeof(x) > alignof(x))  \
        clean_and_invalidate_dcache_va_range(_p, sizeof(x));        \
    else                                                            \
        asm volatile (                                              \
            "dsb sy;"   /* Finish all earlier writes */             \
            __clean_and_invalidate_dcache_one(0)                    \
            "dsb sy;"   /* Finish flush before continuing */        \
            : : "r" (_p), "m" (*_p));                               \
} while (0)

// --------------------------------------------------------------
#endif /* _ORG_CACHE_H */
