/*
 * Hustler's Project
 *
 * File:  print.c
 * Date:  2024/05/20
 * Usage:
 */

#include <asm/barrier.h>
#include <bsp/spinlock.h>
#include <bsp/memz.h>
#include <bsp/errno.h>
#include <bsp/symtbl.h>
#include <bsp/compiler.h>
#include <bsp/panic.h>
#include <bsp/debug.h>
#include <lib/ctype.h>
#include <lib/math.h>
#include <lib/strops.h>
#include <lib/args.h>
#include <lib/bitops.h>

// --------------------------------------------------------------
#define is_digit(c)	((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
    int i=0;

    while (isdigit(**s))
        i = i * 10 + *((*s)++) - '0';
    return i;
}

#define ZEROPAD 1               /* pad with zero */
#define SIGN    2               /* unsigned/signed long */
#define PLUS    4               /* show plus */
#define SPACE   8               /* space if plus */
#define LEFT    16              /* left justified */
#define SPECIAL 32              /* 0x */
#define LARGE   64              /* use 'ABCDEF' instead of 'abcdef' */

static char *number(
    char *buf, const char *end, unsigned long long num,
    int base, int size, int precision, int type)
{
    char c,sign,tmp[66];
    const char *digits;
    static const char small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    static const char large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;

    ASSERT(base >= 2 && base <= 36);

    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
        type &= ~ZEROPAD;
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;

    if (type & SIGN) {
        if ((signed long long) num < 0) {
            sign = '-';
            num = - (signed long long) num;
            size--;
        } else if (type & PLUS) {
            sign = '+';
            size--;
        } else if (type & SPACE) {
            sign = ' ';
            size--;
        }
    }

    if (type & SPECIAL) {
        if (num == 0)
            type &= ~SPECIAL;
        else if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
        else
            type &= ~SPECIAL;
    }

    i = 0;
    if (num == 0)
        tmp[i++]='0';
    else while (num != 0)
        tmp[i++] = digits[do_div(num, base)];
    if (i > precision)
        precision = i;

    size -= precision;

    if (!(type&(ZEROPAD + LEFT))) {
        while(size-->0) {
            if (buf < end)
                *buf = ' ';
            ++buf;
        }
    }

    if (sign) {
        if (buf < end)
            *buf = sign;
        ++buf;
    }

    if (type & SPECIAL) {
        if (buf < end)
            *buf = '0';
        ++buf;
        if (base == 16) {
            if (buf < end)
                *buf = digits[33];
            ++buf;
        }
    }

    if (!(type & LEFT)) {
        while (size-- > 0) {
            if (buf < end)
                *buf = c;
            ++buf;
        }
    }

    while (i < precision--) {
        if (buf < end)
            *buf = '0';
        ++buf;
    }

    while (i-- > 0) {
        if (buf < end)
            *buf = tmp[i];
        ++buf;
    }

    while (size-- > 0) {
        if (buf < end)
            *buf = ' ';
        ++buf;
    }

    return buf;
}

static char *string(char *str, const char *end, const char *s,
                    int field_width, int precision, int flags)
{
    int i, len = (precision < 0) ? strlen(s) : strnlen(s, precision);

    if (!(flags & LEFT)) {
        while (len < field_width--) {
            if (str < end)
                *str = ' ';
            ++str;
        }
    }

    for (i = 0; i < len; ++i) {
        if (str < end)
            *str = *s;
        ++str; ++s;
    }

    while (len < field_width--) {
        if (str < end)
            *str = ' ';
        ++str;
    }

    return str;
}

/* Print a bitmap as '0-3,6-15' */
static char *print_bitmap_list(char *str, const char *end,
                               const unsigned long *bitmap,
                               unsigned int nr_bits)
{
    unsigned int cur, rbot, rtop;
    bool first = true;

    rbot = cur = find_first_bit(bitmap, nr_bits);
    while (cur < nr_bits) {
        rtop = cur;
        cur = find_next_bit(bitmap, nr_bits, cur + 1);

        if (cur < nr_bits && cur <= rtop + 1)
            continue;

        if (!first) {
            if ( str < end )
                *str = ',';
            str++;
        }
        first = false;

        str = number(str, end, rbot, 10, -1, -1, 0);
        if (rbot < rtop) {
            if ( str < end )
                *str = '-';
            str++;

            str = number(str, end, rtop, 10, -1, -1, 0);
        }

        rbot = cur;
    }

    return str;
}

