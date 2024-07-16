/**
 * Hustler's Project
 *
 * File:  vgic.h
 * Date:  2024/07/15
 * Usage:
 */

#ifndef _ORG_VGIC_H
#define _ORG_VGIC_H
// --------------------------------------------------------------

#include <org/esr.h>
#include <bsp/spinlock.h>
#include <bsp/irq.h>
#include <org/vcpu.h>
#include <lib/rbtree.h>

#define GIC_IRQ_GUEST_QUEUED       0
#define GIC_IRQ_GUEST_ACTIVE       1
#define GIC_IRQ_GUEST_VISIBLE      2
#define GIC_IRQ_GUEST_ENABLED      3
#define GIC_IRQ_GUEST_MIGRATING    4
#define GIC_IRQ_GUEST_PRISTINE_LPI 5
#define GIC_INVALID_LR             (u8)(~0)

// --------------------------------------------------------------

struct pending_irq {
    unsigned long   status;
    struct irq_desc *desc;
    unsigned int    irq;

    u8 lr;
    u8 priority;
    u8 lpi_priority;
    u8 lpi_vcpu_id;

    struct list_head inflight;
    struct list_head lr_queue;
};

#define NR_INTERRUPT_PER_RANK      32
#define INTERRUPT_RANK_MASK        (NR_INTERRUPT_PER_RANK - 1)

/* Represents state corresponding to a block of 32 interrupts */
struct vgic_irq_rank {
    spinlock_t lock;

    u8  index;

    u32 ienable;
    u32 icfg[2];

    union {
        u8  priority[32];
        u32 ipriorityr[8];
    };

    u8 vcpu[32];
};

struct vgic_ops;

struct vgic_dist {
    enum gic_version version;
    const struct vgic_ops *handler;
    spinlock_t lock;
    u32 ctlr;
    int nr_spis;

    unsigned long        *allocated_irqs;
    struct vgic_irq_rank *shared_irqs;
    struct pending_irq   *pending_irqs;

    paddr_t dbase; /* Distributor base address */
    paddr_t cbase; /* CPU interface base address */
    paddr_t csize; /* CPU interface size */
    paddr_t vbase; /* Virtual CPU interface base address */

    struct vgic_rdist_region {
        paddr_t base;             /* Base address */
        paddr_t size;             /* Size */
        unsigned int first_cpu;   /* First CPU handled */
    } *rdist_regions;

    int nr_regions;               /* Number of rdist regions */
    unsigned long nr_lpis;
    u64 rdist_propbase;
    struct rb_root   its_devices; /* Devices mapped to an ITS */
    spinlock_t its_devices_lock;  /* Protects the its_devices tree */
    struct list_head vits_list;   /* List of virtual ITSes */
    unsigned int     intid_bits;
    bool rdists_enabled;          /* Is any redistributor enabled? */
    bool has_its;
};

// --------------------------------------------------------------

#define VGIC_V3_RDIST_LAST      (1 << 0)
#define VGIC_V3_LPIS_ENABLED    (1 << 1)

struct vgic_cpu {
    struct pending_irq   pending_irqs[32];
    struct vgic_irq_rank *private_irqs;
    struct list_head     inflight_irqs;
    struct list_head     lr_pending;
    spinlock_t           lock;
    paddr_t              rdist_base;
    u64                  rdist_pendbase;
    u8                   flags;
};

struct sgi_target {
    u8  aff1;
    u16 list;
};

static inline void sgi_target_init(struct sgi_target *sgi_target)
{
    sgi_target->aff1 = 0;
    sgi_target->list = 0;
}

struct vgic_ops {
    int  (*vcpu_setup)(struct vcpu *v);
    bool (*emulate_reg)(struct hcpu_regs *regs,
                        union hcpu_esr hsr);
};

// --------------------------------------------------------------

#define HYPOS_NR_RANKS(h) (((h)->arch.vgic.nr_spis + 31) / 32)

#define vgic_lock(v)   spin_lock_irq(&(v)->hypos->arch.vgic.lock)
#define vgic_unlock(v) spin_unlock_irq(&(v)->hypos->arch.vgic.lock)

#define vgic_lock_rank(v, r, flags)   \
    spin_lock_irqsave(&(r)->lock, flags)
#define vgic_unlock_rank(v, r, flags) \
    spin_unlock_irqrestore(&(r)->lock, flags)

static inline unsigned int REG_RANK_NR(unsigned int b, unsigned int n)
{
    switch (b) {
    case 64:
    case 32: return n >> 5;
    case 16: return n >> 4;
    case 8:  return n >> 3;
    case 4:  return n >> 2;
    case 2:  return n >> 1;
    case 1:  return n;
    default: BUG();
    }
}

static inline paddr_t vgic_cpu_base(const struct vgic_dist *vgic)
{
    return vgic->cbase;
}

static inline paddr_t vgic_dist_base(const struct vgic_dist *vgic)
{
    return vgic->dbase;
}

#define REG_RANK_INDEX(b, n, s) ((((n) >> (s)) & ((b) - 1)) % 32)

// --------------------------------------------------------------

extern struct vcpu *vgic_get_target_vcpu(struct vcpu *v,
                                         unsigned int virq);
extern void vgic_remove_irq_from_queues(struct vcpu *v,
                                        struct pending_irq *p);
extern void gic_remove_from_lr_pending(struct vcpu *v,
                                       struct pending_irq *p);
extern void vgic_init_pending_irq(struct pending_irq *p,
                                  unsigned int virq);
extern struct pending_irq *irq_to_pending(struct vcpu *v,
                                          unsigned int irq);
extern struct pending_irq *spi_to_pending(struct hypos *d,
                                          unsigned int irq);
extern struct vgic_irq_rank *vgic_rank_offset(struct vcpu *v,
                                              unsigned int b,
                                              unsigned int n,
                                              unsigned int s);
extern struct vgic_irq_rank *vgic_rank_irq(struct vcpu *v,
                                           unsigned int irq);
extern void vgic_disable_irqs(struct vcpu *v, u32 r, unsigned int n);
extern void vgic_enable_irqs(struct vcpu *v, u32 r, unsigned int n);
extern void vgic_set_irqs_pending(struct vcpu *v, u32 r,
                                  unsigned int rank);
extern void register_vgic_ops(struct hypos *d,
                              const struct vgic_ops *ops);


extern bool vgic_to_sgi(struct vcpu *v, register_t sgir,
                        enum gic_sgi_mode irqmode, int virq,
                        const struct sgi_target *target);
extern bool vgic_migrate_irq(struct vcpu *old, struct vcpu *new,
                             unsigned int irq);
extern void vgic_check_inflight_irqs_pending(struct hypos *d,
                                             struct vcpu *v,
                                             unsigned int rank,
                                             u32 r);

// --------------------------------------------------------------

int vgic_v3_init(struct hypos *d, unsigned int *mmio_count);
void vgic_inject_irq(struct hypos *d, struct vcpu *v,
                     unsigned int virq,
                     bool level);
void vgic_v3_setup_hw(paddr_t dbase,
                      unsigned int nr_rdist_regions,
                      const struct rdist_region *regions,
                      unsigned int intid_bits);

// --------------------------------------------------------------

#define VREG32(reg) (reg) ... ((reg) + 3)
#define VREG64(reg) (reg) ... ((reg) + 7)

#define VRANGE32(start, end) (start) ... ((end) + 3)
#define VRANGE64(start, end) (start) ... ((end) + 7)

// --------------------------------------------------------------
#endif /* _ORG_VGIC_H */
