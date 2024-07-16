/**
 * Hustler's Project
 *
 * File:  core.c
 * Date:  2024/07/12
 * Usage:
 */

#include <org/section.h>

// --------------------------------------------------------------

extern const bootcall_t __bootcalls_start[], __bootcalls_end[];

void __bootfunc bootcalls(void)
{
    const bootcall_t *call;

    for (call = __bootcalls_start; call < __bootcalls_end; call++)
        (*call)();
}

// --------------------------------------------------------------
