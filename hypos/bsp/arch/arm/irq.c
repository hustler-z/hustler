/**
 * Hustler's Project
 *
 * File:  irq.c
 * Date:  2024/07/09
 * Usage:
 */

#include <org/virq.h>
#include <org/section.h>
#include <bsp/percpu.h>
#include <bsp/hypmem.h>
#include <bsp/errno.h>
#include <bsp/cpu.h>
#include <bsp/config.h>

/* --------------------------------------------------------------
 * asm/vcpu.h ◀---+   XXX: error: field '*' has incomplete type
 *    |           |
 * org/vcpu.h     :
 *    \           |
 *  org/gic.h ---▶+
 * --------------------------------------------------------------
 */

#if IS_IMPLEMENTED(__IRQ_IMPL)
// --------------------------------------------------------------

irq_cpustat_t irq_stat[NR_CPUS];

const unsigned int nr_irqs = NR_IRQS;

static unsigned int local_irqs_type[NR_LOCAL_IRQS];
static DEFINE_SPINLOCK(local_irqs_type_lock);
static irq_desc_t irq_desc[NR_IRQS];
static DEFINE_PERCPU(irq_desc_t[NR_LOCAL_IRQS], local_irq_desc);
DEFINE_PERCPU(const struct hcpu_regs *, irq_regs);

struct irq_guest {
    struct hypos *h;
    unsigned int virq;
};

static void ack_none(struct irq_desc *irq)
{
    MSGH("Unexpected IRQ trap at irq %02x\n", irq->irq);
}

static void end_none(struct irq_desc *irq)
{
    gic_hw_ops->gic_host_irq_type->end(irq);
}

static void irq_actor_none(struct irq_desc *desc)
{
}

#define irq_shutdown_none irq_actor_none
#define irq_enable_none   irq_actor_none
#define irq_disable_none  irq_actor_none

static unsigned int irq_startup_none(struct irq_desc *desc)
{
    return 0;
}

hw_irq_controller no_irq_type = {
    .typename = "none",
    .startup = irq_startup_none,
    .shutdown = irq_shutdown_none,
    .enable = irq_enable_none,
    .disable = irq_disable_none,
    .ack = ack_none,
    .end = end_none
};

struct irq_desc *__irq_to_desc(int irq)
{
    if (irq < NR_LOCAL_IRQS)
        return &this_cpu(local_irq_desc)[irq];

    return &irq_desc[irq - NR_LOCAL_IRQS];
}

int arch_init_one_irq_desc(struct irq_desc *desc)
{
    desc->arch.type = IRQ_TYPE_INVALID;
    return 0;
}

int init_one_irq_desc(struct irq_desc *desc)
{
    int err;

    if (irq_desc_initialized(desc))
        return 0;

    if (!alloc_cpumask_var(&desc->affinity))
        return -ENOMEM;

    desc->status = IRQ_DISABLED;
    desc->handler = &no_irq_type;
    spin_lock_init(&desc->lock);
    cpumask_setall(desc->affinity);
    INIT_LIST_HEAD(&desc->rl_link);

    err = arch_init_one_irq_desc(desc);
    if (err) {
        free_cpumask_var(desc->affinity);
        desc->handler = NULL;
    }

    return err;
}

static int __bootfunc init_irq_data(void)
{
    int irq;

    for (irq = NR_LOCAL_IRQS; irq < NR_IRQS; irq++) {
        struct irq_desc *desc = irq_to_desc(irq);
        int rc = init_one_irq_desc(desc);

        if (rc)
            return rc;

        desc->irq = irq;
        desc->action  = NULL;
    }

    return 0;
}

static int init_local_irq_data(unsigned int cpu)
{
    int irq;

    spin_lock(&local_irqs_type_lock);

    for (irq = 0; irq < NR_LOCAL_IRQS; irq++) {
        struct irq_desc *desc = &percpu(local_irq_desc, cpu)[irq];
        int rc = init_one_irq_desc(desc);

        if ( rc )
            return rc;

        desc->irq = irq;
        desc->action  = NULL;
        desc->arch.type = local_irqs_type[irq];
    }

    spin_unlock(&local_irqs_type_lock);

    return 0;
}

static inline struct irq_guest *irq_get_guest_info(struct irq_desc *desc)
{
    ASSERT(spin_is_locked(&desc->lock));
    ASSERT(test_bit(_IRQ_GUEST, &desc->status));
    ASSERT(desc->action != NULL);

    return desc->action->devid;
}

