/**
 * Hustler's Project
 *
 * File:  ccattr.h
 * Date:  2024/05/22
 * Usage: compiler attributes defined in this header file
 */

#ifndef _COMMON_CCATTR_H
#define _COMMON_CCATTR_H
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
#define fallthrough        __attribute__((__fallthrough__))

#define __pr(a, b)         __attribute__((__format__(printf, a, b)))

#define __always_inline    inline __attribute__((__always_inline__))

#define always_inline      __always_inline

#define RELOC_HIDE(ptr, off)   ({          \
    unsigned long __ptr;                   \
    __asm__ ("" : "=r"(__ptr) : "0"(ptr)); \
    (typeof(ptr))(__ptr + (off)); })

#define likely(x)          __builtin_expect(!!(x), 1)
#define unlikely(x)        __builtin_expect(!!(x), 0)

/* built-in function implements an atomic store operation.
 * It writes val into *ptr.
 */
#define __gnu_atomic_store(p, v) \
    __atomic_store_n((p), (v), __ATOMIC_RELAXED)

/* built-in function implements an atomic load operation.
 * It returns the contents of *ptr.
 */
#define __gnu_atomic_load(p) \
    __atomic_load_n((p), __ATOMIC_RELAXED)

/* built-in function implements an atomic compare and exchange
 * operation. This compares the contents of *ptr with the contents
 * of *expected. If equal, the operation is a read-modify-write
 * operation that writes desired into *ptr. If they are not equal,
 * the operation is a read and the current contents of *ptr are
 * written into *expected. weak is true for weak compare_exchange,
 * which may fail spuriously, and false for the strong variation,
 * which never fails spuriously. Many targets only offer the strong
 * variation and ignore the parameter. When in doubt, use the
 * strong variation.
 */
#define __gnu_compare_exchange(p, oval, nval)              \
    __atomic_compare_exchange_n((p), (oval), (nval), true, \
            __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)            \
// --------------------------------------------------------------
#endif /* _COMMON_CCATTR_H */
