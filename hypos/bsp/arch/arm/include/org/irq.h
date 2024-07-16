/**
 * Hustler's Project
 *
 * File:  irq.h
 * Date:  2024/06/19
 * Usage:
 */

#ifndef _ORG_IRQ_H
#define _ORG_IRQ_H
// --------------------------------------------------------------
#include <org/smp.h>
#include <asm/hcpu.h>
#include <bsp/compiler.h>
#include <bsp/percpu.h>
#include <bsp/type.h>

#define IRQ_TYPE_NONE           0x00
#define IRQ_TYPE_EDGE_RISING    0x01
#define IRQ_TYPE_EDGE_FALLING   0x02
#define IRQ_TYPE_EDGE_BOTH      0x03
#define IRQ_TYPE_LEVEL_HIGH     0x04
#define IRQ_TYPE_LEVEL_LOW      0x08
#define IRQ_TYPE_LEVEL_MASK     0x0C
#define IRQ_TYPE_SENSE_MASK     0x0F
#define IRQ_TYPE_INVALID        0x10

struct arch_irq_desc {
    unsigned int type;
};

typedef struct {
    unsigned long __softirq_pending;
    unsigned int  __local_irq_count;
} __cacheline_aligned irq_cpustat_t;

extern irq_cpustat_t irq_stat[];
#define __IRQ_STAT(cpu, member) (irq_stat[cpu].member)

#define softirq_pending(cpu) __IRQ_STAT((cpu), __softirq_pending)
#define local_irq_count(cpu) __IRQ_STAT((cpu), __local_irq_count)

#define in_irq()        (local_irq_count(smp_processor_id()) != 0)

#define irq_enter()     (local_irq_count(smp_processor_id())++)
#define irq_exit()      (local_irq_count(smp_processor_id())--)

#define NR_LOCAL_IRQS	32
/*
 * This only covers the interrupts that Xen cares about, so SGIs,
 * PPIs and SPIs. LPIs are too numerous, also only propagated to
 * guests, so they are not included in this number.
 */
#define NR_IRQS		    1024

#define LPI_OFFSET      8192

/* LPIs are always numbered starting at 8192, so 0 is a good
 * invalid case.
 */
#define INVALID_LPI     0

/* This is a spurious interrupt ID which never makes it into the
 * GIC code.
 */
#define INVALID_IRQ     1023

static inline bool is_lpi(unsigned int irq)
{
    return irq >= LPI_OFFSET;
}

extern const unsigned int nr_irqs;
#define nr_static_irqs    NR_IRQS

struct irq_desc;
struct irqaction;

struct irq_desc *__irq_to_desc(int irq);

#define irq_to_desc(irq)    __irq_to_desc(irq)
#define irq_desc_initialized(desc) \
    ((desc)->handler != NULL)

DECLARE_PERCPU(const struct hcpu_regs *, irq_regs);

static inline const struct hcpu_regs *get_irq_regs(void)
{
    return this_cpu(irq_regs);
}

static inline const struct hcpu_regs *set_irq_regs(
    const struct hcpu_regs *new_regs)
{
    const struct hcpu_regs *old_regs,
                          **pp_regs = &this_cpu(irq_regs);

    old_regs = *pp_regs;
    *pp_regs = new_regs;

    return old_regs;
}

struct irq_desc *__irq_to_desc(int irq);
void __do_irq(struct hcpu_regs *regs, unsigned int irq, int is_fiq);
int irq_set_spi_type(unsigned int spi, unsigned int type);
int irq_set_type(unsigned int irq, unsigned int type);
void irq_set_affinity(struct irq_desc *desc, const cpumask_t *mask);
// --------------------------------------------------------------
#endif /* _ORG_IRQ_H */