static inline struct hypos *irq_get_hypos(struct irq_desc *desc)
{
    return irq_get_guest_info(desc)->h;
}

// --------------------------------------------------------------

static int cpu_callback(struct notifier_block *nfb,
                        unsigned long action,
                        void *hcpu)
{
    unsigned int cpu = (unsigned long)hcpu;
    int rc = 0;

    switch (action) {
    case CPU_UP_PREPARE:
        rc = init_local_irq_data(cpu);
        if (rc)
            MSGE("Unable to allocate local IRQ for CPU%u\n", cpu);
        break;
    }

    return notifier_from_errno(rc);
}

static struct notifier_block cpu_nfb = {
    .notifier_call = cpu_callback,
};

void __bootfunc irq_setup(void)
{
    int irq;

    spin_lock(&local_irqs_type_lock);
    for (irq = 0; irq < NR_LOCAL_IRQS; irq++)
        local_irqs_type[irq] = IRQ_TYPE_INVALID;
    spin_unlock(&local_irqs_type_lock);

    BUG_ON(init_local_irq_data(smp_processor_id()) < 0);
    BUG_ON(init_irq_data() < 0);

    register_cpu_notifier(&cpu_nfb);
}

void __do_irq(struct hcpu_regs *regs, unsigned int irq, int is_fiq)
{
    struct irq_desc *desc = irq_to_desc(irq);
    struct irqaction *action;
    const struct hcpu_regs *old_regs = set_irq_regs(regs);

    ASSERT(irq >= 16);

    irq_enter();

    spin_lock(&desc->lock);
    desc->handler->ack(desc);

    if (test_bit(_IRQ_GUEST, &desc->status)) {
        struct irq_guest *info = irq_get_guest_info(desc);

        desc->handler->end(desc);

        set_bit(_IRQ_INPROGRESS, &desc->status);

        vgic_inject_irq(info->h, NULL, info->virq, true);
        goto out_no_end;
    }

    if (test_bit(_IRQ_DISABLED, &desc->status))
        goto out;

    set_bit(_IRQ_INPROGRESS, &desc->status);
    action = desc->action;

    spin_unlock_irq(&desc->lock);

    do {
        action->handler(irq, action->devid);
        action = action->next;
    } while (action);

    spin_lock_irq(&desc->lock);

    clear_bit(_IRQ_INPROGRESS, &desc->status);

out:
    desc->handler->end(desc);
out_no_end:
    spin_unlock(&desc->lock);
    irq_exit();
    set_irq_regs(old_regs);
}

void irq_set_affinity(struct irq_desc *desc, const cpumask_t *mask)
{
    if (desc != NULL)
        desc->handler->set_affinity(desc, mask);
}

int request_irq(unsigned int irq, unsigned int irqflags,
                void (*handler)(int irq, void *devid),
                const char *devname, void *devid)
{
    struct irqaction *action;
    int retval;

    if (irq >= nr_irqs)
        return -EINVAL;
    if (!handler)
        return -EINVAL;

    action = malloc(struct irqaction);
    if (!action)
        return -ENOMEM;

    action->handler = handler;
    action->name = devname;
    action->devid = devid;
    action->free_on_release = 1;

    retval = setup_irq(irq, irqflags, action);
    if (retval)
        free(action);

    return retval;
}

void release_irq(unsigned int irq, const void *devid)
{
    struct irq_desc *desc;
    unsigned long flags;
    struct irqaction *action, **action_ptr;

    desc = irq_to_desc(irq);

    spin_lock_irqsave(&desc->lock, flags);

    action_ptr = &desc->action;
    for ( ;; ) {
        action = *action_ptr;
        if (!action) {
            MSGH("Trying to free already-free IRQ %u\n", irq);
            spin_unlock_irqrestore(&desc->lock, flags);
            return;
        }

        if (action->devid == devid)
            break;

        action_ptr = &action->next;
    }

    *action_ptr = action->next;

    if (!desc->action) {
        desc->handler->shutdown(desc);
        clear_bit(_IRQ_GUEST, &desc->status);
    }

    spin_unlock_irqrestore(&desc->lock,flags);

    do {
        smp_mb();
    } while (test_bit(_IRQ_INPROGRESS, &desc->status));

    if (action->free_on_release)
        free(action);
}

