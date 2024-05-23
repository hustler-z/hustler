/**
 * Hustler's Project
 *
 * File:  bsp_print.c
 * Date:  2024/05/20
 * Usage:
 */

#include <bsp_print.h>
#include <board_uart.h>

#define va_start(v,l)       __builtin_va_start((v),l)
#define va_end              __builtin_va_end
#define va_arg              __builtin_va_arg
typedef __builtin_va_list   va_list;

int _bsp_print()
{
    int ret = 0;
    /* To Do */
    return ret;
}

int bsp_print(const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = _bsp_print();
    va_end(args);

    return ret;
}
