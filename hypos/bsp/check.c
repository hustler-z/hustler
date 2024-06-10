/**
 * Hustler's Project
 *
 * File:  check.c
 * Date:  2024/06/07
 * Usage: system execution checker
 */

#include <generic/exit.h>
#include <bsp/stdio.h>
#include <lib/args.h>

// --------------------------------------------------------------
static void panic_finish(void) __attribute__ ((noreturn));

static void panic_finish(void)
{
    putc('\n');

    hang();

    while (1)
        ;
}

void panic_str(const char *str)
{
    puts(str);
    panic_finish();
}

void panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vpr(fmt, args);
    va_end(args);

    panic_finish();
}

void __assert_fail(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    /* This will not return */
    panic("%s:%u: %s: Assertion `%s' failed.", file, line,
        function, assertion);
}
// --------------------------------------------------------------
