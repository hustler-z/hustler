/**
 * Hustler's Project
 *
 * File:  args.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _LIB_ARGS_H
#define _LIB_ARGS_H
// ------------------------------------------------------------------------

#define va_start(v,l)       __builtin_va_start((v),l)
#define va_end              __builtin_va_end
#define va_arg              __builtin_va_arg
typedef __builtin_va_list   va_list;

// ------------------------------------------------------------------------
#endif /* _LIB_ARGS_H */
