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
#include <org/vreg.h>
#include <bsp/spinlock.h>
#include <bsp/rwlock.h>
#include <bsp/irq.h>
#include <org/vcpu.h>
#include <lib/rbtree.h>
#include <lib/radix.h>

#define GIC_IRQ_GUEST_QUEUED       0
#define GIC_IRQ_GUEST_ACTIVE       1
#define GIC_IRQ_GUEST_VISIBLE      2
#define GIC_IRQ_GUEST_ENABLED      3
#define GIC_IRQ_GUEST_MIGRATING    4
#define GIC_IRQ_GUEST_PRISTINE_LPI 5
#define GIC_INVALID_LR             (u8)(~0)

// --------------------------------------------------------------

#define GUEST_GICV3_GICR0_BASE     0x03020000
#define GUEST_GICV3_GICR0_SIZE     0x01000000

#define GUEST_GICV3_GICD_BASE      0x03001000
#define GUEST_GICV3_GICD_SIZE      0x00010000

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

    hpa_t dbase; /* Distributor base address */
    hpa_t cbase; /* CPU interface base address */
    hpa_t csize; /* CPU interface size */
    hpa_t vbase; /* Virtual CPU interface base address */

    struct vgic_rdist_region {
        hpa_t base;             /* Base address */
        hpa_t size;             /* Size */
        unsigned int first_cpu;   /* First CPU handled */
    } *rdist_regions;

    int nr_regions;               /* Number of rdist regions */
    unsigned long nr_lpis;
    u64 rdist_propbase;
    struct rb_root   its_devices; /* Devices mapped to an ITS */
    spinlock_t its_devices_lock;  /* Protects the its_devices tree */
    struct list_head vits_list;   /* List of virtual ITSes */
    struct radix_tree_root pend_lpi_tree;
    rwlock_t pend_lpi_tree_lock;
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
    hpa_t              rdist_base;
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
    int  (*vcpu_init)(struct vcpu *v);
    int  (*hypos_init)(struct hypos *h);
    void (*hypos_free)(struct hypos *h);
    bool (*emulate_reg)(struct hcpu_regs *regs,
                        union hcpu_esr hsr);
    struct pending_irq *(*lpi_to_pending)(struct hypos *h,
                                          unsigned int vlpi);
    int (*lpi_get_priority)(struct hypos *h, u32 vlpi);
};

#define vgic_num_irqs(h)      ((h)->arch.vgic.nr_spis + 32)

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

    /* Never reach here */
    return 0;
}

static inline hpa_t vgic_cpu_base(const struct vgic_dist *vgic)
{
    return vgic->cbase;
}

static inline hpa_t vgic_dist_base(const struct vgic_dist *vgic)
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
void vgic_vcpu_inject_lpi(struct hypos *h, unsigned int virq);
// --------------------------------------------------------------

int vgic_v3_init(struct hypos *d, unsigned int *mmio_count);
void vgic_inject_irq(struct hypos *d, struct vcpu *v,
                     unsigned int virq,
                     bool level);
void vgic_v3_setup_hw(hpa_t dbase,
                      unsigned int nr_rdist_regions,
                      const struct rdist_region *regions,
                      unsigned int intid_bits);

// --------------------------------------------------------------

#define VREG32(reg) (reg) ... ((reg) + 3)
#define VREG64(reg) (reg) ... ((reg) + 7)

#define VRANGE32(start, end) (start) ... ((end) + 3)
#define VRANGE64(start, end) (start) ... ((end) + 7)

static inline bool vgic_reg64_check_access(struct hcpu_dabt dabt)
{
    return (dabt.sas == DABT_DOUBLE_WORD || dabt.sas == DABT_WORD);
}

extern struct list_head host_its_list;
// --------------------------------------------------------------
#endif /* _ORG_VGIC_H */
