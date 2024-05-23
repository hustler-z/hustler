/**
 * Hustler's Project
 *
 * File:  arch_bitops.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _ARCH_BITOPS_H
#define _ARCH_BITOPS_H
// ---------------------------------------------------------

/* bit constant macro for both assembly and C code.
 */
#ifdef __ASSEMBLY__
#define _AC(X, Y)       X
#else
#define __AC(X, Y)      (X##Y)
#define _AC(X, Y)       __AC(X, Y)
#endif

#define BIT(pos, sfx)   (_AC(1, sfx) << (pos))
#define BYTES_PER_LONG  8
#define BITS_PER_LONG   (BYTES_PER_LONG << 3)

// ---------------------------------------------------------
#endif /* _ARCH_BITOPS_H */
