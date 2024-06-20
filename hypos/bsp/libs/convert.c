/**
 * Hustler's Project
 *
 * File:  convert.c
 * Date:  2024/06/19
 * Usage:
 */

// --------------------------------------------------------------
#include <lib/ctype.h>
#include <lib/convert.h>

unsigned long conv_strtoul(
    const char *cp, const char **endp, unsigned int base)
{
    unsigned long result = 0, value;

    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((toupper(*cp) == 'X') && isxdigit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    } else if (base == 16) {
        if (cp[0] == '0' && toupper(cp[1]) == 'X')
            cp += 2;
    }

    while (isxdigit(*cp) &&
            (value = isdigit(*cp) ? *cp - '0'
                                  : toupper(*cp) - 'A' + 10) < base) {
        result = result * base + value;
        cp++;
    }

    if ( endp )
        *endp = cp;

    return result;
}

long conv_strtol(const char *cp, const char **endp, unsigned int base)
{
    if (*cp == '-')
        return -conv_strtoul(cp + 1, endp, base);
    return conv_strtoul(cp, endp, base);
}
// --------------------------------------------------------------
