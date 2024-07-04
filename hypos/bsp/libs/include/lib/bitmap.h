/**
 * Hustler's Project
 *
 * File:  bitmap.h
 * Date:  2024/06/25
 * Usage:
 */

#ifndef _BSP_BITMAP_H
#define _BSP_BITMAP_H
// --------------------------------------------------------------
#include <asm-generic/bitops.h>
#include <asm/bitops.h>
#include <lib/strops.h>
#include <lib/math.h>

int __bitmap_empty(const unsigned long *bitmap, unsigned int bits);
int __bitmap_full(const unsigned long *bitmap, unsigned int bits);
int __bitmap_equal(const unsigned long *bitmap1,
                   const unsigned long *bitmap2, unsigned int bits);
void __bitmap_complement(unsigned long *dst, const unsigned long *src,
                         unsigned int bits);
void __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
                  const unsigned long *bitmap2, unsigned int bits);
void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
                 const unsigned long *bitmap2, unsigned int bits);
void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
                  const unsigned long *bitmap2, unsigned int bits);
void __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
                     const unsigned long *bitmap2, unsigned int bits);
int __bitmap_intersects(const unsigned long *bitmap1,
                        const unsigned long *bitmap2, unsigned int bits);
int __bitmap_subset(const unsigned long *bitmap1,
                    const unsigned long *bitmap2, unsigned int bits);
unsigned int __bitmap_weight(const unsigned long *bitmap, unsigned int bits);
void __bitmap_set(unsigned long *map, unsigned int start, int len);
void __bitmap_clear(unsigned long *map, unsigned int start, int len);

