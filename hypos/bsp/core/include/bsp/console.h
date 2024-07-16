/**
 * Hustler's Project
 *
 * File:  console.h
 * Date:  2024/05/20
 * Usage: general peripheral initialization
 */

#ifndef _BSP_CONSOLE_H
#define _BSP_CONSOLE_H
// --------------------------------------------------------------

#include <bsp/type.h>
#include <bsp/timer.h>

#define TERM_CH(c)       ((c) - 'a' + 1)

int serial_pr(const char *fmt, ...);

int console_assign(int file, const char *devname);
int console_setup(void);

// --------------------------------------------------------------
#endif /* _BSP_CONSOLE_H */
