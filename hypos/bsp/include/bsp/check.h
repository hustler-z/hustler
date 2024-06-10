/**
 * Hustler's Project
 *
 * File:  check.h
 * Date:  2024/06/07
 * Usage:
 */

#ifndef _BSP_CHECK_H
#define _BSP_CHECK_H
// --------------------------------------------------------------
void panic(const char *fmt, ...);

void __assert_fail(const char *assertion, const char *file,
        unsigned int line, const char *function);

#define _DEBUG          (1)

#define assert(x) ({    \
    if (!(x) && _DEBUG) \
		__assert_fail(#x, __FILE__, __LINE__, __func__); })

// --------------------------------------------------------------
#endif /* _BSP_CHECK_H */
