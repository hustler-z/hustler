/**
 * hustler's project
 *
 * file:  time.c
 * date:  2024/07/10
 * usage:
 */

#include <org/section.h>
#include <org/time.h>
#include <asm/barrier.h>
#include <bsp/panic.h>
#include <bsp/config.h>
#include <bsp/spinlock.h>
#include <bsp/delay.h>
#include <bsp/time.h>
#include <bsp/rcu.h>
#include <lib/math.h>

#if IS_IMPLEMENTED(__TIME_IMPL)
// --------------------------------------------------------------
u64 __read_mostly boot_count;

/* For fine-grained timekeeping, we use the ARM "Generic Timer", a
 * register-mapped time source in the SoC.
 */
unsigned long __read_mostly cpu_khz;  /* CPU clock frequency in kHz. */
static unsigned int timer_irq[MAX_TIMER_PPI];

unsigned int timer_get_irq(enum timer_ppi ppi)
{
    ASSERT(ppi >= TIMER_PHYS_SECURE_PPI && ppi < MAX_TIMER_PPI);

    return timer_irq[ppi];
}

stime_t ticks_to_ns(u64 ticks)
{
    return muldiv64(ticks, SECONDS(1), 1000 * cpu_khz);
}

u64 ns_to_ticks(stime_t ns)
{
    return muldiv64(ns, 1000 * cpu_khz, SECONDS(1));
}

/* Return number of nanoseconds since boot */
stime_t get_stime(void)
{
    u64 ticks = get_cycles() - boot_count;
    return ticks_to_ns(ticks);
}

int reprogram_timer(stime_t timeout)
{
    u64 deadline;

    if ( timeout == 0 )
    {
        WRITE_SYSREG(0, CNTHP_CTL_EL2);
        return 1;
    }

    deadline = ns_to_ticks(timeout) + boot_count;
    WRITE_SYSREG64(deadline, CNTHP_CVAL_EL2);
    WRITE_SYSREG(CNTx_CTL_ENABLE, CNTHP_CTL_EL2);
    isb();

    /* No need to check for timers in the past; the Generic Timer
     * fires on a signed 63-bit comparison.
     */
    return 1;
}
// --------------------------------------------------------------

/* XXX: Delay Implementation
 */
void udelay(unsigned long usecs)
{
    stime_t deadline = get_stime() + 1000 * (stime_t)usecs;
    while (get_stime() - deadline < 0)
        ;

    dsb(sy);
    isb();
}

// --------------------------------------------------------------

/* Nonzero if YEAR is a leap year (every 4 years,
 * except every 100th isn't, and every 400th is).
 */
#define __isleap(year) \
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

