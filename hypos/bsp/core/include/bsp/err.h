/**
 * Hustler's Project
 *
 * File:  err.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _COMMON_ERR_H
#define _COMMON_ERR_H
// --------------------------------------------------------------
#include <bsp/compiler.h>

#define ERR_PTR_OFFSET    0x0
#define MAX_ERRNO         4095

#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline void *ERR_PTR(long error)
{
    return (void *)(ERR_PTR_OFFSET + error);
}

static inline long PTR_ERR(const void *ptr)
{
    return ((long)ptr - ERR_PTR_OFFSET);
}

static inline long IS_ERR(const void *ptr)
{
    return IS_ERR_VALUE((unsigned long)PTR_ERR(ptr));
}

static const char error_message[] = "";

static inline const char *errno_str(int errno)
{
    return error_message;
}

// --------------------------------------------------------------
#endif /* _COMMON_ERR_H */
