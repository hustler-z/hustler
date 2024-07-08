/**
 * Hustler's Project
 *
 * File:  debug.h
 * Date:  2024/06/14
 * Usage:
 */

#ifndef _BSP_DEBUG_H
#define _BSP_DEBUG_H
// --------------------------------------------------------------
#include <asm-generic/section.h>
#include <bsp/stdio.h>
#include <common/compiler.h>
#include <common/type.h>

#define HYPOS_VMM_DEBUG_ON     (1)

#if HYPOS_VMM_DEBUG_ON
#define DEBUG(fmt, ...) \
    pr("[debug] "fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

/* For Better LOG Alignment */
#define BLANK_ALIGN    "        "

#define MSGH(fmt, ...) pr("[hypos] "fmt, ##__VA_ARGS__)
#define MSGI(fmt, ...) pr(fmt, ##__VA_ARGS__)
#define MSGE(fmt, ...) pr("[error] "fmt, ##__VA_ARGS__)
#define MSGQ(cond, fmt, ...)                  \
    do {                                      \
        if (cond)                             \
            pr("[query] "fmt, ##__VA_ARGS__); \
    } while (0)

#define MSGO(fmt, ...)                        \
    do {                                      \
        static bool __read_mostly _once;      \
        if (unlikely(!_once)) {               \
            _once = true;                     \
            pr("[first] "fmt, ##__VA_ARGS__); \
        }                                     \
    } while (0)

int sscanf(const char *buf, const char *fmt, ...);
// --------------------------------------------------------------
#endif /* _BSP_DEBUG_H */
