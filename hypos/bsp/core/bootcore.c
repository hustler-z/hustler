/**
 * Hustler's Project
 *
 * File:  core.c
 * Date:  2024/07/12
 * Usage:
 */

#include <org/section.h>
#include <bsp/bootcore.h>

// --------------------------------------------------------------

extern const bootcall_t __bootcalls_start[], __bootcalls_end[];

int __bootfunc bootcalls(void)
{
    const bootcall_t *call;

    for (call = __bootcalls_start; call < __bootcalls_end; call++)
        (*call)();

    return 0;
}

// --------------------------------------------------------------
