/*
 * Hustler's Project
 *
 * File:  print.c
 * Date:  2024/05/20
 * Usage:
 */

#include <bsp/stdio.h>
#include <bsp/alloc.h>
#include <lib/ctype.h>
#include <lib/math.h>
#include <lib/strops.h>
#include <generic/errno.h>
#include <lib/args.h>

// --------------------------------------------------------------
extern struct board_uart_dev serial;

#define is_digit(c)	((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
    int i = 0;

    while (is_digit(**s))
        i = i * 10 + *((*s)++) - '0';

    return i;
}

static char *put_dec_trunc(char *buf, unsigned q)
{
    unsigned d3, d2, d1, d0;
    d1 = (q >> 4) & 0xf;
    d2 = (q >> 8) & 0xf;
    d3 = (q >> 12);

    d0 = 6 * (d3 + d2 + d1) + (q & 0xF);
    q = (d0 * 0xCD) >> 11;
    d0 = d0 - 10 * q;
    *buf++ = d0 + '0'; /* least significant digit */
    d1 = q + 9 * d3 + 5 * d2 + d1;
    if (d1 != 0) {
        q = (d1 * 0xCD) >> 11;
        d1 = d1 - 10 * q;
        *buf++ = d1 + '0'; /* next digit */

        d2 = q + 2 * d2;
        if ((d2 != 0) || (d3 != 0)) {
            q = (d2 * 0xD) >> 7;
            d2 = d2 - 10 * q;
            *buf++ = d2 + '0'; /* next digit */

            d3 = q + 4 * d3;
            if (d3 != 0) {
                q = (d3 * 0xCD) >> 11;
                d3 = d3 - 10 * q;
                *buf++ = d3 + '0';  /* next digit */
                if (q != 0)
                    *buf++ = q + '0'; /* most sign. digit */
            }
        }
    }
    return buf;
}
/* Same with if's removed. Always emits five digits */
static char *put_dec_full(char *buf, unsigned q)
{
    /* BTW, if q is in [0,9999], 8-bit ints will be enough, */
    /* but anyway, gcc produces better code with full-sized ints */
    unsigned d3, d2, d1, d0;
    d1 = (q >> 4) & 0xf;
    d2 = (q >> 8) & 0xf;
    d3 = (q >> 12);

    /*
     * Possible ways to approx. divide by 10
     * gcc -O2 replaces multiply with shifts and adds
     * (x * 0xcd) >> 11: 11001101 - shorter code than * 0x67 (on i386)
     * (x * 0x67) >> 10:  1100111
     * (x * 0x34) >> 9:    110100 - same
     * (x * 0x1a) >> 8:     11010 - same
     * (x * 0x0d) >> 7:      1101 - same, shortest code (on i386)
     */

    d0 = 6 * (d3 + d2 + d1) + (q & 0xf);
    q = (d0 * 0xCD) >> 11;
    d0 = d0 - 10 * q;
    *buf++ = d0 + '0';
    d1 = q + 9 * d3 + 5 * d2 + d1;
        q = (d1 * 0xCD) >> 11;
        d1 = d1 - 10 * q;
        *buf++ = d1 + '0';

        d2 = q + 2 * d2;
            q = (d2 * 0xD) >> 7;
            d2 = d2 - 10 * q;
            *buf++ = d2 + '0';

            d3 = q + 4 * d3;
                q = (d3 * 0xCD) >> 11; /* - shorter code */

                d3 = d3 - 10 * q;
                *buf++ = d3 + '0';
                    *buf++ = q + '0';
    return buf;
}

static char *put_dec(char *buf, u64 num)
{
    while (1) {
        unsigned rem;
        if (num < 100000)
            return put_dec_trunc(buf, num);
        rem = do_div(num, 100000);
        buf = put_dec_full(buf, rem);
    }
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* Must be 32 == 0x20 */
#define SPECIAL	64		/* 0x */
#define ERRSTR	128		/* %dE showing error string if enabled */

#define ADDCH(str, ch) do { \
    if ((str) < end)        \
        *(str) = (ch);      \
    ++str;                  \
    } while (0)

