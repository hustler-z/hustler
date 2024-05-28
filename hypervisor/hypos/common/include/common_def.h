/**
 * Hustler's Project
 *
 * File:  common_def.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _COMMON_DEF_H
#define _COMMON_DEF_H
// ------------------------------------------------------------------------


#define offsetof(a, b) __builtin_offsetof(a, b)

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) ({                  \
        const typeof(((type *)0)->member) *__mptr = (ptr);  \
        (type *)((char *)__mptr - offsetof(type, member)); })

#define max(x, y) ({\
    typeof(x) _max1 = (x);          \
    typeof(y) _max2 = (y);          \
    (void) (&_max1 == &_max2);      \
    _max1 > _max2 ? _max1 : _max2; })

#define min(x, y) ({                \
    typeof(x) _min1 = (x);          \
    typeof(y) _min2 = (y);          \
    (void) (&_min1 == &_min2);      \
    _min1 < _min2 ? _min1 : _min2; })

#define roundup(x, y) ({              \
    const typeof(y) __y = y;          \
    (((x) + (__y - 1)) / __y) * __y; })

// ------------------------------------------------------------------------
#endif /* _COMMON_DEF_H */
