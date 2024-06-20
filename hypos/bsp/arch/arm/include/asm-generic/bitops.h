/**
 * Hustler's Project
 *
 * File:  bitops.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _ASM_GENERIC_BITOPS_H
#define _ASM_GENERIC_BITOPS_H
// --------------------------------------------------------------

/* bit constant macro for both assembly and C code.
 */
#ifdef __ASSEMBLY__
#define _AC(X, Y)       X
#define _AT(T, X)       X
#else
#define __AC(X, Y)      (X##Y)
#define _AC(X, Y)       __AC(X, Y)
#define _AT(T, X)       ((T)(X))
#endif

#define BIT(pos, sfx)   (_AC(1, sfx) << (pos))
#define BYTES_PER_LONG  8
#define BITS_PER_LONG   (BYTES_PER_LONG << 3)

#define GENMASK(h, l) \
    (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define ISOLATE_LSB(x)  ((x) & -(x))

#ifndef __ASSEMBLY__
// --------------------------------------------------------------

static inline int generic_fls(unsigned int x)
{
    int r = 32;

    if (!x)
        return 0;

    if (!(x & 0xFFFF0000U)) {
        x <<= 16;
        r -= 16;
    }

    if (!(x & 0xFF000000U)) {
        x <<= 8;
        r -= 8;
    }

    if (!(x & 0xF0000000U)) {
        x <<= 4;
        r -= 4;
    }

    if (!(x & 0xC0000000U)) {
        x <<= 2;
        r -= 2;
    }

    if (!(x & 0x80000000U)) {
        x <<= 1;
        r -= 1;
    }

    return r;
}

static inline int generic_flsl(unsigned long x)
{
    unsigned int h = x >> 32;

    return h ? generic_fls(h) + 32 : generic_fls(x);
}

static inline int flsl(unsigned long x)
{
    unsigned long ret;

    if (__builtin_constant_p(x))
        return generic_flsl(x);

    asm("clz\t%0, %1" : "=r" (ret) : "r" (x));

    return BITS_PER_LONG - ret;
}

#define ffsl(x) ({ \
    unsigned long __t = (x); flsl(ISOLATE_LSB(__t)); })

// --------------------------------------------------------------
#endif /* __ASSEMBLY__ */

// --------------------------------------------------------------
#endif /* _ASM_GENERIC_BITOPS_H */