#define BITMAP_LAST_WORD_MASK(nbits)			  \
(									              \
    ((nbits) % BITS_PER_LONG) ?					  \
        (1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL \
)

#define bitmap_bytes(nbits) (BITS_TO_LONGS(nbits) * sizeof(unsigned long))

#define bitmap_switch(nbits, zero, small, large)			          \
    unsigned int n__ = (nbits);					                      \
    if (__builtin_constant_p(nbits) && !n__) {			              \
        zero;							                              \
    } else if (__builtin_constant_p(nbits) && n__ <= BITS_PER_LONG) { \
        small;							                              \
    } else {							                              \
        large;							                              \
    }

static inline void bitmap_zero(unsigned long *dst, unsigned int nbits)
{
    bitmap_switch(nbits,,
        *dst = 0UL,
        memset(dst, 0, bitmap_bytes(nbits)));
}

static inline void bitmap_fill(unsigned long *dst, unsigned int nbits)
{
    size_t nlongs = BITS_TO_LONGS(nbits);

    switch (nlongs) {
    case 0:
        break;
    default:
        memset(dst, -1, (nlongs - 1) * sizeof(unsigned long));
        /* fall through */
    case 1:
        dst[nlongs - 1] = BITMAP_LAST_WORD_MASK(nbits);
        break;
    }
}

static inline void bitmap_copy(unsigned long *dst, const unsigned long *src,
        unsigned int nbits)
{
    bitmap_switch(nbits,,
        *dst = *src,
        memcpy(dst, src, bitmap_bytes(nbits)));
}

static inline void bitmap_and(unsigned long *dst, const unsigned long *src1,
        const unsigned long *src2, unsigned int nbits)
{
    bitmap_switch(nbits,,
        *dst = *src1 & *src2,
        __bitmap_and(dst, src1, src2, nbits));
}

static inline void bitmap_or(unsigned long *dst, const unsigned long *src1,
        const unsigned long *src2, unsigned int nbits)
{
    bitmap_switch(nbits,,
        *dst = *src1 | *src2,
        __bitmap_or(dst, src1, src2, nbits));
}

static inline void bitmap_xor(unsigned long *dst, const unsigned long *src1,
        const unsigned long *src2, unsigned int nbits)
{
    bitmap_switch(nbits,,
        *dst = *src1 ^ *src2,
        __bitmap_xor(dst, src1, src2, nbits));
}

static inline void bitmap_andnot(unsigned long *dst, const unsigned long *src1,
        const unsigned long *src2, unsigned int nbits)
{
    bitmap_switch(nbits,,
        *dst = *src1 & ~*src2,
        __bitmap_andnot(dst, src1, src2, nbits));
}

static inline void bitmap_complement(unsigned long *dst, const unsigned long *src,
        unsigned int nbits)
{
    bitmap_switch(nbits,,
        *dst = ~*src & BITMAP_LAST_WORD_MASK(nbits),
        __bitmap_complement(dst, src, nbits));
}

static inline int bitmap_equal(const unsigned long *src1,
        const unsigned long *src2, unsigned int nbits)
{
    bitmap_switch(nbits,
        return -1,
        return !((*src1 ^ *src2) & BITMAP_LAST_WORD_MASK(nbits)),
        return __bitmap_equal(src1, src2, nbits));
}

static inline int bitmap_intersects(const unsigned long *src1,
        const unsigned long *src2, unsigned int nbits)
{
    bitmap_switch(nbits,
        return -1,
        return ((*src1 & *src2) & BITMAP_LAST_WORD_MASK(nbits)) != 0,
        return __bitmap_intersects(src1, src2, nbits));
}

static inline int bitmap_subset(const unsigned long *src1,
        const unsigned long *src2, unsigned int nbits)
{
    bitmap_switch(nbits,
        return -1,
        return !((*src1 & ~*src2) & BITMAP_LAST_WORD_MASK(nbits)),
        return __bitmap_subset(src1, src2, nbits));
}

static inline int bitmap_empty(const unsigned long *src, unsigned int nbits)
{
    bitmap_switch(nbits,
        return -1,
        return !(*src & BITMAP_LAST_WORD_MASK(nbits)),
        return __bitmap_empty(src, nbits));
}

static inline int bitmap_full(const unsigned long *src, unsigned int nbits)
{
    bitmap_switch(nbits,
        return -1,
        return !(~*src & BITMAP_LAST_WORD_MASK(nbits)),
        return __bitmap_full(src, nbits));
}

static inline unsigned int bitmap_weight(const unsigned long *src,
                                     unsigned int nbits)
{
    return __bitmap_weight(src, nbits);
}

/* LITTLE_ENDIAN */
#define BITMAP_MEM_ALIGNMENT          8
#define BITMAP_MEM_MASK               (BITMAP_MEM_ALIGNMENT - 1)
#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))

static inline void bitmap_set(unsigned long *map, unsigned int start,
        unsigned int nbits)
{
    if (__builtin_constant_p(nbits) && nbits == 1)
        set_bit(start, map);
    else if (__builtin_constant_p(start & BITMAP_MEM_MASK) &&
        IS_ALIGNED(start, BITMAP_MEM_ALIGNMENT) &&
        __builtin_constant_p(nbits & BITMAP_MEM_MASK) &&
        IS_ALIGNED(nbits, BITMAP_MEM_ALIGNMENT))
        memset((char *)map + start / 8, 0xff, nbits / 8);
    else
        __bitmap_set(map, start, nbits);
}

static inline void bitmap_clear(unsigned long *map, unsigned int start,
    unsigned int nbits)
{
    if (__builtin_constant_p(nbits) && nbits == 1)
        clear_bit(start, map);
    else if (__builtin_constant_p(start & BITMAP_MEM_MASK) &&
        IS_ALIGNED(start, BITMAP_MEM_ALIGNMENT) &&
        __builtin_constant_p(nbits & BITMAP_MEM_MASK) &&
        IS_ALIGNED(nbits, BITMAP_MEM_ALIGNMENT))
        memset((char *)map + start / 8, 0, nbits / 8);
    else
        __bitmap_clear(map, start, nbits);
}
// --------------------------------------------------------------
#endif /* _BSP_BITMAP_H */