/* Print a bitmap as a comma separated hex string. */
static char *print_bitmap_string(char *str, const char *end,
                                 const unsigned long *bitmap,
                                 unsigned int nr_bits)
{
    const unsigned int CHUNKSZ = 32;
    unsigned int chunksz;
    int i;
    bool first = true;

    chunksz = nr_bits & (CHUNKSZ - 1);
    if (chunksz == 0)
        chunksz = CHUNKSZ;

    for (i = ROUNDUP(nr_bits, CHUNKSZ) - CHUNKSZ;
         i >= 0; i -= CHUNKSZ) {
        unsigned int chunkmask = (1ULL << chunksz) - 1;
        unsigned int word      = i / BITS_PER_LONG;
        unsigned int offset    = i % BITS_PER_LONG;
        unsigned long val      = (bitmap[word] >> offset) & chunkmask;

        if (!first) {
            if ( str < end )
                *str = ',';
            str++;
        }

        first = false;
        str = number(str, end, val, 16,
                     DIV_ROUND_UP(chunksz, 4), -1, ZEROPAD);
        chunksz = CHUNKSZ;
    }

    return str;
}

static char *pointer(char *str, const char *end,
                     const char **fmt_ptr,
                     const void *arg, int field_width,
                     int precision,
                     int flags)
{
    const char *fmt = *fmt_ptr, *s;

    switch (fmt[1]) {
    case 'b': /* Bitmap as hex, or list */
        ++*fmt_ptr;

        if (field_width < 0)
            return str;

        if (fmt[2] == 'l') {
            ++*fmt_ptr;

            return print_bitmap_list(str, end, arg, field_width);
        }

        return print_bitmap_string(str, end, arg, field_width);

    case 'h': /* Raw buffer as hex string. */
    {
        const u8 *hex_buffer = arg;
        char sep = ' '; /* Separator character. */
        unsigned int i;

        /* Consumed 'h' from the format string. */
        ++*fmt_ptr;

        if (field_width <= 0)
            return str;
        if (field_width > 64)
            field_width = 64;

        switch (fmt[2]) {
        case 'C': /* Colons. */
            ++*fmt_ptr;
            sep = ':';
            break;

        case 'D': /* Dashes. */
            ++*fmt_ptr;
            sep = '-';
            break;

        case 'N': /* No separator. */
            ++*fmt_ptr;
            sep = 0;
            break;
        }

        for (i = 0; ; ) {
            /* Each byte: 2 chars, 0-padded, base 16, no hex prefix. */
            str = number(str, end, hex_buffer[i], 16, 2, -1, ZEROPAD);

            if (++i == field_width)
                break;

            if (sep) {
                if ( str < end )
                    *str = sep;
                ++str;
            }
        }

        return str;
    }
    case 's': /* Symbol name only */
    {
        unsigned long sym_size, sym_offset;
        char namebuf[KSYM_NAME_LEN + 1];

        ++*fmt_ptr;

        s = symbols_lookup((unsigned long)arg, &sym_size,
                           &sym_offset, namebuf);

        if (!s)
            break;

        return string(str, end, s, -1, -1, 0);
    }
    case 'S': /* Symbol name unconditionally with offset and size */
    {
        unsigned long sym_size, sym_offset;
        char namebuf[KSYM_NAME_LEN + 1];

        ++*fmt_ptr;

        s = symbols_lookup((unsigned long)arg, &sym_size,
                            &sym_offset, namebuf);

        if (!s)
            break;

        /* Print symbol name */
        str = string(str, end, s, -1, -1, 0);

        if (fmt[1] == 'S' || sym_offset != 0) {
            /* Print '+<offset>/<len>' */
            str = number(str, end, sym_offset, 16, -1, -1, SPECIAL|SIGN|PLUS);
            if ( str < end )
                *str = '/';
            ++str;
            str = number(str, end, sym_size, 16, -1, -1, SPECIAL);
        }

        if (namebuf != s) {
            str = string(str, end, " [", -1, -1, 0);
            str = string(str, end, namebuf, -1, -1, 0);
            str = string(str, end, "]", -1, -1, 0);
        }

        return str;
    }
    default:
        break;
    }

    if (field_width == -1) {
        field_width = 2 * sizeof(void *);
        flags |= ZEROPAD;
    }

    return number(str, end, (unsigned long)arg,
                  16, field_width, precision, flags);
}

