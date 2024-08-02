/**
 * Hustler's Project
 *
 * File:  calculate.c
 * Date:  2024/07/14
 * Usage:
 */

#include <org/section.h>
#include <bsp/bootcore.h>
#include <bsp/time.h>
#include <bsp/percpu.h>
#include <bsp/calculate.h>

// --------------------------------------------------------------

static DEFINE_PERCPU(unsigned int, seed);
unsigned int __read_mostly boot_random;

unsigned int get_random(void)
{
    unsigned int next = this_cpu(seed), val = 0;

    if (unlikely(!next))
        next = val ?: NOW();

    if (!val) {
        unsigned int i;

        for (i = 0; i < sizeof(val) * 8; i += 11) {
            next = next * 1103515245 + 12345;
            val |= ((next >> 16) & 0x7FF) << i;
        }
    }

    this_cpu(seed) = next;

    return val;
}

static int __bootfunc boot_random_setup(void)
{
    boot_random = get_random();
    return 0;
}
__bootcall(boot_random_setup);

// --------------------------------------------------------------
