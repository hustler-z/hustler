/**
 * Hustler's Project
 *
 * File:  time.h
 * Date:  2024/07/10
 * Usage:
 */

#ifndef _BSP_TIME_H
#define _BSP_TIME_H
// --------------------------------------------------------------
#include <org/vcpu.h>
#include <bsp/type.h>

stime_t get_stime_fixed(u64 at_tick);
stime_t get_stime(void);
u64 get_localtime(struct hypos *h);
u64 get_localtime_us(struct hypos *h);
u64 get_sec(void);
u64 get_usec(void);
u64 get_msec(void);
u64 get_msec_bias(u64 base);

struct tm {
    int     tm_sec;         /* seconds */
    int     tm_min;         /* minutes */
    int     tm_hour;        /* hours */
    int     tm_mday;        /* day of the month */
    int     tm_mon;         /* month */
    int     tm_year;        /* year */
    int     tm_wday;        /* day of the week */
    int     tm_yday;        /* day in the year */
    int     tm_isdst;       /* daylight saving time */
};

struct tm gmtime(unsigned long t);
struct tm wallclock_time(u64 *ns);

#define SYSTEM_TIME_HZ  1000000000ULL
#define NOW()           ((stime_t)get_stime())
#define DAYS(_d)        SECONDS((_d) * 86400ULL)
#define SECONDS(_s)     ((stime_t)((_s)  * 1000000000ULL))
#define MILLISECS(_ms)  ((stime_t)((_ms) * 1000000ULL))
#define MICROSECS(_us)  ((stime_t)((_us) * 1000ULL))
#define STIME_MAX       ((stime_t)((u64)~0ULL >> 1))

/* Chosen so (NOW() + delta) wont overflow without an uptime of
 * 200 years.
 */
#define STIME_DELTA_MAX ((stime_t)((u64)~0ULL>>2))

/* Explicitly OR with 1 just in case version number gets out of
 * sync.
 */
#define version_update_begin(v) (((v) + 1) | 1)
#define version_update_end(v)   ((v) + 1)

// --------------------------------------------------------------
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

// --------------------------------------------------------------
#endif /* _BSP_TIME_H */
