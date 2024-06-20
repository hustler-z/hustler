/**
 * Hustler's Project
 *
 * File:  bitops.c
 * Date:  2024/06/19
 * Usage:
 */

#include <asm-generic/bitops.h>
#include <lib/bitops.h>

// --------------------------------------------------------------
#define __ffs(x)       (ffsl(x) - 1)
#define ffz(x)         __ffs(~(x))
#define BIT_WORD(nr)   ((nr) / BITS_PER_LONG)

unsigned long find_next_bit(const unsigned long *addr,
                            unsigned long size,
                            unsigned long offset)
{
    const unsigned long *p = addr + BIT_WORD(offset);
    unsigned long result = offset & ~(BITS_PER_LONG-1);
    unsigned long tmp;

    if (offset >= size)
        return size;
    size -= result;
    offset %= BITS_PER_LONG;
    if (offset) {
        tmp = *(p++);
        tmp &= (~0UL << offset);
        if (size < BITS_PER_LONG)
            goto found_first;
        if (tmp)
            goto found_middle;
        size -= BITS_PER_LONG;
        result += BITS_PER_LONG;
    }
    while (size & ~(BITS_PER_LONG-1)) {
        if ((tmp = *(p++)))
            goto found_middle;
        result += BITS_PER_LONG;
        size -= BITS_PER_LONG;
    }
    if (!size)
        return result;
    tmp = *p;

found_first:
    tmp &= (~0UL >> (BITS_PER_LONG - size));
    if (tmp == 0UL)		/* Are any bits set? */
        return result + size;	/* Nope. */
found_middle:
    return result + __ffs(tmp);
}

unsigned long find_next_zero_bit(const unsigned long *addr,
                                 unsigned long size,
                                 unsigned long offset)
{
    const unsigned long *p = addr + BIT_WORD(offset);
    unsigned long result = offset & ~(BITS_PER_LONG-1);
    unsigned long tmp;

    if (offset >= size)
        return size;
    size -= result;
    offset %= BITS_PER_LONG;
    if (offset) {
        tmp = *(p++);
        tmp |= ~0UL >> (BITS_PER_LONG - offset);
        if (size < BITS_PER_LONG)
            goto found_first;
        if (~tmp)
            goto found_middle;
        size -= BITS_PER_LONG;
        result += BITS_PER_LONG;
    }
    while (size & ~(BITS_PER_LONG-1)) {
        if (~(tmp = *(p++)))
            goto found_middle;
        result += BITS_PER_LONG;
        size -= BITS_PER_LONG;
    }
    if (!size)
        return result;
    tmp = *p;

found_first:
    tmp |= ~0UL << size;
    if (tmp == ~0UL)	/* Are any bits zero? */
        return result + size;	/* Nope. */
found_middle:
    return result + ffz(tmp);
}

unsigned long find_first_bit(const unsigned long *addr,
                             unsigned long size)
{
    const unsigned long *p = addr;
    unsigned long result = 0;
    unsigned long tmp;

    while (size & ~(BITS_PER_LONG-1)) {
        if ((tmp = *(p++)))
            goto found;
        result += BITS_PER_LONG;
        size -= BITS_PER_LONG;
    }
    if (!size)
        return result;

    tmp = (*p) & (~0UL >> (BITS_PER_LONG - size));
    if (tmp == 0UL)		/* Are any bits set? */
        return result + size;	/* Nope. */
found:
    return result + __ffs(tmp);
}

unsigned long find_first_zero_bit(const unsigned long *addr,
                                  unsigned long size)
{
    const unsigned long *p = addr;
    unsigned long result = 0;
    unsigned long tmp;

    while (size & ~(BITS_PER_LONG-1)) {
        if (~(tmp = *(p++)))
            goto found;
        result += BITS_PER_LONG;
        size -= BITS_PER_LONG;
    }
    if (!size)
        return result;

    tmp = (*p) | (~0UL << size);
    if (tmp == ~0UL)	/* Are any bits zero? */
        return result + size;	/* Nope. */
found:
    return result + ffz(tmp);
}
// --------------------------------------------------------------
