/**
 * Hustler's Project
 *
 * File:  time.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _GENERIC_TIME_H
#define _GENERIC_TIME_H
// ------------------------------------------------------------------------

#include <generic/type.h>

#define time_after(a,b)		        \
    (typecheck(unsigned long, a) && \
     typecheck(unsigned long, b) && \
     ((long)((b) - (a)) < 0))
#define time_before(a,b)	time_after(b,a)

#define time_after_eq(a,b)	        \
    (typecheck(unsigned long, a) && \
     typecheck(unsigned long, b) && \
     ((long)((a) - (b)) >= 0))
#define time_before_eq(a,b)	time_after_eq(b,a)

#define time_in_range(a,b,c)  \
    (time_after_eq(a,b) &&    \
     time_before_eq(a,c))

#define time_in_range_open(a,b,c) \
    (time_after_eq(a,b) &&        \
     time_before(a,c))

#define time_after64(a,b)	\
    (typecheck(u64, a) &&	\
     typecheck(u64, b) &&   \
     ((__s64)((b) - (a)) < 0))
#define time_before64(a,b)	time_after64(b,a)

#define time_after_eq64(a,b) \
    (typecheck(u64, a) &&    \
     typecheck(u64, b) &&    \
     ((s64)((a) - (b)) >= 0))
#define time_before_eq64(a,b)	time_after_eq64(b,a)

#define time_in_range64(a, b, c) \
    (time_after_eq64(a, b) &&    \
     time_before_eq64(a, c))

// ------------------------------------------------------------------------
#endif /* _GENERIC_TIME_H */
