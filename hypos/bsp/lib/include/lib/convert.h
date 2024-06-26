/**
 * Hustler's Project
 *
 * File:  convert.h
 * Date:  2024/05/24
 * Usage:
 */

#ifndef _LIB_CONVERT_H
#define _LIB_CONVERT_H
// --------------------------------------------------------------
unsigned long conv_strtoul(
    const char *cp, const char **endp, unsigned int base);
long conv_strtol(const char *cp, const char **endp, unsigned int base);
// --------------------------------------------------------------
#endif /* _LIB_CONVERT_H */
