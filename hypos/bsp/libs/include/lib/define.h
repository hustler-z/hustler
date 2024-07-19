/**
 * Hustler's Project
 *
 * File:  define.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _LIB_DEFINE_H
#define _LIB_DEFINE_H
// ------------------------------------------------------------------------

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                  \
        const typeof(((type *)0)->member) *__mptr = (ptr);  \
        (type *)((char *)__mptr - offsetof(type, member)); })

#define ABS(x) ({        \
    typeof(x) x_ = (x);  \
    (x_ < 0) ? -x_ : x_; \
})

#define SWAP(a, b)       \
    do { typeof(a) t_ = (a); (a) = (b); (b) = t_; } while (0)

// ------------------------------------------------------------------------
#endif /* _LIB_DEFINE_H */
