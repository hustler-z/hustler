/**
 * Hustler's Project
 *
 * File:  bitops.c
 * Date:  2024/06/20
 * Usage: bitwise operation implementation
 */

#include <asm/bitops.h>
#include <asm/barrier.h>
#include <common/stringify.h>
#include <common/compiler.h>
#include <bsp/panic.h>

// --------------------------------------------------------------

#define bitop(name, instr)                                                  \
static always_inline bool int_##name(int nr, volatile void *p, bool timeout,\
                                     unsigned int max_try)                  \
{                                                                           \
    volatile u32 *ptr = (volatile u32 *)p +                                 \
                             BITOP_WORD((unsigned int)nr);                  \
    const u32 mask = BITOP_MASK((unsigned int)nr);                          \
    unsigned long res, tmp;                                                 \
                                                                            \
    do {                                                                    \
        asm volatile ("// " __stringify(name) "\n"                          \
        "   ldxr    %w2, %1\n"                                              \
        "   " __stringify(instr) "     %w2, %w2, %w3\n"                     \
        "   stxr    %w0, %w2, %1\n"                                         \
        : "=&r" (res), "+Q" (*ptr), "=&r" (tmp)                             \
        : "r" (mask));                                                      \
                                                                            \
        if (!res)                                                           \
            break;                                                          \
    } while (!timeout || ((--max_try) > 0));                                \
                                                                            \
    return !res;                                                            \
}                                                                           \
                                                                            \
void name(int nr, volatile void *p)                                         \
{                                                                           \
    if (!int_##name(nr, p, false, 0))                                       \
        ASSERT_UNREACHABLE();                                               \
}                                                                           \
                                                                            \
bool name##_timeout(int nr, volatile void *p, unsigned int max_try)         \
{                                                                           \
    return int_##name(nr, p, true, max_try);                                \
}

#define testop(name, instr)                                                 \
static always_inline bool int_##name(int nr, volatile void *p, int *oldbit, \
                                     bool timeout, unsigned int max_try)    \
{                                                                           \
    volatile u32 *ptr = (volatile u32 *)p +                                 \
                             BITOP_WORD((unsigned int)nr);                  \
    unsigned int bit = (unsigned int)nr % BITOP_BITS_PER_WORD;              \
    const u32 mask = BITOP_MASK(bit);                                       \
    unsigned long res, tmp;                                                 \
                                                                            \
    do {                                                                    \
        asm volatile ("// " __stringify(name) "\n"                          \
        "   ldxr    %w3, %2\n"                                              \
        "   lsr     %w1, %w3, %w5 // Save old value of bit\n"               \
        "   " __stringify(instr) "  %w3, %w3, %w4 // Toggle bit\n"          \
        "   stlxr   %w0, %w3, %2\n"                                         \
        : "=&r" (res), "=&r" (*oldbit), "+Q" (*ptr), "=&r" (tmp)            \
        : "r" (mask), "r" (bit)                                             \
        : "memory");                                                        \
                                                                            \
        if (!res)                                                           \
            break;                                                          \
    } while (!timeout || ((--max_try) > 0));                                \
                                                                            \
    dmb(ish);                                                               \
                                                                            \
    *oldbit &= 1;                                                           \
                                                                            \
    return !res;                                                            \
}                                                                           \
                                                                            \
int name(int nr, volatile void *p)                                          \
{                                                                           \
    int oldbit;                                                             \
                                                                            \
    if (!int_##name(nr, p, &oldbit, false, 0))                              \
        ASSERT_UNREACHABLE();                                               \
                                                                            \
    return oldbit;                                                          \
}                                                                           \
                                                                            \
bool name##_timeout(int nr, volatile void *p,                               \
                    int *oldbit, unsigned int max_try)                      \
{                                                                           \
    return int_##name(nr, p, oldbit, true, max_try);                        \
}

bitop(change_bit, eor)
bitop(clear_bit, bic)
bitop(set_bit, orr)

testop(test_and_change_bit, eor)
testop(test_and_clear_bit, bic)
testop(test_and_set_bit, orr)
// --------------------------------------------------------------
