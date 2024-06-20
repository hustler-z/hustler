/**
 * Hustler's Project
 *
 * File:  ccattr.h
 * Date:  2024/05/22
 * Usage: compiler attributes defined in this header file
 */

#ifndef _GENERIC_CCATTR_H
#define _GENERIC_CCATTR_H
// --------------------------------------------------------------

#define __aligned(x)       __attribute__((__aligned__(x)))
#define __packed           __attribute__((__packed__))
#define __maybe_unused     __attribute__((__unused__))
#define __always_unused    __attribute__((__unused__))
#define __section(section) __attribute__((__section__(section)))
#define __notrace          __attribute__((__no_instrument_function__))
#define __weak             __attribute__((__weak__))
#define __iomem
#define __force

#define __pr(a, b)         __attribute__((__format__(printf, a, b)))

#define __always_inline    inline __attribute__((__always_inline__))

#define always_inline      __always_inline

#define barrier()          __asm__ __attribute__("": : :"memory")

#define RELOC_HIDE(ptr, off)   ({          \
    unsigned long __ptr;                   \
    __asm__ ("" : "=r"(__ptr) : "0"(ptr)); \
    (typeof(ptr))(__ptr + (off)); })

#define likely(x)          __builtin_expect(!!(x), 1)
#define unlikely(x)        __builtin_expect(!!(x), 0)
// --------------------------------------------------------------
#endif /* _GENERIC_CCATTR_H */
