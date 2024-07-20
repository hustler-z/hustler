/**
 * Hustler's Project
 *
 * File:  math.c
 * Date:  2024/05/20
 * Usage:
 */

#include <lib/math.h>
#include <bsp/config.h>

// --------------------------------------------------------------
static unsigned int y = 1U;

unsigned int rand_r(unsigned int *seedp)
{
    *seedp ^= (*seedp << 13);
    *seedp ^= (*seedp >> 17);
    *seedp ^= (*seedp << 5);

    return *seedp;
}

unsigned int rand(void)
{
    return rand_r(&y);
}

void srand(unsigned int seed)
{
    y = seed;
}

/* Compute with 96 bit intermediate result: (a*b)/c */
u64 muldiv64(u64 a, u32 b, u32 c)
{
    union {
        u64 ll;
        struct {
#if IS_ENABLED(CFG_WORDS_BIGENDIAN)
            u32 high, low;
#else
            u32 low, high;
#endif
        } l;
    } u, res;
    u64 rl, rh;

    u.ll = a;
    rl = (u64)u.l.low * (u64)b;
    rh = (u64)u.l.high * (u64)b;
    rh += (rl >> 32);
    res.l.high = rh / c;
    res.l.low = (((rh % c) << 32) + (u32)rl) / c;

    return res.ll;
}
// --------------------------------------------------------------
