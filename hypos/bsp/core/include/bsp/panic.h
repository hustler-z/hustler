/**
 * Hustler's Project
 *
 * File:  panic.h
 * Date:  2024/06/07
 * Usage:
 */

#ifndef _BSP_PANIC_H
#define _BSP_PANIC_H
// --------------------------------------------------------------

#include <common/compiler.h>
#include <common/type.h>

void panic(bool in_exception, const char *fmt, ...);

#define trap_panic(fmt, ...) \
    panic(true, fmt, ##__VA_ARGS__)

#define exec_panic(fmt, ...) \
    panic(false, fmt, ##__VA_ARGS__)

void __bug_crap(const char *assertion, const char *file,
        unsigned int line, const char *function);
void __warn_crap(const char *assertion, const char *file,
        unsigned int line, const char *function);
void __assert_crap(const char *assertion, const char *file,
        unsigned int line, const char *function);

#define assert_fail(msg) \
    __assert_crap(msg, __FILE__, __LINE__, __func__)

#define BUG() ({        \
    __bug_crap("BUG", __FILE__, __LINE__, __func__); })

#define WARN_ON(c) ({   \
    if (unlikely(c))    \
        __warn_crap(#c, __FILE__, __LINE__, __func__); })

/* When shit is real, do BUG()
 */
#define BUG_ON(c) do {                                \
    if (unlikely(c))                                  \
        __bug_crap(#c, __FILE__, __LINE__, __func__); \
} while (0)

/* When shit ain't real, do ASSERT()
 */
#define ASSERT(x) ({    \
    if (unlikely(!(x))) \
        __assert_crap(#x, __FILE__, __LINE__, __func__); })

#define ASSERT_UNREACHABLE()    assert_fail("unreachable")
// --------------------------------------------------------------
#endif /* _BSP_PANIC_H */
