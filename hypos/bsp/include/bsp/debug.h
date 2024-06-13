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

#include <bsp/stdio.h>


#define HYPOS_VMM_DEBUG_ON     (1)

#if HYPOS_VMM_DEBUG_ON
#define DEBUG(fmt, ...) \
    pr("<HYPOS> "fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#define MSG(fmt, ...) pr(fmt, ##__VA_ARGS__)


// --------------------------------------------------------------
#endif /* _BSP_DEBUG_H */
