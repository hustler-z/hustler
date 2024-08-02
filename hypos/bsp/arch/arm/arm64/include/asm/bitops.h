/**
 * Hustler's Project
 *
 * File:  bitops.h
 * Date:  2024/06/20
 * Usage:
 */

#ifndef _ASM_BITOPS_H
#define _ASM_BITOPS_H
// --------------------------------------------------------------
#include <bsp/type.h>

#define BITOP_BITS_PER_WORD     32
#define BITOP_MASK(nr)          (1UL << ((nr) % BITOP_BITS_PER_WORD))
#define BITOP_WORD(nr)          ((nr) / BITOP_BITS_PER_WORD)
#define BITS_PER_BYTE           8

void set_bit(int nr, volatile void *p);
void clear_bit(int nr, volatile void *p);
void change_bit(int nr, volatile void *p);
int  test_and_set_bit(int nr, volatile void *p);
int  test_and_clear_bit(int nr, volatile void *p);
int  test_and_change_bit(int nr, volatile void *p);
bool set_bit_timeout(int nr, volatile void *p,
                     unsigned int max_try);
bool clear_bit_timeout(int nr, volatile void *p,
                       unsigned int max_try);
bool change_bit_timeout(int nr, volatile void *p,
                        unsigned int max_try);
bool test_and_set_bit_timeout(int nr, volatile void *p,
                              int *oldbit,
                              unsigned int max_try);
bool test_and_clear_bit_timeout(int nr, volatile void *p,
                                int *oldbit,
                                unsigned int max_try);
bool test_and_change_bit_timeout(int nr, volatile void *p,
                                 int *oldbit,
                                 unsigned int max_try);

// --------------------------------------------------------------
static inline int __test_and_set_bit(int nr, volatile void *addr)
{
        unsigned int mask = BITOP_MASK(nr);
        volatile unsigned int *p =
                ((volatile unsigned int *)addr) + BITOP_WORD(nr);
        unsigned int old = *p;

        *p = old | mask;
        return (old & mask) != 0;
}

static inline int __test_and_clear_bit(int nr, volatile void *addr)
{
        unsigned int mask = BITOP_MASK(nr);
        volatile unsigned int *p =
                ((volatile unsigned int *)addr) + BITOP_WORD(nr);
        unsigned int old = *p;

        *p = old & ~mask;
        return (old & mask) != 0;
}

static inline int __test_and_change_bit(int nr,
                                        volatile void *addr)
{
        unsigned int mask = BITOP_MASK(nr);
        volatile unsigned int *p =
                ((volatile unsigned int *)addr) + BITOP_WORD(nr);
        unsigned int old = *p;

        *p = old ^ mask;
        return (old & mask) != 0;
}

static inline int test_bit(int nr, const volatile void *addr)
{
        const volatile unsigned int *p =
            (const volatile unsigned int *)addr;

        return 1UL & (p[BITOP_WORD(nr)] >>
                (nr & (BITOP_BITS_PER_WORD - 1)));
}
// --------------------------------------------------------------
#endif /* _ASM_BITOPS_H */