static int __setup_irq(struct irq_desc *desc, unsigned int irqflags,
                       struct irqaction *new)
{
    bool shared = irqflags & IRQF_SHARED;

    ASSERT(new != NULL);

    if (desc->action != NULL && (!test_bit(_IRQF_SHARED,
                                 &desc->status) || !shared))
        return -EINVAL;
    if (shared && new->devid == NULL)
        return -EINVAL;

    if (shared)
        set_bit(_IRQF_SHARED, &desc->status);

    new->next = desc->action;
    dsb(ish);
    desc->action = new;
    dsb(ish);

    return 0;
}

int setup_irq(unsigned int irq, unsigned int irqflags,
              struct irqaction *new)
{
    int rc;
    unsigned long flags;
    struct irq_desc *desc;
    bool disabled;

    desc = irq_to_desc(irq);

    spin_lock_irqsave(&desc->lock, flags);

    if (test_bit(_IRQ_GUEST, &desc->status)) {
        struct hypos *h = irq_get_hypos(desc);

        spin_unlock_irqrestore(&desc->lock, flags);
        MSGE("IRQ %u is already in use by the hypos %u\n",
               irq, h->hid);
        return -EBUSY;
    }

    disabled = (desc->action == NULL);

    rc = __setup_irq(desc, irqflags, new);
    if (rc)
        goto err;

    if (disabled) {
        gic_route_irq_to_hypos(desc, GIC_PRI_IRQ);

        irq_set_affinity(desc, cpumask_of(smp_processor_id()));
        desc->handler->startup(desc);
    }

err:
    spin_unlock_irqrestore(&desc->lock, flags);

    return rc;
}

bool irq_type_set_by_hypos(const struct hypos *h)
{
    return is_hardware_hypos(h);
}

#if IS_IMPLEMENTED(__ROUTE_IRQ_TO_GUEST)
// --------------------------------------------------------------
bool is_assignable_irq(unsigned int irq)
{
    return (irq >= NR_LOCAL_IRQS) && (irq < gic_number_lines());
}

int route_irq_to_guest(struct hypos *h, unsigned int virq,
                       unsigned int irq, const char * devname)
{
    struct irqaction *action;
    struct irq_guest *info;
    struct irq_desc *desc;
    unsigned long flags;
    int retval = 0;

    if (virq >= vgic_num_irqs(h)) {
        MSGH("the vIRQ number %u is too high for hypos %u (max = %u)\n",
             irq, h->hypos_id, vgic_num_irqs(h));
        return -EINVAL;
    }

    if (virq < NR_LOCAL_IRQS) {
        MSGH("IRQ can only be routed to an SPI\n");
        return -EINVAL;
    }

    if (!is_assignable_irq(irq)) {
        MSGH("the IRQ%u is not routable\n", irq);
        return -EINVAL;
    }

    desc = irq_to_desc(irq);

    action = malloc(struct irqaction);
    if (!action)
        return -ENOMEM;

    info = malloc(struct irq_guest);
    if (!info) {
        free(action);
        return -ENOMEM;
    }

    info->h = h;
    info->virq = virq;

    action->devid = info;
    action->name = devname;
    action->free_on_release = 1;

    spin_lock_irqsave(&desc->lock, flags);

    if (!irq_type_set_by_hypos(d) &&
        desc->arch.type == IRQ_TYPE_INVALID) {
        MSGH("IRQ %u has not been configured\n", irq);
        retval = -EIO;
        goto out;
    }

    if (desc->action != NULL) {
        if (test_bit(_IRQ_GUEST, &desc->status)) {
            struct hypos *ad = irq_get_hypos(desc);

            if (h != ad) {
                MSGH("IRQ %u is already used by hypos %u\n",
                       irq, ad->hypos_id);
                retval = -EBUSY;
            } else if (irq_get_guest_info(desc)->virq != virq) {
                MSGH("d%u: IRQ %u is already assigned to vIRQ %u\n",
                     h->hypos_id, irq, irq_get_guest_info(desc)->virq);
                retval = -EBUSY;
            }
        } else {
            MSGH("IRQ %u is already used by Xen\n", irq);
            retval = -EBUSY;
        }
        goto out;
    }

    retval = __setup_irq(desc, 0, action);
    if (retval)
        goto out;

    retval = gic_route_irq_to_guest(d, virq, desc, GIC_PRI_IRQ);

    spin_unlock_irqrestore(&desc->lock, flags);

    if (retval) {
        release_irq(desc->irq, info);
        goto free_info;
    }

    return 0;

out:
    spin_unlock_irqrestore(&desc->lock, flags);
    free(action);
free_info:
    free(info);

    return retval;
}

