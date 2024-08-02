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
#include <org/section.h>
#include <bsp/compiler.h>
#include <bsp/type.h>
#include <bsp/config.h>
#include <lib/args.h>

// --------------------------------------------------------------
void vpr_common(const char *fmt, va_list args);
void pr_hypos(const char *fmt, ...);
// --------------------------------------------------------------

#if CFG_HYPOS_DEBUG_ON
#define DEBUG(fmt, ...) \
    pr_hypos("[debug] "fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

/* For Better LOG Alignment */
#define BLANK_ALIGN    "        "

#define MSGH(fmt, ...) pr_hypos("[hypos] "fmt, ##__VA_ARGS__)
#define MSGI(fmt, ...) pr_hypos(fmt, ##__VA_ARGS__)
#define MSGE(fmt, ...) pr_hypos("[error] "fmt, ##__VA_ARGS__)
#define MSGQ(cond, fmt, ...)                  \
    do {                                      \
        if (cond)                             \
            pr_hypos("[query] "fmt, ##__VA_ARGS__); \
    } while (0)

#define MSGO(fmt, ...)                        \
    do {                                      \
        static bool __read_mostly _once;      \
        if (unlikely(!_once)) {               \
            _once = true;                     \
            pr_hypos("[first] "fmt, ##__VA_ARGS__); \
        }                                     \
    } while (0)

#define __ACCESS_ONCE(x) ({                             \
            (void)(typeof(x))0; /* Scalar typecheck. */ \
            (volatile typeof(x) *)&(x); })
#define ACCESS_ONCE(x) (*__ACCESS_ONCE(x))

int sscanf(const char *buf, const char *fmt, ...);
// --------------------------------------------------------------
#endif /* _BSP_DEBUG_H */
