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
unsigned long _strtoul(const char *cp, const char **endp,
                       unsigned int base);
unsigned long __strtoul(const char *cp, char **endp,
        unsigned int base);
unsigned long __hextoul(const char *cp, char **endp);
// --------------------------------------------------------------
#endif /* _LIB_CONVERT_H */
