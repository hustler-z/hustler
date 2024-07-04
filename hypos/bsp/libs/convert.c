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

unsigned long _strtoul(const char *cp, const char **endp,
                       unsigned int base)
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

    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp - '0'
           : toupper(*cp) - 'A' + 10) < base) {
        result = result * base + value;
        cp++;
    }

    if ( endp )
        *endp = cp;

    return result;
}

static unsigned int decode_digit(int ch)
{
    if (!isxdigit(ch))
        return 256;

    ch = tolower(ch);

    return ch <= '9' ? ch - '0' : ch - 'a' + 0xa;
}

static const char *_parse_integer_fixup_radix(const char *s,
        unsigned int *basep)
{
    /* Look for a 0x prefix */
    if (s[0] == '0') {
        int ch = tolower(s[1]);

        if (ch == 'x') {
            *basep = 16;
            s += 2;
        } else if (!*basep)
            *basep = 8;
    }

    if (!*basep)
        *basep = 10;

    return s;
}

unsigned long __strtoul(const char *cp, char **endp,
        unsigned int base)
{
    unsigned long result = 0;
    unsigned int value;

    cp = _parse_integer_fixup_radix(cp, &base);

    while (value = decode_digit(*cp), value < base) {
        result = result * base + value;
        cp++;
    }

    if (endp)
        *endp = (char *)cp;

    return result;
}

unsigned long __hextoul(const char *cp, char **endp)
{
    return __strtoul(cp, endp, 16);
}
// --------------------------------------------------------------
