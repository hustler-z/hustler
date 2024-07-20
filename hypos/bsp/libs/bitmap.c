/**
 * Hustler's Project
 *
 * File:  bitmap.c
 * Date:  2024/06/25
 * Usage:
 */

#include <lib/bitmap.h>
#include <lib/bitops.h>

// --------------------------------------------------------------
int __bitmap_empty(const unsigned long *bitmap,
                   unsigned int bits)
{
    int k, lim = bits / BITS_PER_LONG;

    for (k = 0; k < lim; ++k)
        if (bitmap[k])
            return 0;

    if (bits % BITS_PER_LONG)
        if (bitmap[k] & BITMAP_LAST_WORD_MASK(bits))
            return 0;

    return 1;
}

int __bitmap_full(const unsigned long *bitmap,
                  unsigned int bits)
{
    int k, lim = bits / BITS_PER_LONG;

    for (k = 0; k < lim; ++k)
        if (~bitmap[k])
            return 0;

    if (bits % BITS_PER_LONG)
        if (~bitmap[k] & BITMAP_LAST_WORD_MASK(bits))
            return 0;

    return 1;
}

int __bitmap_equal(const unsigned long *bitmap1,
                   const unsigned long *bitmap2,
                   unsigned int bits)
{
    int k, lim = bits / BITS_PER_LONG;

    for (k = 0; k < lim; ++k)
        if (bitmap1[k] != bitmap2[k])
            return 0;

    if (bits % BITS_PER_LONG)
        if ((bitmap1[k] ^ bitmap2[k]) &
                BITMAP_LAST_WORD_MASK(bits))
            return 0;

    return 1;
}

void __bitmap_complement(unsigned long *dst,
                         const unsigned long *src,
                         unsigned int bits)
{
    int k, lim = bits / BITS_PER_LONG;

    for (k = 0; k < lim; ++k)
        dst[k] = ~src[k];

    if (bits % BITS_PER_LONG)
        dst[k] = ~src[k] & BITMAP_LAST_WORD_MASK(bits);
}

void __bitmap_and(unsigned long *dst,
                  const unsigned long *bitmap1,
                  const unsigned long *bitmap2,
                  unsigned int bits)
{
    int k;
    int nr = BITS_TO_LONGS(bits);

    for (k = 0; k < nr; k++)
        dst[k] = bitmap1[k] & bitmap2[k];
}

void __bitmap_or(unsigned long *dst,
                 const unsigned long *bitmap1,
                 const unsigned long *bitmap2,
                 unsigned int bits)
{
	int k;
	int nr = BITS_TO_LONGS(bits);

	for (k = 0; k < nr; k++)
		dst[k] = bitmap1[k] | bitmap2[k];
}

void __bitmap_xor(unsigned long *dst,
                  const unsigned long *bitmap1,
                  const unsigned long *bitmap2,
                  unsigned int bits)
{
    int k;
    int nr = BITS_TO_LONGS(bits);

    for (k = 0; k < nr; k++)
        dst[k] = bitmap1[k] ^ bitmap2[k];
}

void __bitmap_andnot(unsigned long *dst,
                     const unsigned long *bitmap1,
                     const unsigned long *bitmap2,
                     unsigned int bits)
{
    int k;
    int nr = BITS_TO_LONGS(bits);

    for (k = 0; k < nr; k++)
        dst[k] = bitmap1[k] & ~bitmap2[k];
}

int __bitmap_intersects(const unsigned long *bitmap1,
                        const unsigned long *bitmap2,
                        unsigned int bits)
{
    int k, lim = bits / BITS_PER_LONG;

    for (k = 0; k < lim; ++k)
        if (bitmap1[k] & bitmap2[k])
            return 1;

    if (bits % BITS_PER_LONG)
        if ((bitmap1[k] & bitmap2[k]) &
                BITMAP_LAST_WORD_MASK(bits))
            return 1;
    return 0;
}

int __bitmap_subset(const unsigned long *bitmap1,
                    const unsigned long *bitmap2,
                    unsigned int bits)
{
    int k, lim = bits / BITS_PER_LONG;

    for (k = 0; k < lim; ++k)
        if (bitmap1[k] & ~bitmap2[k])
            return 0;

    if (bits % BITS_PER_LONG)
        if ((bitmap1[k] & ~bitmap2[k]) &
                BITMAP_LAST_WORD_MASK(bits))
            return 0;
    return 1;
}

unsigned int __bitmap_weight(const unsigned long *bitmap,
                             unsigned int bits)
{
    unsigned int k, w = 0, lim = bits / BITS_PER_LONG;

    for (k = 0; k < lim; k++)
        w += hweight_long(bitmap[k]);

    if (bits % BITS_PER_LONG)
        w += hweight_long(bitmap[k] &
                BITMAP_LAST_WORD_MASK(bits));

    return w;
}

void __bitmap_set(unsigned long *map,
                  unsigned int start, int len)
{
    unsigned long *p = map + BIT_WORD(start);
    const unsigned int size = start + len;
    int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);
    unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);

    while (len - bits_to_set >= 0) {
        *p |= mask_to_set;
        len -= bits_to_set;
        bits_to_set = BITS_PER_LONG;
        mask_to_set = ~0UL;
        p++;
    }
    if (len) {
        mask_to_set &= BITMAP_LAST_WORD_MASK(size);
        *p |= mask_to_set;
    }
}

void __bitmap_clear(unsigned long *map,
                    unsigned int start, int len)
{
    unsigned long *p = map + BIT_WORD(start);
    const unsigned int size = start + len;
    int bits_to_clear = BITS_PER_LONG - (start % BITS_PER_LONG);
    unsigned long mask_to_clear = BITMAP_FIRST_WORD_MASK(start);

    while (len - bits_to_clear >= 0) {
        *p &= ~mask_to_clear;
        len -= bits_to_clear;
        bits_to_clear = BITS_PER_LONG;
        mask_to_clear = ~0UL;
        p++;
    }
    if (len) {
        mask_to_clear &= BITMAP_LAST_WORD_MASK(size);
        *p &= ~mask_to_clear;
    }
}
// --------------------------------------------------------------
