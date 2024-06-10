/**
 * Hustler's Project
 *
 * File:  bootchain.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _BSP_BOOTCHAIN_H
#define _BSP_BOOTCHAIN_H
// --------------------------------------------------------------

#include <bsp/stdio.h>

typedef int (*boot_func_t)(void);

static inline int bsp_bootchain(const boot_func_t boot_sequence[])
{
    const boot_func_t *boot_one;
    int ret;

    for (boot_one = boot_sequence; *boot_one; boot_one++) {
        ret = (*boot_one)();
        if (ret) {
            pr("boot squence %p failed at call %p (err=%d)\n",
                    boot_sequence, (char *)(*boot_one), ret);
            return -1;
        }
    }

    return 0;
}

// --------------------------------------------------------------
#endif /* _BSP_BOOTCHAIN_H */
