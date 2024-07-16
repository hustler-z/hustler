/**
 * Hustler's Project
 *
 * File:  bitops.h
 * Date:  2024/06/19
 * Usage:
 */

#ifndef _LIB_BITOPS_H
#define _LIB_BITOPS_H
// ------------------------------------------------------------------------
#include <org/bitops.h>

#define BIT_WORD(nr)           ((nr) / BITS_PER_LONG)

unsigned long find_next_bit(const unsigned long *addr,
                            unsigned long size,
                            unsigned long offset);
unsigned long find_next_zero_bit(const unsigned long *addr,
                                 unsigned long size,
                                 unsigned long offset);
unsigned long find_first_bit(const unsigned long *addr,
                             unsigned long size);
unsigned long find_first_zero_bit(const unsigned long *addr,
                                  unsigned long size);

#define HAS_FAST_MULTIPLY         (0)

static inline unsigned int generic_hweight32(unsigned int w)
{
    w -= (w >> 1) & 0x55555555;
    w =  (w & 0x33333333) + ((w >> 2) & 0x33333333);
    w =  (w + (w >> 4)) & 0x0f0f0f0f;

    if (HAS_FAST_MULTIPLY)
        return (w * 0x01010101) >> 24;

    w += w >> 8;

    return (w + (w >> 16)) & 0xff;
}

static inline unsigned int generic_hweight16(unsigned int w)
{
    w -= ((w >> 1) & 0x5555);
    w =  (w & 0x3333) + ((w >> 2) & 0x3333);
    w =  (w + (w >> 4)) & 0x0f0f;

    return (w + (w >> 8)) & 0xff;
}

static inline unsigned int generic_hweight8(unsigned int w)
{
    w -= ((w >> 1) & 0x55);
    w =  (w & 0x33) + ((w >> 2) & 0x33);

    return (w + (w >> 4)) & 0x0f;
}

static inline unsigned int generic_hweight64(unsigned long w)
{
    if (BITS_PER_LONG < 64)
        return generic_hweight32(w >> 32) + generic_hweight32(w);

    w -= (w >> 1) & 0x5555555555555555UL;
    w =  (w & 0x3333333333333333UL) + ((w >> 2) & 0x3333333333333333UL);
    w =  (w + (w >> 4)) & 0x0f0f0f0f0f0f0f0fUL;

    if (HAS_FAST_MULTIPLY)
        return (w * 0x0101010101010101UL) >> 56;

    w += w >> 8;
    w += w >> 16;

    return (w + (w >> 32)) & 0xFF;
}

static inline unsigned int hweight_long(unsigned long w)
{
    return sizeof(w) == 4 ? generic_hweight32(w) : generic_hweight64(w);
}

#define for_each_set_bit(bit, addr, size)              \
    for ((bit) = find_first_bit(addr, size);           \
         (bit) < (size);                               \
         (bit) = find_next_bit(addr, size, (bit) + 1))
// ------------------------------------------------------------------------
#endif /* _LIB_BITOPS_H */