static char *number(char *buf, char *end, u64 num,
		int base, int size, int precision, int type)
{
    static const char digits[16] = "0123456789ABCDEF";

    char tmp[66];
    char sign;
    char locase;
    int need_pfx = ((type & SPECIAL) && base != 10);
    int i;

    /* locase = 0 or 0x20. ORing digits or letters with 'locase'
     * produces same digits or (maybe lowercased) letters */
    locase = (type & SMALL);
    if (type & LEFT)
        type &= ~ZEROPAD;
    sign = 0;
    if (type & SIGN) {
        if ((s64) num < 0) {
            sign = '-';
            num = -(s64) num;
            size--;
        } else if (type & PLUS) {
            sign = '+';
            size--;
        } else if (type & SPACE) {
            sign = ' ';
            size--;
        }
    }
    if (need_pfx) {
        size--;
        if (base == 16)
            size--;
    }

    /* generate full string in tmp[], in reverse order */
    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    /* Generic code, for any base:
    else do {
        tmp[i++] = (digits[do_div(num,base)] | locase);
    } while (num != 0);
    */
    else if (base != 10) { /* 8 or 16 */
        int mask = base - 1;
        int shift = 3;

        if (base == 16)
            shift = 4;

        do {
            tmp[i++] = (digits[((unsigned char)num) & mask]
                    | locase);
            num >>= shift;
        } while (num);
    } else { /* base 10 */
        i = put_dec(tmp, num) - tmp;
    }

    /* printing 100 using %2d gives "100", not "00" */
    if (i > precision)
        precision = i;
    /* leading space padding */
    size -= precision;
    if (!(type & (ZEROPAD + LEFT))) {
        while (--size >= 0)
            ADDCH(buf, ' ');
    }
    /* sign */
    if (sign)
        ADDCH(buf, sign);
    /* "0x" / "0" prefix */
    if (need_pfx) {
        ADDCH(buf, '0');
        if (base == 16)
            ADDCH(buf, 'X' | locase);
    }
    /* zero or space padding */
    if (!(type & LEFT)) {
        char c = (type & ZEROPAD) ? '0' : ' ';

        while (--size >= 0)
            ADDCH(buf, c);
    }
    /* hmm even more zero padding? */
    while (i <= --precision)
        ADDCH(buf, '0');
    /* actual digits of result */
    while (--i >= 0)
        ADDCH(buf, tmp[i]);
    /* trailing space padding */
    while (--size >= 0)
        ADDCH(buf, ' ');
    return buf;
}

static char *string(char *buf, char *end, const char *s, int field_width,
		int precision, int flags)
{
    int len, i;

    if (s == NULL)
        s = "<NULL>";

    len = strnlen(s, precision);

    if (!(flags & LEFT))
        while (len < field_width--)
            ADDCH(buf, ' ');
    for (i = 0; i < len; ++i)
        ADDCH(buf, *s++);
    while (len < field_width--)
        ADDCH(buf, ' ');
    return buf;
}

static char *pointer(const char *fmt, char *buf, char *end, void *ptr,
		int field_width, int precision, int flags)
{
    u64 num = (uptr_t)ptr;

    switch (*fmt) {
    case 'a':
        flags |= SPECIAL | ZEROPAD;

        switch (fmt[1]) {
        case 'p':
        default:
            field_width = sizeof(paddr_t) * 2 + 2;
            num = *(paddr_t *)ptr;
            break;
        }
        break;
    case 'm':
        flags |= SPECIAL;
        /* Fallthrough */
    case 'i':
        flags |= SPECIAL;
        /* Fallthrough */
    default:
        break;
    }
    flags |= SMALL;
    if (field_width == -1) {
        field_width = 2*sizeof(void *);
        flags |= ZEROPAD;
    }
    return number(buf, end, num, 16, field_width, precision, flags);
}