int vsnpr(char *buf, size_t size, const char *fmt, va_list args)
{
    unsigned long long num;
    int base;
    char *str, *end, c;
    const char *s;

    int flags;          /* flags to number() */

    int field_width;    /* width of output field */
    int precision;      /* min. # of digits for integers; max
                           number of chars for from string */
    int qualifier;      /* 'h', 'l', or 'L' for integer fields */
                        /* 'z' support added 23/7/1999 S.H.    */
                        /* 'z' changed to 'Z' --davidm 1/25/99 */

    /* Reject out-of-range values early */
    BUG_ON(((int)size < 0) || ((unsigned int)size != size));

    str = buf;
    end = buf + size;

    if (end < buf) {
        end = ((void *) -1);
        size = end - buf;
    }

    for (; *fmt ; ++fmt) {
        if (*fmt != '%') {
            if (str < end)
                *str = *fmt;
            ++str;
            continue;
        }

        /* process flags */
        flags = 0;
repeat:
        ++fmt;          /* this also skips first '%' */
        switch (*fmt) {
        case '-': flags |= LEFT; goto repeat;
        case '+': flags |= PLUS; goto repeat;
        case ' ': flags |= SPACE; goto repeat;
        case '#': flags |= SPECIAL; goto repeat;
        case '0': flags |= ZEROPAD; goto repeat;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt))
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
            if (isdigit(*fmt))
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
            *fmt =='Z' || *fmt == 'z') {
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
                while (--field_width > 0) {
                    if (str < end)
                        *str = ' ';
                    ++str;
                }
            }
            c = (unsigned char)va_arg(args, int);
            if (str < end)
                *str = c;
            ++str;
            while (--field_width > 0) {
                if (str < end)
                    *str = ' ';
                ++str;
            }
            continue;
        case 's':
            s = va_arg(args, char *);
            if ((unsigned long)s < PAGE_SIZE)
                s = "<NULL>";

            str = string(str, end, s, field_width, precision, flags);
            continue;
        case 'p':
            /* pointer() might advance fmt (%pS for example) */
            str = pointer(str, end, &fmt, va_arg(args, const void *),
                          field_width, precision, flags);
            continue;
        case 'n':
            if (qualifier == 'l') {
                long * ip = va_arg(args, long *);
                *ip = (str - buf);
            } else if (qualifier == 'Z' || qualifier == 'z') {
                size_t * ip = va_arg(args, size_t *);
                *ip = (str - buf);
            } else {
                int * ip = va_arg(args, int *);
                *ip = (str - buf);
            }
            continue;
        case '%':
            if (str < end)
                *str = '%';
            ++str;
            continue;
        case 'o':
            base = 8;
            break;
        case 'X':
            flags |= LARGE;
            fallthrough;
        case 'x':
            base = 16;
            break;
        case 'd':
        case 'i':
            flags |= SIGN;
            fallthrough;
        case 'u':
            break;
        default:
            if (str < end)
                *str = '%';
            ++str;
            if (*fmt) {
                if (str < end)
                    *str = *fmt;
                ++str;
            } else {
                --fmt;
            }
            continue;
        }
        if (qualifier == 'L')
            num = va_arg(args, long long);
        else if (qualifier == 'l') {
            num = va_arg(args, unsigned long);
            if (flags & SIGN)
                num = (signed long) num;
        } else if (qualifier == 'Z' || qualifier == 'z') {
            num = va_arg(args, size_t);
        } else if (qualifier == 'h') {
            num = (unsigned short) va_arg(args, int);
            if (flags & SIGN)
                num = (signed short) num;
        } else {
            num = va_arg(args, unsigned int);
            if (flags & SIGN)
                num = (signed int) num;
        }

        str = number(str, end, num, base,
                     field_width, precision, flags);
    }

    if (size > 0) {
        if (str < end)
            *str = '\0';
        else
            end[-1] = '\0';
    }

    return str - buf;
}

