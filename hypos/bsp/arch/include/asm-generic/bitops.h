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
// --------------------------------------------------------------
#endif /* _ASM_GENERIC_BITOPS_H */
