/**
 * Hustler's Project
 *
 * File:  vtimer.c
 * Date:  2024/05/22
 * Usage:
 */

#include <org/vtimer.h>

#if IS_IMPLEMENTED(__VTIMER_IMPL)
// --------------------------------------------------------------

#define ACCESS_ALLOWED(regs, user_gate) \
    (!regs_mode_is_user(regs) || \
     (READ_SYSREG(CNTKCTL_EL1) & CNTKCTL_EL1_##user_gate))

static void phys_timer_expired(void *data)
{
    struct vtimer *t = data;
    t->ctl |= CNTx_CTL_PENDING;
    if (!(t->ctl & CNTx_CTL_MASK)) {
        vgic_inject_irq(t->v->hypos, t->v, t->irq, true);
    }
}

static void virt_timer_expired(void *data)
{
    struct vtimer *t = data;
    t->ctl |= CNTx_CTL_MASK;
    vgic_inject_irq(t->v->hypos, t->v, t->irq, true);
}

int hypos_vtimer_init(struct hypos *h,
                      struct arch_hypos_config *config)
{
    h->arch.virt_timer_base.offset = get_cycles();
    h->arch.virt_timer_base.nanoseconds =
        ticks_to_ns(h->arch.virt_timer_base.offset - boot_count);
    h->time_offset.seconds = h->arch.virt_timer_base.nanoseconds;
    do_div(h->time_offset.seconds, 1000000000);

    config->clock_frequency = timer_dt_clock_frequency;

    if (is_hardware_hypos(d)) {
        if (acpi_disabled &&
            !vgic_reserve_virq(d,
                    timer_get_irq(TIMER_PHYS_SECURE_PPI)))
            BUG();

        if (!vgic_reserve_virq(d,
                    timer_get_irq(TIMER_PHYS_NONSECURE_PPI)))
            BUG();

        if (!vgic_reserve_virq(d,
                    timer_get_irq(TIMER_VIRT_PPI)))
            BUG();
    } else {
        if (!vgic_reserve_virq(d,
                    GUEST_TIMER_PHYS_S_PPI))
            BUG();

        if (!vgic_reserve_virq(d,
                    GUEST_TIMER_PHYS_NS_PPI))
            BUG();

        if (!vgic_reserve_virq(d,
                    GUEST_TIMER_VIRT_PPI))
            BUG();
    }

    return 0;
}

int vcpu_vtimer_init(struct vcpu *v)
{
    struct vtimer *t = &v->arch.phys_timer;
    bool h0 = is_hardware_hypos(v->hypos);

    init_timer(&t->timer, phys_timer_expired, t, v->processor);
    t->ctl = 0;
    t->irq = h0
        ? timer_get_irq(TIMER_PHYS_NONSECURE_PPI)
        : GUEST_TIMER_PHYS_NS_PPI;
    t->v = v;

    t = &v->arch.virt_timer;
    init_timer(&t->timer, virt_timer_expired, t, v->processor);
    t->ctl = 0;
    t->irq = h0
        ? timer_get_irq(TIMER_VIRT_PPI)
        : GUEST_TIMER_VIRT_PPI;
    t->v = v;

    v->arch.vtimer_initialized = 1;

    return 0;
}

void vcpu_timer_destroy(struct vcpu *v)
{
    if (!v->arch.vtimer_initialized)
        return;

    kill_timer(&v->arch.virt_timer.timer);
    kill_timer(&v->arch.phys_timer.timer);
}

void virt_timer_save(struct vcpu *v)
{
    ASSERT(!is_idle_vcpu(v));

    v->arch.virt_timer.ctl = READ_SYSREG(CNTV_CTL_EL0);
    WRITE_SYSREG(v->arch.virt_timer.ctl & ~CNTx_CTL_ENABLE,
                 CNTV_CTL_EL0);
    v->arch.virt_timer.cval = READ_SYSREG64(CNTV_CVAL_EL0);

    if ((v->arch.virt_timer.ctl & CNTx_CTL_ENABLE) &&
        !(v->arch.virt_timer.ctl & CNTx_CTL_MASK)) {
        set_timer(&v->arch.virt_timer.timer,
                  v->hypos->arch.virt_timer_base.nanoseconds +
                  ticks_to_ns(v->arch.virt_timer.cval));
    }
}

void virt_timer_restore(struct vcpu *v)
{
    ASSERT(!is_idle_vcpu(v));

    stop_timer(&v->arch.virt_timer.timer);
    migrate_timer(&v->arch.virt_timer.timer, v->processor);
    migrate_timer(&v->arch.phys_timer.timer, v->processor);

    WRITE_SYSREG64(v->hypos->arch.virt_timer_base.offset,
                   CNTVOFF_EL2);
    WRITE_SYSREG64(v->arch.virt_timer.cval, CNTV_CVAL_EL0);
    WRITE_SYSREG(v->arch.virt_timer.ctl, CNTV_CTL_EL0);
}

static bool vtimer_cntp_ctl(struct hcpu_regs *regs, register_t *r,
                            bool read)
{
    struct vcpu *v = current;
    s_time_t expires;

    if (!ACCESS_ALLOWED(regs, EL0PTEN))
        return false;

    if (read) {
        *r = v->arch.phys_timer.ctl;
    } else {
        u32 ctl = *r & ~CNTx_CTL_PENDING;
        if (ctl & CNTx_CTL_ENABLE)
            ctl |= v->arch.phys_timer.ctl & CNTx_CTL_PENDING;
        v->arch.phys_timer.ctl = ctl;

        if (v->arch.phys_timer.ctl & CNTx_CTL_ENABLE) {
            expires = v->arch.phys_timer.cval > boot_count
                      ? ticks_to_ns(v->arch.phys_timer.cval
                        - boot_count) : 0;
            set_timer(&v->arch.phys_timer.timer, expires);
        } else
            stop_timer(&v->arch.phys_timer.timer);
    }
    return true;
}

static bool vtimer_cntp_tval(struct hcpu_regs *regs, register_t *r,
                             bool read)
{
    struct vcpu *v = current;
    u64 cntpct;
    s_time_t expires;

    if (!ACCESS_ALLOWED(regs, EL0PTEN))
        return false;

    cntpct = get_cycles();

    if (read) {
        *r = (u32)((v->arch.phys_timer.cval - cntpct)
                & 0xFFFFFFFFULL);
    } else {
        v->arch.phys_timer.cval = cntpct + (u64)(s32)*r;
        if (v->arch.phys_timer.ctl & CNTx_CTL_ENABLE) {
            v->arch.phys_timer.ctl &= ~CNTx_CTL_PENDING;

            expires = v->arch.phys_timer.cval > boot_count
                      ? ticks_to_ns(v->arch.phys_timer.cval
                        - boot_count) : 0;
            set_timer(&v->arch.phys_timer.timer, expires);
        }
    }
    return true;
}

static bool vtimer_cntp_cval(struct hcpu_regs *regs, u64 *r,
                             bool read)
{
    struct vcpu *v = current;
    s_time_t expires;

    if (!ACCESS_ALLOWED(regs, EL0PTEN))
        return false;

    if (read) {
        *r = v->arch.phys_timer.cval;
    } else {
        v->arch.phys_timer.cval = *r;
        if (v->arch.phys_timer.ctl & CNTx_CTL_ENABLE) {
            v->arch.phys_timer.ctl &= ~CNTx_CTL_PENDING;

            expires = v->arch.phys_timer.cval > boot_count
                      ? ticks_to_ns(v->arch.phys_timer.cval
                        - boot_count) : 0;
            set_timer(&v->arch.phys_timer.timer, expires);
        }
    }
    return true;
}

static bool vtimer_emulate_cp32(struct hcpu_regs *regs,
                                union hcpu_esr hsr)
{
    struct hsr_cp32 cp32 = hsr.cp32;

    switch (hsr.bits & ESR_CP32_REGS_MASK) {
    case ESR_CPREG32(CNTP_CTL):
        return vreg_emulate_cp32(regs, hsr, vtimer_cntp_ctl);
    case ESR_CPREG32(CNTP_TVAL):
        return vreg_emulate_cp32(regs, hsr, vtimer_cntp_tval);
    default:
        return false;
    }
}

static bool vtimer_emulate_cp64(struct hcpu_regs *regs,
                                union hcpu_esr hsr)
{
    struct hsr_cp64 cp64 = hsr.cp64;

    switch (hsr.bits & ESR_CP64_REGS_MASK) {
    case ESR_CPREG64(CNTP_CVAL):
        return vreg_emulate_cp64(regs, hsr, vtimer_cntp_cval);
    default:
        return false;
    }
}

static bool vtimer_emulate_sysreg(struct hcpu_regs *regs,
                                  union hcpu_esr hsr)
{
    struct hsr_sysreg sysreg = hsr.sysreg;

    switch (hsr.bits & ESR_SYSREG_REGS_MASK) {
    case ESR_SYSREG_CNTP_CTL_EL0:
        return vreg_emulate_sysreg(regs, hsr, vtimer_cntp_ctl);
    case ESR_SYSREG_CNTP_TVAL_EL0:
        return vreg_emulate_sysreg(regs, hsr, vtimer_cntp_tval);
    case ESR_SYSREG_CNTP_CVAL_EL0:
        return vreg_emulate_sysreg(regs, hsr, vtimer_cntp_cval);

    default:
        return false;
    }

}

bool vtimer_emulate(struct hcpu_regs *regs, union hcpu_esr hsr)
{

    switch (hsr.ec) {
    case ESR_EC_CP15_32:
        return vtimer_emulate_cp32(regs, hsr);
    case ESR_EC_CP15_64:
        return vtimer_emulate_cp64(regs, hsr);
    case ESR_EC_SYSREG:
        return vtimer_emulate_sysreg(regs, hsr);
    default:
        return false;
    }
}

static void vtimer_update_irq(struct vcpu *v,
                              struct vtimer *vtimer,
                              register_t vtimer_ctl)
{
    bool level;

    vtimer_ctl &= (CNTx_CTL_ENABLE | CNTx_CTL_PENDING | CNTx_CTL_MASK);
    level = (vtimer_ctl == (CNTx_CTL_ENABLE | CNTx_CTL_PENDING));
    vgic_inject_irq(v->hypos, v, vtimer->irq, level);
}

void vtimer_update_irqs(struct vcpu *v)
{
    vtimer_update_irq(v, &v->arch.virt_timer,
                      READ_SYSREG(CNTV_CTL_EL0) & ~CNTx_CTL_MASK);

    vtimer_update_irq(v, &v->arch.phys_timer,
                      v->arch.phys_timer.ctl);
}

// --------------------------------------------------------------
#endif
