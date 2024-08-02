/**
 * Hustler's Project
 *
 * File:  check.c
 * Date:  2024/06/07
 * Usage: system execution checker
 */

#include <org/globl.h>
#include <org/psci.h>
#include <org/smp.h>
#include <org/time.h>
#include <asm/barrier.h>
#include <asm/sysregs.h>
#include <asm/hcpu.h>
#include <asm/page.h>
#include <asm/define.h>
#include <bsp/time.h>
#include <bsp/debug.h>
#include <bsp/delay.h>
#include <bsp/percpu.h>
#include <bsp/panic.h>
#include <bsp/spinlock.h>

// --------------------------------------------------------------
void hang(void)
{
    while (1)
        wfe();
}

static void halt_cpu(void *arg)
{
    unsigned int i;

    local_irq_disable();
    dsb(sy);
    isb();

    MSGE("hypos crashed within %lu milliseconds, rebooting ...\n",
            get_msec());

    while (1)
        wfi();
}

void halt(void)
{
    int timeout = 10;

    local_irq_enable();
    smp_call_function(halt_cpu, NULL, 0);
    local_irq_disable();

    while ((num_online_cpus() > 1) && (timeout-- > 0))
        mdelay(1);

    call_psci_system_off();

    halt_cpu(NULL);
}

void reboot(unsigned int delay_ms)
{
    int timeout = 10;
    unsigned long count = 0;

    local_irq_enable();
    smp_call_function(halt_cpu, NULL, 0);
    local_irq_disable();

    mdelay(delay_ms);

    while ((num_online_cpus() > 1) && (timeout-- > 0))
        mdelay(1);

    call_psci_system_reset();
}

void crash(void) {
    MSGE("------------------ [Hypervisor Crashed] ------------------\n");

    flush();

    /* XXX: At HYPOS_EARLY_BOOT_STAGE, GIC might ain't set yet.
     *      In this case, simply halt the boot cpu directly.
     */
    if (hypos_get(hypos_status) == HYPOS_EARLY_BOOT_STAGE)
        halt_cpu(NULL);

#if IS_ENABLED(CFG_REBOOT_ON_PANIC)
    reboot(5000);
#else
    halt();
#endif
}

// --------------------------------------------------------------

extern void save_context(struct hcpu_regs *regs);

static void hack_stack(void)
{
    struct hcpu_regs regs;

    save_context(&regs);

    register_t *frame, next, addr, low, high;

    MSGI(">_@\n");

    MSGI("Call Trace on CPU [%d]\n", smp_processor_id());

    MSGI("    [<%p>]  %pS\n", __void__(regs.lr),
            __void__(regs.lr));

    low  = (register_t)((register_t *)regs.sp);
    high = (low & ~(STACK_SIZE - 1)) +
        (STACK_SIZE - sizeof(struct hcpu));

    next = regs.fp;

    for (;;) {
        if ((next < low) || (next >= high))
            break;

        frame = (register_t *)next;
        next  = frame[0];
        addr  = frame[1];

        MSGI("    [<%p>]  %pS\n", __void__(addr),
                __void__(addr));

        low = (register_t)&frame[1];
    }

    MSGI("@_<\n");
}

// --------------------------------------------------------------

static void panic_end(void)
{
    MSGI("\n");

    crash();
}

void panic(bool in_exception, const char *fmt, ...)
{
    unsigned long flags;
    static DEFINE_SPINLOCK(panic_lock);

    local_irq_disable();

    if (!in_exception)
        hack_stack();

    spin_lock_irqsave(&panic_lock, flags);

    va_list args;
    va_start(args, fmt);
    vpr_common(fmt, args);
    va_end(args);

    spin_unlock_irqrestore(&panic_lock, flags);

    panic_end();
}

void warn(const char *fmt, ...)
{
    va_list args;
    static DEFINE_SPINLOCK(warn_lock);
    unsigned long flags;

    spin_lock_irqsave(&warn_lock, flags);

    hack_stack();

    va_start(args, fmt);
    vpr_common(fmt, args);
    va_end(args);

    spin_unlock_irqrestore(&warn_lock, flags);
}

void __warn_crap(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    warn("[warns] %s %u - %s()\n"
         BLANK_ALIGN"[%s] bombed >_@",
         file, line,
         function, assertion);
}

void __bug_crap(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    panic(false, "[panic] %s %u - %s()\n"
                 BLANK_ALIGN"[%s] -> true  bombed >_@",
                 file, line,
                 function, assertion);
}

void __assert_crap(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    panic(false, "[assrt] %s %u - %s()\n"
                 BLANK_ALIGN"[%s] -> false bombed >_@",
                 file, line,
                 function, assertion);
}
// --------------------------------------------------------------