int release_guest_irq(struct hypos *h, unsigned int virq)
{
    struct irq_desc *desc;
    struct irq_guest *info;
    unsigned long flags;
    int ret;

    /* Only SPIs are supported */
    if (virq < NR_LOCAL_IRQS || virq >= vgic_num_irqs(h))
        return -EINVAL;

    desc = vgic_get_hw_irq_desc(h, NULL, virq);
    if (!desc)
        return -EINVAL;

    spin_lock_irqsave(&desc->lock, flags);

    ret = -EINVAL;
    if (!test_bit(_IRQ_GUEST, &desc->status))
        goto unlock;

    info = irq_get_guest_info(desc);
    ret = -EINVAL;
    if (d != info->h)
        goto unlock;

    ret = gic_remove_irq_from_guest(h, virq, desc);
    if (ret)
        goto unlock;

    spin_unlock_irqrestore(&desc->lock, flags);

    release_irq(desc->irq, info);
    free(info);

    return 0;

unlock:
    spin_unlock_irqrestore(&desc->lock, flags);

    return ret;
}

struct pirq *alloc_pirq_struct(struct hypos *d)
{
    return NULL;
}

int pirq_guest_bind(struct vcpu *v, struct pirq *pirq,
                    int will_share)
{
    BUG();
}

void pirq_guest_unbind(struct hypos *d, struct pirq *pirq)
{
    BUG();
}

void pirq_set_affinity(struct hypos *d, int pirq,
                       const cpumask_t *mask)
{
    BUG();
}

static bool irq_validate_new_type(unsigned int curr,
                                  unsigned int new)
{
    return (curr == IRQ_TYPE_INVALID || curr == new );
}

int irq_set_spi_type(unsigned int spi, unsigned int type)
{
    unsigned long flags;
    struct irq_desc *desc = irq_to_desc(spi);
    int ret = -EBUSY;

    /* This function should not be used for other than SPIs */
    if (spi < NR_LOCAL_IRQS)
        return -EINVAL;

    spin_lock_irqsave(&desc->lock, flags);

    if (!irq_validate_new_type(desc->arch.type, type))
        goto err;

    desc->arch.type = type;

    ret = 0;

err:
    spin_unlock_irqrestore(&desc->lock, flags);
    return ret;
}

static int irq_local_set_type(unsigned int irq,
                              unsigned int type)
{
    unsigned int cpu;
    unsigned int old_type;
    unsigned long flags;
    int ret = -EBUSY;
    struct irq_desc *desc;

    ASSERT(irq < NR_LOCAL_IRQS);

    spin_lock(&local_irqs_type_lock);

    old_type = local_irqs_type[irq];

    if (!irq_validate_new_type(old_type, type))
        goto unlock;

    ret = 0;

    if (old_type == type)
        goto unlock;

    local_irqs_type[irq] = type;

    for_each_cpu(cpu, &cpu_online_map) {
        desc = &per_cpu(local_irq_desc, cpu)[irq];
        spin_lock_irqsave(&desc->lock, flags);
        desc->arch.type = type;
        spin_unlock_irqrestore(&desc->lock, flags);
    }

unlock:
    spin_unlock(&local_irqs_type_lock);
    return ret;
}

int irq_set_type(unsigned int irq, unsigned int type)
{
    int res;

    /* Setup the IRQ type */
    if ( irq < NR_LOCAL_IRQS )
        res = irq_local_set_type(irq, type);
    else
        res = irq_set_spi_type(irq, type);

    return res;
}
#else
// --------------------------------------------------------------

/* TODO Leave those things unimplemented.
 */
int irq_set_type(unsigned int irq, unsigned int type)
{
    return 0;
}

int irq_set_spi_type(unsigned int spi, unsigned int type)
{
    return 0;
}

int route_irq_to_guest(struct hypos *h, unsigned int virq,
                       unsigned int irq, const char * devname)
{
    return 0;
}

int release_guest_irq(struct hypos *h, unsigned int virq)
{
    return 0;
}

// --------------------------------------------------------------
#endif


// --------------------------------------------------------------
#endif