/* How many days are in each month.  */
static const u16 __mon_lengths[2][12] = {
    /* Normal years.  */
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    /* Leap years.  */
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

#define SECS_PER_HOUR (60 * 60)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24)

static u64 wc_sec; /* UTC time at last 'time update'. */
static unsigned int wc_nsec;
static DEFINE_SPINLOCK(wc_lock);

struct tm gmtime(unsigned long t)
{
    struct tm tbuf;
    long days, rem;
    int y;
    const u16 *ip;

    y = 1970;
#if BITS_PER_LONG >= 64
    /* Allow the concept of time before 1970.  64-bit only;
     * for 32-bit time after 2038 seems more important than
     * time before 1970.
     */
    while (t & (1UL << 39)) {
        y -= 400;
        t += ((unsigned long)(365 * 303 + 366 * 97)) * SECS_PER_DAY;
    }
    t &= (1UL << 40) - 1;
#endif

    days = t / SECS_PER_DAY;
    rem  = t % SECS_PER_DAY;

    tbuf.tm_hour = rem / SECS_PER_HOUR;
    rem %= SECS_PER_HOUR;
    tbuf.tm_min = rem / 60;
    tbuf.tm_sec = rem % 60;
    /* January 1, 1970 was a Thursday.  */
    tbuf.tm_wday = (4 + days) % 7;
    if (tbuf.tm_wday < 0)
        tbuf.tm_wday += 7;

    while (days >= (rem = __isleap(y) ? 366 : 365)) {
        ++y;
        days -= rem;
    }

    while (days < 0) {
        --y;
        days += __isleap(y) ? 366 : 365;
    }

    tbuf.tm_year = y - 1900;
    tbuf.tm_yday = days;
    ip = (const u16 *)__mon_lengths[__isleap(y)];
    for (y = 0; days >= ip[y]; ++y)
        days -= ip[y];
    tbuf.tm_mon = y;
    tbuf.tm_mday = days + 1;
    tbuf.tm_isdst = -1;

    return tbuf;
}

void update_hypos_wallclock_time(struct hypos *h)
{
    u32 *wc_version;
    u64 sec;

    spin_lock(&wc_lock);

    wc_version = &shared_info(h, wc_version);
    *wc_version = version_update_begin(*wc_version);
    smp_wmb();

    sec = wc_sec + h->time_offset.seconds;
    shared_info(h, wc_sec)    = sec;
    shared_info(h, wc_nsec)   = wc_nsec;
    shared_info(h, wc_sec_hi) = sec >> 32;

    smp_wmb();
    *wc_version = version_update_end(*wc_version);

    spin_unlock(&wc_lock);
}

/* Set clock to <secs,usecs> after 00:00:00 UTC, 1 January, 1970. */
void do_settime(u64 secs, unsigned int nsecs,
                          u64 system_time_base)
{
    u64 x;
    u32 y;
    struct hypos *h;

    x = SECONDS(secs) + nsecs - system_time_base;
    y = do_div(x, 1000000000UL);

    spin_lock(&wc_lock);
    wc_sec  = x;
    wc_nsec = y;
    spin_unlock(&wc_lock);

    rcu_read_lock();
    for_each_hypos(h)
        update_hypos_wallclock_time(h);
    rcu_read_unlock();
}

/* Return secs after 00:00:00 localtime, 1 January, 1970. */
u64 get_localtime(struct hypos *h)
{
    return wc_sec + (wc_nsec + NOW()) / 1000000000UL
        + h->time_offset.seconds;
}

/* Return microsecs after 00:00:00 localtime, 1 January, 1970. */
u64 get_localtime_us(struct hypos *h)
{
    return (SECONDS(wc_sec + h->time_offset.seconds)
            + wc_nsec + NOW()) / 1000UL;
}

u64 get_sec(void)
{
    return wc_sec + (wc_nsec + NOW()) / 1000000000UL;
}

struct tm wallclock_time(u64 *ns)
{
    u64 seconds, nsec;

    if (!wc_sec)
        return (struct tm) { 0 };

    seconds = NOW() + SECONDS(wc_sec) + wc_nsec;
    nsec = do_div(seconds, 1000000000UL);

    if (ns)
        *ns = nsec;

    return gmtime(seconds);
}

static void __bootfunc validate_timer_frequency(void)
{
    if (!cpu_khz)
        exec_panic("Timer frequency is less than 1 KHz");
}

int __bootfunc time_preset(void)
{
    if (!cpu_khz) {
        cpu_khz = (READ_SYSREG(CNTFRQ_EL0) & CNTFRQ_MASK) / 1000;
        validate_timer_frequency();
    }

    boot_count = get_cycles();

    return 0;
}
// --------------------------------------------------------------

/* Generic Timer
 *
 * XXX: Time Unit Conversion
 *
 * s ◀-- ms ◀-- us ◀-- ns
 *    |      |      |
 *  1000     |      |
 *         1000     |
 *                1000
 */
u64 get_usec(void)
{
    return wc_sec * 1000000UL + (wc_nsec + NOW()) / 1000UL;
}

u64 get_msec(void)
{
    return wc_sec * 1000UL + (wc_nsec + NOW()) / 1000000UL;
}

u64 get_msec_bias(u64 base)
{
    return (get_stime() / 1000000UL) - base;
}

// --------------------------------------------------------------
#endif /* __TIME_IMPL */
