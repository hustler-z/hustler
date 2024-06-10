/**
 * Hustler's Project
 *
 * File:  math.c
 * Date:  2024/05/20
 * Usage:
 */

#include <lib/math.h>

// ------------------------------------------------------------------------
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
// ------------------------------------------------------------------------
