/**
 * Hustler's Project
 *
 * File:  core.h
 * Date:  2024/07/12
 * Usage:
 */

#ifndef _BSP_BOOTCORE_H
#define _BSP_BOOTCORE_H
// --------------------------------------------------------------
#include <bsp/compiler.h>

typedef int (*bootcall_t)(void);

#define __bootcall(fn) \
    const static bootcall_t __bootcall_##fn __boot_call("1") = (fn)

int bootcalls(void);

// --------------------------------------------------------------
#endif /* _BSP_BOOTCORE_H */
