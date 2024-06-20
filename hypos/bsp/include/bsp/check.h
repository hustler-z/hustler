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

void __bug_crap(const char *file,
        unsigned int line, const char *function);

void __assert_fail(const char *assertion, const char *file,
        unsigned int line, const char *function);

#define BUG() ({        \
    __bug_crap(__FILE__, __LINE__, __func__); })

#define _DEBUG          (1)

#define ASSERT(x) ({    \
    if (!(x) && _DEBUG) \
		__assert_fail(#x, __FILE__, __LINE__, __func__); })

#define assert_fail(msg) \
    __assert_fail(msg, __FILE__, __LINE__, __func__)

#define ASSERT_UNREACHABLE()    assert_fail("unreachable")
// --------------------------------------------------------------
#endif /* _BSP_CHECK_H */
