/**
 * Hustler's Project
 *
 * File:  check.c
 * Date:  2024/06/07
 * Usage: system execution checker
 */

#include <common/exit.h>
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
    vpr_common(fmt, args);
    va_end(args);

    panic_end();
}

void warn(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vpr_common(fmt, args);
    va_end(args);
}

void __warn_crap(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    warn("[warns] %s %u - %s() * %s * Bombed", file, line,
            function, assertion);
}

void __bug_crap(const char *file,
        unsigned int line, const char *function)
{
    panic("[panic] %s %u - %s() F*cked up", file, line,
            function);
}

void __assert_crap(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    panic("[panic] %s %u - %s() * %s * Bombed", file, line,
            function, assertion);
}
// --------------------------------------------------------------
