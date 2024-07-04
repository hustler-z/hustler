/**
 * Hustler's Project
 *
 * File:  check.c
 * Date:  2024/06/07
 * Usage: system execution checker
 */

#include <asm-generic/spinlock.h>
#include <asm/barrier.h>
#include <asm/sysregs.h>
#include <asm/hcpu.h>
#include <asm/page.h>
#include <asm/exit.h>
#include <asm/define.h>
#include <common/exit.h>
#include <bsp/debug.h>
#include <bsp/percpu.h>
#include <bsp/panic.h>
#include <lib/args.h>

// --------------------------------------------------------------
void hang(void) {
    MSGE("------------------ [Hypervisor Crashed] ------------------\n");

    flush();

    for (;;)
        ;
}

void do_reboot(void)
{
    arch_cpu_reboot();
}

void do_exit(int exit_code)
{

}

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

static void panic_end(void)
{
    MSGI("\n");

    hang();
}

static DEFINE_SPINLOCK(panic_lock);

void panic(bool in_exception, const char *fmt, ...)
{


    local_irq_disable();

    if (!in_exception)
        hack_stack();

    spinlock(&panic_lock);

    va_list args;
    va_start(args, fmt);
    vpr_common(fmt, args);
    va_end(args);

    spinunlock(&panic_lock);

    panic_end();
}

void warn(const char *fmt, ...)
{
    va_list args;

    spinlock(&panic_lock);

    hack_stack();

    va_start(args, fmt);
    vpr_common(fmt, args);
    va_end(args);

    spinunlock(&panic_lock);
}

void __warn_crap(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    warn("[warns] %s %u - %s()\n"
         "        \"%s\" Bombed ^_^",
         file, line,
         function, assertion);
}

void __bug_crap(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    panic(false, "[panic] %s %u - %s()\n"
                 "        \"%s\" -> True  F*cked Up ^_^",
                 file, line,
                 function, assertion);
}

void __assert_crap(const char *assertion, const char *file,
        unsigned int line, const char *function)
{
    panic(false, "[assrt] %s %u - %s()\n"
                 "        \"%s\" -> False Bombed ^_^",
                 file, line,
                 function, assertion);
}
// --------------------------------------------------------------
