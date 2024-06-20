/**
 * Hustler's Project
 *
 * File:  check.c
 * Date:  2024/06/07
 * Usage: system execution checker
 */

#include <generic/exit.h>
#include <bsp/debug.h>
#include <lib/args.h>

// --------------------------------------------------------------
static void panic_end(void)
{
    MSGI("\n");

    hang();

    flush();
}

void panic_str(const char *str)
{
    MSGI(str);
    panic_end();
}

void panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vpr(fmt, args);
    va_end(args);

    panic_end();
}

void __bug_crap(const char *file,
        unsigned int line, const char *function)
{
    panic("BUG %s:%u - %s() man, ain't this a joke.", file, line,
            function);
}

void __assert_fail(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    panic("ASSERT %s:%u - %s() `%s' failed.", file, line,
            function, assertion);
}
// --------------------------------------------------------------