int vscnpr(char *buf, size_t size, const char *fmt, va_list args)
{
    int i;

    i = vsnpr(buf, size, fmt, args);
    if (i >= size)
        i = size - 1;
    return (i > 0) ? i : 0;
}

int scnpr(char * buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnpr(buf, size, fmt, args);
    va_end(args);
    if (i >= size)
        i = size - 1;
    return (i > 0) ? i : 0;
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

// --------------------------------------------------------------

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

    info = (struct str_info *)alloc(sizeof(struct str_info));
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

    free(info);

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

    free(info);

    return acc;
}

static const unsigned char *
__sccl(char *tab, const unsigned char *fmt)
{
    int c, n, v;

    c = *fmt++;            /* first char hat => negated scanset */
    if (c == '^') {
        v = 1;             /* default => accept */
        c = *fmt++;        /* get new first char */
    } else {
        v = 0;             /* default => reject */
    }

    for (n = 0; n < 256; n++)
        tab[n] = v;        /* memset(tab, v, 256) */

    if (c == 0)
        return (fmt - 1);  /* format ended before closing ] */

    v = 1 - v;
    for (;;) {
        tab[c] = v;        /* take character c */
doswitch:
        n = *fmt++;        /* and examine the next */
        switch (n) {
        case 0:            /* format ended too soon */
            return (fmt - 1);

        case '-':
            n = *fmt;
            if (n == ']' || n < c) {
                c = '-';
                break;     /* resume the for(;;) */
            }
            fmt++;

            do {
                tab[++c] = v;
            } while (c < n);
            c = n;

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

// --------------------------------------------------------------
#define BUF             32      /* Maximum length of numeric string. */

#define LONG            0x01    /* l: long or double */
#define SHORT           0x04    /* h: short */
#define SUPPRESS        0x08    /* suppress assignment */
#define POINTER         0x10    /* weird %p pointer (`fake hex') */
#define NOSKIP          0x20    /* do not skip blanks */
#define QUAD            0x400
#define SHORTSHORT      0x4000  /** hh: char */

#define SIGNOK          0x40    /* +/- is (still) legal */
#define NDIGITS         0x80    /* no digits detected */
#define DPTOK           0x100   /* (float) decimal point is still legal */
#define EXPOK           0x200   /* (float) exponent (e+3, etc) still legal */
#define PFXOK           0x100   /* 0x prefix is (still) legal */
#define NZDIGITS        0x200   /* no zero digits detected */

#define CT_CHAR         0       /* %c conversion */
#define CT_CCL          1       /* %[...] conversion */
#define CT_STRING       2       /* %s conversion */
#define CT_INT          3       /* integer, i.e., strtoq or strtouq */
// --------------------------------------------------------------

typedef u64 (*ccfntype)(const char *, char **, int);

static int
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
    static short basefix[17] = { 10, 1, 2, 3, 4, 5,
                                 6, 7, 8, 9, 10, 11,
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
            flags |= PFXOK;
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
            if (flags & SUPPRESS)
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

        if (inr <= 0)
            goto input_failure;

        if ((flags & NOSKIP) == 0) {
            while (isspace(*inp)) {
                nread++;
                if (--inr > 0)
                    inp++;
                else
                    goto input_failure;
            }
        }

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

                switch (c) {
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

                case 'x': case 'X':
                    if (flags & PFXOK && p == buf + 1) {
                        base = 16;      /* if %i */
                        flags &= ~PFXOK;
                        goto ok;
                    }
                    break;
                }
                break;
ok:
                *p++ = c;
                if (--inr > 0)
                    inp++;
                else
                    break;          /* end of input */
            }

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
