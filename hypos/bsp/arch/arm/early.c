/**
 * Hustler's Project
 *
 * File:  info.c
 * Date:  2024/06/15
 * Usage: early debug info
 */

#include <asm/debug.h>
#include <bsp/type.h>
#include <lib/strops.h>

// --------------------------------------------------------------
void early_debug(const char *s)
{
    size_t len = strlen(s);

    while (len-- > 0) {
        if (*s == '\n')
            early_putc('\r');
        early_putc(*s);
        s++;
    }

    early_flush();
}
// --------------------------------------------------------------
