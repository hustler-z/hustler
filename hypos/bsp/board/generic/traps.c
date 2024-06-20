/**
 * Hustler's Project
 *
 * File:  traps.c
 * Date:  2024/05/22
 * Usage:
 */

#include <asm/page.h>
#include <asm/hcpu.h>
#include <asm/barrier.h>
#include <generic/ccattr.h>
#include <generic/exception.h>
#include <bsp/debug.h>
#include <bsp/check.h>

// --------------------------------------------------------------
char *bad_tags[] = {
    "Synchronous Abort",
    "IRQ",
    "FIQ",
    "Error"
};

enum bad_code {
    SYNC_CODE = 0,
    IRQ_CODE,
    FIQ_CODE,
    ERROR_CODE,
};
// --------------------------------------------------------------
static void dump_far(unsigned long esr)
{
    unsigned long el, far;

    switch ((esr >> 26) & 0b111111) {
    case 0x20:
    case 0x21:
    case 0x24:
    case 0x25:
    case 0x22:
    case 0x34:
    case 0x35:
        break;
    default:
        return;
    }

    asm("mrs %0, CurrentEL": "=r" (el));

    switch (el >> 2) {
    case 1:
        asm("mrs %0, FAR_EL1": "=r" (far));
        break;
    case 2:
        asm("mrs %0, FAR_EL2": "=r" (far));
        break;
    default:
        return;
    }

    MSGI("Fault address at 0x%016lx\n", far);
}

#define __void__(x)     ((void *)(unsigned long)(x))

static void dump_stack(const struct hcpu_regs *regs)
{
    register_t *frame, next, addr, low, high;

    MSGI("Call trace:\n");

    MSGI("   [<%p>] %pS (PC)\n", __void__(regs->pc),
            __void__(regs->pc));
    MSGI("   [<%p>] %pS (LR)\n", __void__(regs->lr),
            __void__(regs->lr));

    /* Bounds for range of valid frame pointer. */
    low  = (register_t)((register_t *)regs->sp);
    high = (low & ~(STACK_SIZE - 1)) +
        (STACK_SIZE - sizeof(struct hcpu));

    /* The initial frame pointer. */
    next = regs->fp;

    for (;;) {
        if ((next < low) || (next >= high))
            break;

        frame = (register_t *)next;
        next  = frame[0];
        addr  = frame[1];

        MSGI("   [<%p>] %pS\n", __void__(addr),
                __void__(addr));

        low = (register_t)&frame[1];
    }

    MSGI("\n");
}

void dump_regs(const struct hcpu_regs *regs)
{
    MSGI("elr: %016lx lr : %016lx\n", regs->pc,  regs->lr);
    MSGI("x0 : %016lx x1 : %016lx\n", regs->x0,  regs->x1);
    MSGI("x2 : %016lx x3 : %016lx\n", regs->x2,  regs->x3);
    MSGI("x4 : %016lx x5 : %016lx\n", regs->x4,  regs->x5);
    MSGI("x6 : %016lx x7 : %016lx\n", regs->x6,  regs->x7);
    MSGI("x8 : %016lx x9 : %016lx\n", regs->x8,  regs->x9);
    MSGI("x10: %016lx x11: %016lx\n", regs->x10, regs->x11);
    MSGI("x12: %016lx x13: %016lx\n", regs->x12, regs->x13);
    MSGI("x14: %016lx x15: %016lx\n", regs->x14, regs->x15);
    MSGI("x16: %016lx x17: %016lx\n", regs->x16, regs->x17);
    MSGI("x18: %016lx x19: %016lx\n", regs->x18, regs->x19);
    MSGI("x20: %016lx x21: %016lx\n", regs->x20, regs->x21);
    MSGI("x22: %016lx x23: %016lx\n", regs->x22, regs->x23);
    MSGI("x24: %016lx x25: %016lx\n", regs->x24, regs->x25);
    MSGI("x26: %016lx x27: %016lx\n", regs->x26, regs->x27);
    MSGI("x28: %016lx fp : %016lx\n", regs->x28, regs->fp);
    MSGI("\n");
}

static void dump_execution_status(struct hcpu_regs *regs,
        char *tag)
{
    MSGH("------------- [Hypervisor Crashed] -------------\n");
    MSGI("%s - ", tag);
    dump_far(regs->esr);
    dump_regs(regs);
    dump_stack(regs);
}

void do_bad_sync(struct hcpu_regs *regs)
{
    local_irq_enable();
    dump_execution_status(regs, bad_tags[SYNC_CODE]);
    panic("<BUG> Bad %s busted.", bad_tags[SYNC_CODE]);
}

/*
 * do_bad_irq handles the impossible case in the Irq vector.
 */
void do_bad_irq(struct hcpu_regs *regs)
{
    local_irq_enable();
    dump_execution_status(regs, bad_tags[IRQ_CODE]);
    panic("<BUG> Bad %s busted.", bad_tags[IRQ_CODE]);
}

/*
 * do_bad_fiq handles the impossible case in the Fiq vector.
 */
void do_bad_fiq(struct hcpu_regs *regs)
{
    local_irq_enable();
    dump_execution_status(regs, bad_tags[FIQ_CODE]);
    panic("<BUG> Bad %s busted.", bad_tags[FIQ_CODE]);
}

/*
 * do_bad_error handles the impossible case in the Error vector.
 */
void do_bad_error(struct hcpu_regs *regs)
{
    local_irq_enable();
    dump_execution_status(regs, bad_tags[ERROR_CODE]);
    panic("<BUG> Bad %s busted.", bad_tags[ERROR_CODE]);
}

/*
 * do_sync handles the Synchronous Abort exception.
 */
void do_sync(struct hcpu_regs *regs)
{

}

/*
 * do_irq handles the Irq exception.
 */
void do_irq(struct hcpu_regs *regs)
{

}

/*
 * do_fiq handles the Fiq exception.
 */
void do_fiq(struct hcpu_regs *regs)
{

}

/*
 * do_error handles the Error exception.
 * Errors are more likely to be processor specific,
 * it is defined with weak attribute and can be redefined
 * in processor specific code.
 */
void __weak do_error(struct hcpu_regs *regs)
{

}
// --------------------------------------------------------------