static int vsnpr_internal(char *buf, size_t size, const char *fmt,
			      va_list args)
{
    u64 num;
    int base;
    char *str;

    int flags;		    /* flags to number() */

    int field_width;	/* width of output field */
    int precision;		/* min. # of digits for integers; max
                           number of chars for from string */
    int qualifier;		/* 'h', 'l', or 'L' for integer fields */
                        /* 'z' support added 23/7/1999 S.H.    */
                        /* 'z' changed to 'Z' --davidm 1/25/99 */
                        /* 't' added for ptrdiff_t */
    char *end = buf + size;

    if (end < buf) {
        end = ((void *) - 1);
        size = end - buf;
    }
    str = buf;

    for (; *fmt ; ++fmt) {
        if (*fmt != '%') {
            ADDCH(str, *fmt);
            continue;
        }

        /* process flags */
        flags = 0;
repeat:
        ++fmt;		/* this also skips first '%' */
        switch (*fmt) {
        case '-':
            flags |= LEFT;
            goto repeat;
        case '+':
            flags |= PLUS;
            goto repeat;
        case ' ':
            flags |= SPACE;
            goto repeat;
        case '#':
            flags |= SPECIAL;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        /* get field width */
        field_width = -1;
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (is_digit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*') {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
            *fmt == 'Z' || *fmt == 'z' || *fmt == 't') {
            qualifier = *fmt;
            ++fmt;
            if (qualifier == 'l' && *fmt == 'l') {
                qualifier = 'L';
                ++fmt;
            }
        }

        /* default base */
        base = 10;

        switch (*fmt) {
        case 'c':
            if (!(flags & LEFT)) {
                while (--field_width > 0)
                    ADDCH(str, ' ');
            }
            ADDCH(str, (unsigned char) va_arg(args, int));
            while (--field_width > 0)
                ADDCH(str, ' ');
            continue;

        case 's':
            {
                str = string(str, end, va_arg(args, char *),
                         field_width, precision, flags);
            }
            continue;

        case 'p':
            str = pointer(fmt + 1, str, end,
                    va_arg(args, void *),
                    field_width, precision, flags);
            if (IS_ERR(str))
                return PTR_ERR(str);
            /* Skip all alphanumeric pointer suffixes */
            while (isalnum(fmt[1]))
                fmt++;
            continue;

        case 'n':
            if (qualifier == 'l') {
                long *ip = va_arg(args, long *);
                *ip = (str - buf);
            } else {
                int *ip = va_arg(args, int *);
                *ip = (str - buf);
            }
            continue;

        case '%':
            ADDCH(str, '%');
            continue;

        /* integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'x':
            flags |= SMALL;
        /* fallthrough */
        case 'X':
            base = 16;
            break;

        case 'd':
            if (fmt[1] == 'E') {
                flags |= ERRSTR;
                fmt++;
            }
        /* fallthrough */
        case 'i':
            flags |= SIGN;
        /* fallthrough */
        case 'u':
            break;

        default:
            ADDCH(str, '%');
            if (*fmt)
                ADDCH(str, *fmt);
            else
                --fmt;
            continue;
        }
        if (qualifier == 'L')  /* "quad" for 64 bit variables */
            num = va_arg(args, unsigned long long);
        else if (qualifier == 'l') {
            num = va_arg(args, unsigned long);
            if (flags & SIGN)
                num = (signed long) num;
        } else if (qualifier == 'Z' || qualifier == 'z') {
            num = va_arg(args, size_t);
        } else if (qualifier == 't') {
            num = va_arg(args, ptrdiff_t);
        } else if (qualifier == 'h') {
            num = (unsigned short) va_arg(args, int);
            if (flags & SIGN)
                num = (signed short) num;
        } else {
            num = va_arg(args, unsigned int);
            if (flags & SIGN)
                num = (signed int) num;
        }
        str = number(str, end, num, base, field_width, precision,
                 flags);
        if (flags & ERRSTR) {
            const char *p;

            ADDCH(str, ':');
            ADDCH(str, ' ');
            for (p = errno_str(num); *p; p++)
                ADDCH(str, *p);
        }
    }

    if (size > 0) {
        ADDCH(str, '\0');
        if (str > end)
            end[-1] = '\0';
        --str;
    }

    /* the trailing null byte doesn't count towards the total */
    return str - buf;
}

int vsnpr(char *buf, size_t size, const char *fmt,
        va_list args)
{
    return vsnpr_internal(buf, size, fmt, args);
}

int vscnpr(char *buf, size_t size, const char *fmt,
        va_list args)
{
    int ret;

    ret = vsnpr(buf, size, fmt, args);

    if (likely(ret < size))
        return ret;
    if (size != 0)
        return size - 1;

    return 0;
}

int snpr(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnpr(buf, size, fmt, args);
    va_end(args);

    return i;
}

int vspr(char *buf, const char *fmt, va_list args)
{
    return vsnpr_internal(buf, INT_MAX, fmt, args);
}

int spr(char *buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vspr(buf, fmt, args);
    va_end(args);
    return i;
}

int vpr(const char *fmt, va_list args)
{
    int ret;
    char buf[HYPOS_PBSIZE];

    ret = vscnpr(buf, sizeof(buf), fmt, args);

    if (ret <= 0)
        return ret;

    puts(buf);

    return ret;
}

int pr(const char *fmt, ...)
{
    va_list args;
    int ret;

    va_start(args, fmt);
    ret = vpr(fmt, args);
    va_end(args);

    return ret;
}

// ------------------------------------------------------------------------

/* SSCANF IMPLEMENTATION
 */

#define __DECONST(type, var)  ((type)(uptr_t)(const void *)(var))

struct str_info {
    int neg, any;
    u64 acc;
};

static struct str_info *
str_to_int_convert(const char **nptr, int base, unsigned int unsign)
{
    const char *s = *nptr;
    u64 acc;
    unsigned char c;
    u64 cutoff;
    int neg, any, cutlim;
    u64 qbase;
    struct str_info *info;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    info = (struct str_info *)balloc(sizeof(struct str_info));
    if (!info)
        return NULL;

    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    qbase = (unsigned int)base;

    if (!unsign) {
        cutoff = neg ? (u64)-(LLONG_MIN + LLONG_MAX) + LLONG_MAX : LLONG_MAX;
        cutlim = cutoff % qbase;
        cutoff /= qbase;
    } else {
        cutoff = (u64)ULLONG_MAX / qbase;
        cutlim = (u64)ULLONG_MAX % qbase;
    }

    for (acc = 0, any = 0;; c = *s++) {
        if (!isascii(c))
            break;
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= qbase;
            acc += c;
        }
    }

    info->any = any;
    info->neg = neg;
    info->acc = acc;

    *nptr = s;

    return info;
}

