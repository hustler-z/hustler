/**
 * Hustler's Project
 *
 * File:  irq.h
 * Date:  2024/07/10
 * Usage:
 */

#ifndef _BSP_IRQ_H
#define _BSP_IRQ_H
// --------------------------------------------------------------
#include <org/irq.h>
#include <bsp/spinlock.h>
#include <lib/list.h>

struct irqaction {
    void (*handler)(int irq, void *devid);
    const char *name;
    void *devid;
    bool free_on_release;
    struct irqaction *next;
};

struct irq_desc;

/*
 * Interrupt controller descriptor. This is all we need
 * to describe about the low-level hardware.
 */
struct hw_interrupt_type {
    const char *typename;
    unsigned int (*startup)(struct irq_desc *desc);
    void (*shutdown)(struct irq_desc *desc);
    void (*enable)(struct irq_desc *desc);
    void (*disable)(struct irq_desc *desc);
    void (*ack)(struct irq_desc *desc);
    void (*end)(struct irq_desc *desc);
    void (*set_affinity)(struct irq_desc *desc,
                         const cpumask_t *mask);
};

typedef const struct hw_interrupt_type hw_irq_controller;

/*
 * This is the "IRQ descriptor", which contains various
 * information about the irq, including what kind of hardware
 * handling it has, whether it is disabled etc etc.
 *
 * Note: on ARMv8 we can use normal bit manipulation functions
 * to access the status field because struct irq_desc contains
 * pointers, therefore the alignment of the struct is at least
 * 8 bytes and status is the first field.
 */
typedef struct irq_desc {
    unsigned int status;        /* IRQ status */
    hw_irq_controller *handler;
    struct irqaction *action;   /* IRQ action list */
    int irq;
    spinlock_t lock;
    struct arch_irq_desc arch;
    cpumask_var_t affinity;

    /* irq ratelimit */
    stime_t rl_quantum_start;
    unsigned int rl_cnt;
    struct list_head rl_link;
} __cacheline_aligned irq_desc_t;

/*
 * XXX: IRQ Line Status
 */
#define _IRQ_INPROGRESS         0 /* IRQ handler active - do not enter! */
#define _IRQ_DISABLED           1 /* IRQ disabled - do not enter! */
#define _IRQ_PENDING            2 /* IRQ pending - replay on enable */
#define _IRQ_REPLAY             3 /* IRQ has been replayed but not acked yet */
#define _IRQ_GUEST              4 /* IRQ is handled by guest OS(es) */
#define _IRQ_MOVE_PENDING       5 /* IRQ is migrating to another CPUs */
#define _IRQ_PER_CPU            6 /* IRQ is per CPU */
#define _IRQ_GUEST_EOI_PENDING  7 /* IRQ was disabled, pending a guest EOI */
#define _IRQF_SHARED            8 /* IRQ is shared */
#define IRQ_INPROGRESS          (1u<<_IRQ_INPROGRESS)
#define IRQ_DISABLED            (1u<<_IRQ_DISABLED)
#define IRQ_PENDING             (1u<<_IRQ_PENDING)
#define IRQ_REPLAY              (1u<<_IRQ_REPLAY)
#define IRQ_GUEST               (1u<<_IRQ_GUEST)
#define IRQ_MOVE_PENDING        (1u<<_IRQ_MOVE_PENDING)
#define IRQ_PER_CPU             (1u<<_IRQ_PER_CPU)
#define IRQ_GUEST_EOI_PENDING   (1u<<_IRQ_GUEST_EOI_PENDING)
#define IRQF_SHARED             (1u<<_IRQF_SHARED)

/* Special IRQ numbers. */
#define AUTO_ASSIGN_IRQ         (-1)
#define NEVER_ASSIGN_IRQ        (-2)
#define FREE_TO_ASSIGN_IRQ      (-3)

void irq_setup(void);
void release_irq(unsigned int irq, const void *dev_id);
int setup_irq(unsigned int irq, unsigned int irqflags,
              struct irqaction *new);
// --------------------------------------------------------------
#endif /* _BSP_IRQ_H */