static s64
strtoq(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    u64 acc;
    int unsign = 0;
    struct str_info *info;

    info = str_to_int_convert(&s, base, unsign);
    if (!info)
        return -1;

    acc = info->acc;

    if (info->any < 0)
        acc = info->neg ? LLONG_MIN : LLONG_MAX;
    else if (info->neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = __DECONST(char *, info->any ? s - 1 : nptr);

    bfree(info);

    return acc;
}

u64
strtouq(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    u64 acc;
    int unsign = 1;
    struct str_info *info;

    info = str_to_int_convert(&s, base, unsign);
    if (!info)
        return -1;

    acc = info->acc;

    if (info->any < 0)
        acc = ULLONG_MAX;
    else if (info->neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = __DECONST(char *, info->any ? s - 1 : nptr);

    bfree(info);

    return acc;
}

static const unsigned char *
__sccl(char *tab, const unsigned char *fmt)
{
    int c, n, v;

    /* first `clear' the whole table */
    c = *fmt++;             /* first char hat => negated scanset */
    if (c == '^') {
        v = 1;          /* default => accept */
        c = *fmt++;     /* get new first char */
    } else {
        v = 0;          /* default => reject */
    }

    /* XXX: Will not work if sizeof(tab*) > sizeof(char) */
    for (n = 0; n < 256; n++)
        tab[n] = v;        /* memset(tab, v, 256) */

    if (c == 0)
        return (fmt - 1);/* format ended before closing ] */

    /*
     * Now set the entries corresponding to the actual scanset
     * to the opposite of the above.
     *
     * The first character may be ']' (or '-') without being special;
     * the last character may be '-'.
     */
    v = 1 - v;
    for (;;) {
        tab[c] = v;             /* take character c */
    doswitch:
        n = *fmt++;             /* and examine the next */
        switch (n) {
        case 0:                 /* format ended too soon */
            return (fmt - 1);

        case '-':
            /*
             * A scanset of the form
             *      [01+-]
             * is defined as `the digit 0, the digit 1,
             * the character +, the character -', but
             * the effect of a scanset such as
             *      [a-zA-Z0-9]
             * is implementation defined.  The V7 Unix
             * scanf treats `a-z' as `the letters a through
             * z', but treats `a-a' as `the letter a, the
             * character -, and the letter a'.
             *
             * For compatibility, the `-' is not considerd
             * to define a range if the character following
             * it is either a close bracket (required by ANSI)
             * or is not numerically greater than the character
             * we just stored in the table (c).
             */
            n = *fmt;
            if (n == ']' || n < c) {
                c = '-';
                break;  /* resume the for(;;) */
            }
            fmt++;
            /* fill in the range */
            do {
                tab[++c] = v;
            } while (c < n);
            c = n;
            /*
             * Alas, the V7 Unix scanf also treats formats
             * such as [a-c-e] as `the letters a through e'.
             * This too is permitted by the standard....
             */
            goto doswitch;
            break;

        case ']':               /* end of scanset */
            return (fmt);

        default:                /* just another character */
            c = n;
            break;
        }
    }
/* NOTREACHED */
}

/**
 * vsscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	format of buffer
 * @args:	arguments
 */
#define BUF             32      /* Maximum length of numeric string. */

/*
 * Flags used during conversion.
 */
#define LONG            0x01    /* l: long or double */
#define SHORT           0x04    /* h: short */
#define SUPPRESS        0x08    /* suppress assignment */
#define POINTER         0x10    /* weird %p pointer (`fake hex') */
#define NOSKIP          0x20    /* do not skip blanks */
#define QUAD            0x400
#define SHORTSHORT      0x4000  /** hh: char */

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */
#define SIGNOK          0x40    /* +/- is (still) legal */
#define NDIGITS         0x80    /* no digits detected */

#define DPTOK           0x100   /* (float) decimal point is still legal */
#define EXPOK           0x200   /* (float) exponent (e+3, etc) still legal */

#define PFXOK           0x100   /* 0x prefix is (still) legal */
#define NZDIGITS        0x200   /* no zero digits detected */

/*
 * Conversion types.
 */
#define CT_CHAR         0       /* %c conversion */
#define CT_CCL          1       /* %[...] conversion */
#define CT_STRING       2       /* %s conversion */
#define CT_INT          3       /* integer, i.e., strtoq or strtouq */

typedef u64 (*ccfntype)(const char *, char **, int);

int
vsscanf(const char *inp, char const *fmt0, va_list ap)
{
    int inr;
    const unsigned char *fmt = (const unsigned char *)fmt0;
    int c;                  /* character from format, or conversion */
    size_t width;           /* field width, or 0 */
    char *p;                /* points into all kinds of strings */
    int n;                  /* handy integer */
    int flags;              /* flags as defined above */
    char *p0;               /* saves original value of p when necessary */
    int nassigned;          /* number of fields assigned */
    int nconversions;       /* number of conversions */
    int nread;              /* number of characters consumed from fp */
    int base;               /* base argument to strtoq/strtouq */
    ccfntype ccfn;          /* conversion function (strtoq/strtouq) */
    char ccltab[256];       /* character class table for %[...] */
    char buf[BUF];          /* buffer for numeric conversions */

    /* `basefix' is used to avoid `if' tests in the integer scanner */
    static short basefix[17] = { 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                     12, 13, 14, 15, 16 };

    inr = strlen(inp);

    nassigned = 0;
    nconversions = 0;
    nread = 0;
    base = 0;
    ccfn = NULL;
    for (;;) {
        c = *fmt++;
        if (c == 0)
            return (nassigned);
        if (isspace(c)) {
            while (inr > 0 && isspace(*inp))
                nread++, inr--, inp++;
            continue;
        }
        if (c != '%')
            goto literal;
        width = 0;
        flags = 0;
        /*
         * switch on the format.  continue if done;
         * break once format type is derived.
         */
again:
        c = *fmt++;
        switch (c) {
        case '%':
literal:
            if (inr <= 0)
                goto input_failure;
            if (*inp != c)
                goto match_failure;
            inr--, inp++;
            nread++;
            continue;

        case '*':
            flags |= SUPPRESS;
            goto again;
        case 'l':
            if (flags & LONG) {
                flags &= ~LONG;
                flags |= QUAD;
            } else {
                flags |= LONG;
            }
            goto again;
        case 'q':
            flags |= QUAD;
            goto again;
        case 'h':
            if (flags & SHORT) {
                flags &= ~SHORT;
                flags |= SHORTSHORT;
            } else {
                flags |= SHORT;
            }
            goto again;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            width = width * 10 + c - '0';
            goto again;

        /*
         * Conversions.
         *
         */
        case 'd':
            c = CT_INT;
            ccfn = (ccfntype)strtoq;
            base = 10;
            break;

        case 'i':
            c = CT_INT;
            ccfn = (ccfntype)strtoq;
            base = 0;
            break;

        case 'o':
            c = CT_INT;
            ccfn = strtouq;
            base = 8;
            break;

        case 'u':
            c = CT_INT;
            ccfn = strtouq;
            base = 10;
            break;

        case 'x':
            flags |= PFXOK; /* enable 0x prefixing */
            c = CT_INT;
            ccfn = strtouq;
            base = 16;
            break;

        case 's':
            c = CT_STRING;
            break;

        case '[':
            fmt = __sccl(ccltab, fmt);
            flags |= NOSKIP;
            c = CT_CCL;
            break;

        case 'c':
            flags |= NOSKIP;
            c = CT_CHAR;
            break;

        case 'p':       /* pointer format is like hex */
            flags |= POINTER | PFXOK;
            c = CT_INT;
            ccfn = strtouq;
            base = 16;
            break;

        case 'n':
            nconversions++;
            if (flags & SUPPRESS)   /* ??? */
                continue;
            if (flags & SHORTSHORT)
                *va_arg(ap, char *) = nread;
            else if (flags & SHORT)
                *va_arg(ap, short *) = nread;
            else if (flags & LONG)
                *va_arg(ap, long *) = nread;
            else if (flags & QUAD)
                *va_arg(ap, s64 *) = nread;
            else
                *va_arg(ap, int *) = nread;
            continue;
        }

        /*
         * We have a conversion that requires input.
         */
        if (inr <= 0)
            goto input_failure;

        /*
         * Consume leading white space, except for formats
         * that suppress this.
         */
        if ((flags & NOSKIP) == 0) {
            while (isspace(*inp)) {
                nread++;
                if (--inr > 0)
                    inp++;
                else
                    goto input_failure;
            }
            /*
             * Note that there is at least one character in
             * the buffer, so conversions that do not set NOSKIP
             * can no longer result in an input failure.
             */
        }

        /*
         * Do the conversion.
         */
        switch (c) {
        case CT_CHAR:
            /* scan arbitrary characters (sets NOSKIP) */
            if (width == 0)
                width = 1;
            if (flags & SUPPRESS) {
                size_t sum = 0;

                n = inr;
                if (n < width) {
                    sum += n;
                    width -= n;
                    inp += n;
                    if (sum == 0)
                        goto input_failure;
                } else {
                    sum += width;
                    inr -= width;
                    inp += width;
                }
                nread += sum;
            } else {
                memcpy(va_arg(ap, char *), inp, width);
                inr -= width;
                inp += width;
                nread += width;
                nassigned++;
            }
            nconversions++;
            break;

        case CT_CCL:
            /* scan a (nonempty) character class (sets NOSKIP) */
            if (width == 0)
                width = (size_t)~0;     /* `infinity' */
            /* take only those things in the class */
            if (flags & SUPPRESS) {
                n = 0;
                while (ccltab[(unsigned char)*inp]) {
                    n++, inr--, inp++;
                    if (--width == 0)
                        break;
                    if (inr <= 0) {
                        if (n == 0)
                            goto input_failure;
                        break;
                    }
                }
                if (n == 0)
                    goto match_failure;
            } else {
                p = va_arg(ap, char *);
                p0 = p;
                while (ccltab[(unsigned char)*inp]) {
                    inr--;
                    *p++ = *inp++;
                    if (--width == 0)
                        break;
                    if (inr <= 0) {
                        if (p == p0)
                            goto input_failure;
                        break;
                    }
                }
                n = p - p0;
                if (n == 0)
                    goto match_failure;
                *p = 0;
                nassigned++;
            }
            nread += n;
            nconversions++;
            break;

        case CT_STRING:
            /* like CCL, but zero-length string OK, & no NOSKIP */
            if (width == 0)
                width = (size_t)~0;
            if (flags & SUPPRESS) {
                n = 0;
                while (!isspace(*inp)) {
                    n++, inr--, inp++;
                    if (--width == 0)
                        break;
                    if (inr <= 0)
                        break;
                }
                nread += n;
            } else {
                p = va_arg(ap, char *);
                p0 = p;
                while (!isspace(*inp)) {
                    inr--;
                    *p++ = *inp++;
                    if (--width == 0)
                        break;
                    if (inr <= 0)
                        break;
                }
                *p = 0;
                nread += p - p0;
                nassigned++;
            }
            nconversions++;
            continue;

        case CT_INT:
            /* scan an integer as if by strtoq/strtouq */
#ifdef hardway
            if (width == 0 || width > sizeof(buf) - 1)
                width = sizeof(buf) - 1;
#else
            /* size_t is unsigned, hence this optimisation */
            if (--width > sizeof(buf) - 2)
                width = sizeof(buf) - 2;
            width++;
#endif
            flags |= SIGNOK | NDIGITS | NZDIGITS;
            for (p = buf; width; width--) {
                c = *inp;
                /*
                 * Switch on the character; `goto ok'
                 * if we accept it as a part of number.
                 */
                switch (c) {
                /*
                 * The digit 0 is always legal, but is
                 * special.  For %i conversions, if no
                 * digits (zero or nonzero) have been
                 * scanned (only signs), we will have
                 * base==0.  In that case, we should set
                 * it to 8 and enable 0x prefixing.
                 * Also, if we have not scanned zero digits
                 * before this, do not turn off prefixing
                 * (someone else will turn it off if we
                 * have scanned any nonzero digits).
                 */
                case '0':
                    if (base == 0) {
                        base = 8;
                        flags |= PFXOK;
                    }
                    if (flags & NZDIGITS)
                        flags &= ~(SIGNOK | NZDIGITS | NDIGITS);
                    else
                        flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* 1 through 7 always legal */
                case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                    base = basefix[base];
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* digits 8 and 9 ok iff decimal or hex */
                case '8': case '9':
                    base = basefix[base];
                    if (base <= 8)
                        break;  /* not legal here */
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* letters ok iff hex */
                case 'A': case 'B': case 'C':
                case 'D': case 'E': case 'F':
                case 'a': case 'b': case 'c':
                case 'd': case 'e': case 'f':
                    /* no need to fix base here */
                    if (base <= 10)
                        break;  /* not legal here */
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* sign ok only as first character */
                case '+': case '-':
                    if (flags & SIGNOK) {
                        flags &= ~SIGNOK;
                        goto ok;
                        }
                    break;

                /* x ok iff flag still set & 2nd char */
                case 'x': case 'X':
                    if (flags & PFXOK && p == buf + 1) {
                        base = 16;      /* if %i */
                        flags &= ~PFXOK;
                        goto ok;
                    }
                    break;
                }

                /*
                 * If we got here, c is not a legal character
                 * for a number.  Stop accumulating digits.
                 */
                break;
ok:
                /*
                 * c is legal: store it and look at the next.
                 */
                *p++ = c;
                if (--inr > 0)
                    inp++;
                else
                    break;          /* end of input */
            }
            /*
             * If we had only a sign, it is no good; push
             * back the sign.  If the number ends in `x',
             * it was [sign] '' 'x', so push back the x
             * and treat it as [sign] ''.
             */
            if (flags & NDIGITS) {
                if (p > buf) {
                    inp--;
                    inr++;
                }
                goto match_failure;
            }
            c = ((unsigned char *)p)[-1];
            if (c == 'x' || c == 'X') {
                --p;
                inp--;
                inr++;
            }
            if ((flags & SUPPRESS) == 0) {
                u64 res;

                *p = 0;
                res = (*ccfn)(buf, (char **)NULL, base);
                if (flags & POINTER)
                    *va_arg(ap, void **) =
                    (void *)(uptr_t)res;
                else if (flags & SHORTSHORT)
                    *va_arg(ap, char *) = res;
                else if (flags & SHORT)
                    *va_arg(ap, short *) = res;
                else if (flags & LONG)
                    *va_arg(ap, long *) = res;
                else if (flags & QUAD)
                    *va_arg(ap, s64 *) = res;
                else
                    *va_arg(ap, int *) = res;
                nassigned++;
            }
            nread += p - buf;
            nconversions++;
            break;
        }
    }

input_failure:
    return (nconversions != 0 ? nassigned : -1);
match_failure:
    return (nassigned);
}

/**
 * sscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	formatting of buffer
 * @...:	resulting arguments
 */
int sscanf(const char *buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsscanf(buf, fmt, args);
    va_end(args);
    return i;
}
// --------------------------------------------------------------
