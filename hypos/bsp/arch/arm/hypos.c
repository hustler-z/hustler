/**
 * Hustler's Project
 *
 * File:  hypos.c
 * Date:  2024/07/10
 * Usage:
 */

#include <org/section.h>
#include <org/vcpu.h>
#include <bsp/sched.h>
#include <bsp/cpu.h>
#include <bsp/config.h>

// --------------------------------------------------------------
struct hypos *hypos_list;
struct vcpu  *hypos_vcpus[NR_CPUS] __read_mostly;


void __bootfunc hypos_setup(void)
{
    vcpu_sched_setup();
    set_current(hypos_vcpus[0]);
}

// --------------------------------------------------------------
#if IS_IMPLEMENTED(__HYPOS_IMPL)

DEFINE_PER_CPU(struct vcpu *, curr_vcpu);

static void do_idle(void)
{
    unsigned int cpu = smp_processor_id();

    rcu_idle_enter(cpu);
    /* rcu_idle_enter() can raise TIMER_SOFTIRQ. Process it now. */
    process_pending_softirqs();

    local_irq_disable();
    if ( cpu_is_haltable(cpu) )
    {
        dsb(sy);
        wfi();
    }
    local_irq_enable();

    rcu_idle_exit(cpu);
}

static void noreturn idle_loop(void)
{
    unsigned int cpu = smp_processor_id();

    for ( ; ; )
    {
        if ( cpu_is_offline(cpu) )
            stop_cpu();

        /* Are we here for running vcpu context tasklets, or for idling? */
        if ( unlikely(tasklet_work_to_do(cpu)) )
        {
            do_tasklet();
            /* Livepatch work is always kicked off via a tasklet. */
            check_for_livepatch_work();
        }
        /*
         * Test softirqs twice --- first to see if should even try scrubbing
         * and then, after it is done, whether softirqs became pending
         * while we were scrubbing.
         */
        else if ( !softirq_pending(cpu) && !scrub_free_pages() &&
                  !softirq_pending(cpu) )
            do_idle();

        do_softirq();
    }
}

static void ctxt_switch_from(struct vcpu *p)
{
    /* When the idle VCPU is running, Xen will always stay in hypervisor
     * mode. Therefore we don't need to save the context of an idle VCPU.
     */
    if ( is_idle_vcpu(p) )
        return;

    p2m_save_state(p);

    /* CP 15 */
    p->arch.csselr = READ_SYSREG(CSSELR_EL1);

    /* VFP */
    vfp_save_state(p);

    /* Control Registers */
    p->arch.cpacr = READ_SYSREG(CPACR_EL1);

    p->arch.contextidr = READ_SYSREG(CONTEXTIDR_EL1);
    p->arch.tpidr_el0 = READ_SYSREG(TPIDR_EL0);
    p->arch.tpidrro_el0 = READ_SYSREG(TPIDRRO_EL0);
    p->arch.tpidr_el1 = READ_SYSREG(TPIDR_EL1);

    /* Arch timer */
    p->arch.cntkctl = READ_SYSREG(CNTKCTL_EL1);
    virt_timer_save(p);

    if ( is_32bit_domain(p->domain) && cpu_has_thumbee )
    {
        p->arch.teecr = READ_SYSREG(TEECR32_EL1);
        p->arch.teehbr = READ_SYSREG(TEEHBR32_EL1);
    }

#ifdef CONFIG_ARM_32
    p->arch.joscr = READ_CP32(JOSCR);
    p->arch.jmcr = READ_CP32(JMCR);
#endif

    isb();

    /* MMU */
    p->arch.vbar = READ_SYSREG(VBAR_EL1);
    p->arch.ttbcr = READ_SYSREG(TCR_EL1);
    p->arch.ttbr0 = READ_SYSREG64(TTBR0_EL1);
    p->arch.ttbr1 = READ_SYSREG64(TTBR1_EL1);
    if ( is_32bit_domain(p->domain) )
        p->arch.dacr = READ_SYSREG(DACR32_EL2);
    p->arch.par = read_sysreg_par();
#if defined(CONFIG_ARM_32)
    p->arch.mair0 = READ_CP32(MAIR0);
    p->arch.mair1 = READ_CP32(MAIR1);
    p->arch.amair0 = READ_CP32(AMAIR0);
    p->arch.amair1 = READ_CP32(AMAIR1);
#else
    p->arch.mair = READ_SYSREG64(MAIR_EL1);
    p->arch.amair = READ_SYSREG64(AMAIR_EL1);
#endif

    /* Fault Status */
#if defined(CONFIG_ARM_32)
    p->arch.dfar = READ_CP32(DFAR);
    p->arch.ifar = READ_CP32(IFAR);
    p->arch.dfsr = READ_CP32(DFSR);
#elif defined(CONFIG_ARM_64)
    p->arch.far = READ_SYSREG64(FAR_EL1);
    p->arch.esr = READ_SYSREG64(ESR_EL1);
#endif

    if ( is_32bit_domain(p->domain) )
        p->arch.ifsr  = READ_SYSREG(IFSR32_EL2);
    p->arch.afsr0 = READ_SYSREG(AFSR0_EL1);
    p->arch.afsr1 = READ_SYSREG(AFSR1_EL1);

    /* XXX MPU */

    /* VGIC */
    gic_save_state(p);

    isb();
}

static void ctxt_switch_to(struct vcpu *n)
{
    register_t vpidr;

    /* When the idle VCPU is running, Xen will always stay in hypervisor
     * mode. Therefore we don't need to restore the context of an idle VCPU.
     */
    if ( is_idle_vcpu(n) )
        return;

    vpidr = READ_SYSREG(MIDR_EL1);
    WRITE_SYSREG(vpidr, VPIDR_EL2);
    WRITE_SYSREG(n->arch.vmpidr, VMPIDR_EL2);

    /* VGIC */
    gic_restore_state(n);

    /* XXX MPU */

    /* Fault Status */
#if defined(CONFIG_ARM_32)
    WRITE_CP32(n->arch.dfar, DFAR);
    WRITE_CP32(n->arch.ifar, IFAR);
    WRITE_CP32(n->arch.dfsr, DFSR);
#elif defined(CONFIG_ARM_64)
    WRITE_SYSREG64(n->arch.far, FAR_EL1);
    WRITE_SYSREG64(n->arch.esr, ESR_EL1);
#endif

    if ( is_32bit_domain(n->domain) )
        WRITE_SYSREG(n->arch.ifsr, IFSR32_EL2);
    WRITE_SYSREG(n->arch.afsr0, AFSR0_EL1);
    WRITE_SYSREG(n->arch.afsr1, AFSR1_EL1);

    /* MMU */
    WRITE_SYSREG(n->arch.vbar, VBAR_EL1);
    WRITE_SYSREG(n->arch.ttbcr, TCR_EL1);
    WRITE_SYSREG64(n->arch.ttbr0, TTBR0_EL1);
    WRITE_SYSREG64(n->arch.ttbr1, TTBR1_EL1);

    /*
     * Erratum #852523 (Cortex-A57) or erratum #853709 (Cortex-A72):
     * DACR32_EL2 must be restored before one of the
     * following sysregs: SCTLR_EL1, TCR_EL1, TTBR0_EL1, TTBR1_EL1 or
     * CONTEXTIDR_EL1.
     */
    if ( is_32bit_domain(n->domain) )
        WRITE_SYSREG(n->arch.dacr, DACR32_EL2);
    WRITE_SYSREG64(n->arch.par, PAR_EL1);
#if defined(CONFIG_ARM_32)
    WRITE_CP32(n->arch.mair0, MAIR0);
    WRITE_CP32(n->arch.mair1, MAIR1);
    WRITE_CP32(n->arch.amair0, AMAIR0);
    WRITE_CP32(n->arch.amair1, AMAIR1);
#elif defined(CONFIG_ARM_64)
    WRITE_SYSREG64(n->arch.mair, MAIR_EL1);
    WRITE_SYSREG64(n->arch.amair, AMAIR_EL1);
#endif
    isb();

    /*
     * ARM64_WORKAROUND_AT_SPECULATE: The P2M should be restored after
     * the stage-1 MMU sysregs have been restored.
     */
    p2m_restore_state(n);

    /* Control Registers */
    WRITE_SYSREG(n->arch.cpacr, CPACR_EL1);

    /*
     * This write to sysreg CONTEXTIDR_EL1 ensures we don't hit erratum
     * #852523 (Cortex-A57) or #853709 (Cortex-A72).
     * I.e DACR32_EL2 is not correctly synchronized.
     */
    WRITE_SYSREG(n->arch.contextidr, CONTEXTIDR_EL1);
    WRITE_SYSREG(n->arch.tpidr_el0, TPIDR_EL0);
    WRITE_SYSREG(n->arch.tpidrro_el0, TPIDRRO_EL0);
    WRITE_SYSREG(n->arch.tpidr_el1, TPIDR_EL1);

    if ( is_32bit_domain(n->domain) && cpu_has_thumbee )
    {
        WRITE_SYSREG(n->arch.teecr, TEECR32_EL1);
        WRITE_SYSREG(n->arch.teehbr, TEEHBR32_EL1);
    }

#ifdef CONFIG_ARM_32
    WRITE_CP32(n->arch.joscr, JOSCR);
    WRITE_CP32(n->arch.jmcr, JMCR);
#endif

    /*
     * CPTR_EL2 needs to be written before calling vfp_restore_state, a
     * synchronization instruction is expected after the write (isb)
     */
    WRITE_SYSREG(n->arch.cptr_el2, CPTR_EL2);
    isb();

    /* VFP - call vfp_restore_state after writing on CPTR_EL2 + isb */
    vfp_restore_state(n);

    /* CP 15 */
    WRITE_SYSREG(n->arch.csselr, CSSELR_EL1);

    isb();

    /* This is could trigger an hardware interrupt from the virtual
     * timer. The interrupt needs to be injected into the guest. */
    WRITE_SYSREG(n->arch.cntkctl, CNTKCTL_EL1);
    virt_timer_restore(n);

    WRITE_SYSREG(n->arch.mdcr_el2, MDCR_EL2);
}

static void schedule_tail(struct vcpu *prev)
{
    ASSERT(prev != current);

    ctxt_switch_from(prev);

    ctxt_switch_to(current);

    local_irq_enable();

    sched_context_switched(prev, current);

    update_runstate_area(current);

    /* Ensure that the vcpu has an up-to-date time base. */
    update_vcpu_system_time(current);
}

extern void noreturn return_to_new_vcpu32(void);
extern void noreturn return_to_new_vcpu64(void);

static void continue_new_vcpu(struct vcpu *prev)
{
    current->arch.actlr = READ_SYSREG(ACTLR_EL1);
    processor_vcpu_initialise(current);

    schedule_tail(prev);

    if ( is_idle_vcpu(current) )
        reset_stack_and_jump(idle_loop);
    else if ( is_32bit_domain(current->domain) )
        /* check_wakeup_from_wait(); */
        reset_stack_and_jump(return_to_new_vcpu32);
    else
        /* check_wakeup_from_wait(); */
        reset_stack_and_jump(return_to_new_vcpu64);
}

void context_switch(struct vcpu *prev, struct vcpu *next)
{
    ASSERT(local_irq_is_enabled());
    ASSERT(prev != next);
    ASSERT(!vcpu_cpu_dirty(next));

    update_runstate_area(prev);

    local_irq_disable();

    set_current(next);

    prev = __context_switch(prev, next);

    schedule_tail(prev);
}

void continue_running(struct vcpu *same)
{
    /* Nothing to do */
}

void sync_local_execstate(void)
{
    /* Nothing to do -- no lazy switching */
}

void sync_vcpu_execstate(struct vcpu *v)
{
    /*
     * We don't support lazy switching.
     *
     * However the context may have been saved from a remote pCPU so we
     * need a barrier to ensure it is observed before continuing.
     *
     * Per vcpu_context_saved(), the context can be observed when
     * v->is_running is false (the caller should check it before calling
     * this function).
     *
     * Note this is a full barrier to also prevent update of the context
     * to happen before it was observed.
     */
    smp_mb();
}

#define NEXT_ARG(fmt, args)                                                 \
({                                                                          \
    unsigned long __arg;                                                    \
    switch ( *(fmt)++ )                                                     \
    {                                                                       \
    case 'i': __arg = (unsigned long)va_arg(args, unsigned int);  break;    \
    case 'l': __arg = (unsigned long)va_arg(args, unsigned long); break;    \
    case 'h': __arg = (unsigned long)va_arg(args, void *);        break;    \
    default:  goto bad_fmt;                                                 \
    }                                                                       \
    __arg;                                                                  \
})

unsigned long hypercall_create_continuation(
    unsigned int op, const char *format, ...)
{
    struct mc_state *mcs = &current->mc_state;
    struct cpu_user_regs *regs;
    const char *p = format;
    unsigned long arg, rc;
    unsigned int i;
    /* SAF-4-safe allowed variadic function */
    va_list args;

    current->hcall_preempted = true;

    va_start(args, format);

    if ( mcs->flags & MCSF_in_multicall )
    {
        for ( i = 0; *p != '\0'; i++ )
            mcs->call.args[i] = NEXT_ARG(p, args);

        /* Return value gets written back to mcs->call.result */
        rc = mcs->call.result;
    }
    else
    {
        regs = guest_cpu_user_regs();

#ifdef CONFIG_ARM_64
        if ( !is_32bit_domain(current->domain) )
        {
            regs->x16 = op;

            for ( i = 0; *p != '\0'; i++ )
            {
                arg = NEXT_ARG(p, args);

                switch ( i )
                {
                case 0: regs->x0 = arg; break;
                case 1: regs->x1 = arg; break;
                case 2: regs->x2 = arg; break;
                case 3: regs->x3 = arg; break;
                case 4: regs->x4 = arg; break;
                case 5: regs->x5 = arg; break;
                }
            }

            /* Return value gets written back to x0 */
            rc = regs->x0;
        }
        else
#endif
        {
            regs->r12 = op;

            for ( i = 0; *p != '\0'; i++ )
            {
                arg = NEXT_ARG(p, args);

                switch ( i )
                {
                case 0: regs->r0 = arg; break;
                case 1: regs->r1 = arg; break;
                case 2: regs->r2 = arg; break;
                case 3: regs->r3 = arg; break;
                case 4: regs->r4 = arg; break;
                case 5: regs->r5 = arg; break;
                }
            }

            /* Return value gets written back to r0 */
            rc = regs->r0;
        }
    }

    va_end(args);

    return rc;

 bad_fmt:
    va_end(args);
    gprintk(XENLOG_ERR, "Bad hypercall continuation format '%c'\n", *p);
    ASSERT_UNREACHABLE();
    domain_crash(current->domain);
    return 0;
}

#undef NEXT_ARG

void startup_cpu_idle_loop(void)
{
    struct vcpu *v = current;

    ASSERT(is_idle_vcpu(v));
    /* TODO
       cpumask_set_cpu(v->processor, v->domain->dirty_cpumask);
       v->dirty_cpu = v->processor;
    */

    reset_stack_and_jump(idle_loop);
}

struct domain *alloc_domain_struct(void)
{
    struct domain *d;
    BUILD_BUG_ON(sizeof(*d) > PAGE_SIZE);
    d = alloc_xenheap_pages(0, 0);
    if ( d == NULL )
        return NULL;

    clear_page(d);
    return d;
}

void free_domain_struct(struct domain *d)
{
    free_xenheap_page(d);
}

void dump_pageframe_info(struct domain *d)
{

}

/*
 * The new VGIC has a bigger per-IRQ structure, so we need more than one
 * page on ARM64. Cowardly increase the limit in this case.
 */
#if defined(CONFIG_NEW_VGIC) && defined(CONFIG_ARM_64)
#define MAX_PAGES_PER_VCPU  2
#else
#define MAX_PAGES_PER_VCPU  1
#endif

struct vcpu *alloc_vcpu_struct(const struct domain *d)
{
    struct vcpu *v;

    BUILD_BUG_ON(sizeof(*v) > MAX_PAGES_PER_VCPU * PAGE_SIZE);
    v = alloc_xenheap_pages(get_order_from_bytes(sizeof(*v)), 0);
    if ( v != NULL )
    {
        unsigned int i;

        for ( i = 0; i < DIV_ROUND_UP(sizeof(*v), PAGE_SIZE); i++ )
            clear_page((void *)v + i * PAGE_SIZE);
    }

    return v;
}

void free_vcpu_struct(struct vcpu *v)
{
    free_xenheap_pages(v, get_order_from_bytes(sizeof(*v)));
}

int arch_vcpu_create(struct vcpu *v)
{
    int rc = 0;

    BUILD_BUG_ON( sizeof(struct cpu_info) > STACK_SIZE );

    v->arch.stack = alloc_xenheap_pages(STACK_ORDER, MEMF_node(vcpu_to_node(v)));
    if ( v->arch.stack == NULL )
        return -ENOMEM;

    v->arch.cpu_info = (struct cpu_info *)(v->arch.stack
                                           + STACK_SIZE
                                           - sizeof(struct cpu_info));
    memset(v->arch.cpu_info, 0, sizeof(*v->arch.cpu_info));

    v->arch.saved_context.sp = (register_t)v->arch.cpu_info;
    v->arch.saved_context.pc = (register_t)continue_new_vcpu;

    /* Idle VCPUs don't need the rest of this setup */
    if ( is_idle_vcpu(v) )
        return rc;

    v->arch.sctlr = SCTLR_GUEST_INIT;

    v->arch.vmpidr = MPIDR_SMP | vcpuid_to_vaffinity(v->vcpu_id);

    v->arch.cptr_el2 = get_default_cptr_flags();
    if ( is_sve_domain(v->domain) )
    {
        if ( (rc = sve_context_init(v)) != 0 )
            goto fail;
        v->arch.cptr_el2 &= ~HCPTR_CP(8);
    }

    v->arch.hcr_el2 = get_default_hcr_flags();

    v->arch.mdcr_el2 = HDCR_TDRA | HDCR_TDOSA | HDCR_TDA;
    if ( !(v->domain->options & XEN_DOMCTL_CDF_vpmu) )
        v->arch.mdcr_el2 |= HDCR_TPM | HDCR_TPMCR;

    if ( (rc = vcpu_vgic_init(v)) != 0 )
        goto fail;

    if ( (rc = vcpu_vtimer_init(v)) != 0 )
        goto fail;

    /*
     * The workaround 2 (i.e SSBD mitigation) is enabled by default if
     * supported.
     */
    if ( get_ssbd_state() == ARM_SSBD_RUNTIME )
        v->arch.cpu_info->flags |= CPUINFO_WORKAROUND_2_FLAG;

    return rc;

fail:
    arch_vcpu_destroy(v);
    return rc;
}

void arch_vcpu_destroy(struct vcpu *v)
{
    if ( is_sve_domain(v->domain) )
        sve_context_free(v);
    vcpu_timer_destroy(v);
    vcpu_vgic_free(v);
    free_xenheap_pages(v->arch.stack, STACK_ORDER);
}

void vcpu_switch_to_aarch64_mode(struct vcpu *v)
{
    v->arch.hcr_el2 |= HCR_RW;
}

int arch_sanitise_domain_config(struct xen_domctl_createdomain *config)
{
    unsigned int max_vcpus;
    unsigned int flags_required = (XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap);
    unsigned int flags_optional = (XEN_DOMCTL_CDF_iommu | XEN_DOMCTL_CDF_vpmu);
    unsigned int sve_vl_bits = sve_decode_vl(config->arch.sve_vl);

    if ( (config->flags & ~flags_optional) != flags_required )
    {
        dprintk(XENLOG_INFO, "Unsupported configuration %#x\n",
                config->flags);
        return -EINVAL;
    }

    /* Check feature flags */
    if ( sve_vl_bits > 0 )
    {
        unsigned int zcr_max_bits = get_sys_vl_len();

        if ( !zcr_max_bits )
        {
            dprintk(XENLOG_INFO, "SVE is unsupported on this machine.\n");
            return -EINVAL;
        }

        if ( sve_vl_bits > zcr_max_bits )
        {
            dprintk(XENLOG_INFO,
                    "Requested SVE vector length (%u) > supported length (%u)\n",
                    sve_vl_bits, zcr_max_bits);
            return -EINVAL;
        }
    }

    /* The P2M table must always be shared between the CPU and the IOMMU */
    if ( config->iommu_opts & XEN_DOMCTL_IOMMU_no_sharept )
    {
        dprintk(XENLOG_INFO,
                "Unsupported iommu option: XEN_DOMCTL_IOMMU_no_sharept\n");
        return -EINVAL;
    }

    /* Fill in the native GIC version, passed back to the toolstack. */
    if ( config->arch.gic_version == XEN_DOMCTL_CONFIG_GIC_NATIVE )
    {
        switch ( gic_hw_version() )
        {
        case GIC_V2:
            config->arch.gic_version = XEN_DOMCTL_CONFIG_GIC_V2;
            break;

        case GIC_V3:
            config->arch.gic_version = XEN_DOMCTL_CONFIG_GIC_V3;
            break;

        default:
            ASSERT_UNREACHABLE();
            return -EINVAL;
        }
    }

    max_vcpus = min(vgic_max_vcpus(config->arch.gic_version), MAX_VIRT_CPUS);

    if ( max_vcpus == 0 )
    {
        dprintk(XENLOG_INFO, "Unsupported GIC version\n");
        return -EINVAL;
    }

    if ( config->max_vcpus > max_vcpus )
    {
        dprintk(XENLOG_INFO, "Requested vCPUs (%u) exceeds max (%u)\n",
                config->max_vcpus, max_vcpus);
        return -EINVAL;
    }

    if ( config->arch.tee_type != XEN_DOMCTL_CONFIG_TEE_NONE &&
         config->arch.tee_type != tee_get_type() )
    {
        dprintk(XENLOG_INFO, "Unsupported TEE type\n");
        return -EINVAL;
    }

    return 0;
}

int arch_domain_create(struct domain *d,
                       struct xen_domctl_createdomain *config,
                       unsigned int flags)
{
    unsigned int count = 0;
    int rc;

    BUILD_BUG_ON(GUEST_MAX_VCPUS < MAX_VIRT_CPUS);

    /* Idle domains do not need this setup */
    if ( is_idle_domain(d) )
        return 0;

    ASSERT(config != NULL);

    if ( (rc = iommu_domain_init(d, config->iommu_opts)) != 0 )
        goto fail;

    if ( (rc = p2m_init(d)) != 0 )
        goto fail;

    rc = -ENOMEM;
    if ( (d->shared_info = alloc_xenheap_pages(0, 0)) == NULL )
        goto fail;

    clear_page(d->shared_info);
    share_xen_page_with_guest(virt_to_page(d->shared_info), d, SHARE_rw);

    switch ( config->arch.gic_version )
    {
    case XEN_DOMCTL_CONFIG_GIC_V2:
        d->arch.vgic.version = GIC_V2;
        break;

    case XEN_DOMCTL_CONFIG_GIC_V3:
        d->arch.vgic.version = GIC_V3;
        break;

    default:
        BUG();
    }

    if ( (rc = domain_vgic_register(d, &count)) != 0 )
        goto fail;

    count += domain_vpci_get_num_mmio_handlers(d);

    if ( (rc = domain_io_init(d, count + MAX_IO_HANDLER)) != 0 )
        goto fail;

    if ( (rc = domain_vgic_init(d, config->arch.nr_spis)) != 0 )
        goto fail;

    if ( (rc = domain_vtimer_init(d, &config->arch)) != 0 )
        goto fail;

    if ( (rc = tee_domain_init(d, config->arch.tee_type)) != 0 )
        goto fail;

    update_domain_wallclock_time(d);

    if ( !is_hardware_domain(d) )
    {
        d->arch.evtchn_irq = GUEST_EVTCHN_PPI;
        /* At this stage vgic_reserve_virq should never fail */
        if ( !vgic_reserve_virq(d, GUEST_EVTCHN_PPI) )
            BUG();
    }

    if ( is_hardware_domain(d) && (rc = domain_vuart_init(d)) )
        goto fail;

    if ( (rc = domain_vpci_init(d)) != 0 )
        goto fail;

    d->arch.sve_vl = config->arch.sve_vl;


    return 0;

fail:
    d->is_dying = DOMDYING_dead;
    arch_domain_destroy(d);

    return rc;
}

int arch_domain_teardown(struct domain *d)
{
    int ret = 0;

    BUG_ON(!d->is_dying);

    /* See domain_teardown() for an explanation of all of this magic. */
    switch ( d->teardown.arch_val )
    {
#define PROGRESS(x)                             \
        d->teardown.arch_val = PROG_ ## x;      \
        fallthrough;                            \
    case PROG_ ## x

        enum {
            PROG_none,
            PROG_tee,
            PROG_done,
        };

    case PROG_none:
        BUILD_BUG_ON(PROG_none != 0);

    PROGRESS(tee):
        ret = tee_domain_teardown(d);
        if ( ret )
            return ret;

    PROGRESS(done):
        break;

#undef PROGRESS

    default:
        BUG();
    }

    return 0;
}

void arch_domain_destroy(struct domain *d)
{
    /* IOMMU page table is shared with P2M, always call
     * iommu_domain_destroy() before p2m_final_teardown().
     */
    iommu_domain_destroy(d);
    p2m_final_teardown(d);
    domain_vgic_free(d);
    domain_vuart_free(d);
    free_xenheap_page(d->shared_info);
    domain_io_free(d);
}

void arch_domain_shutdown(struct domain *d)
{
}

void arch_domain_pause(struct domain *d)
{
}

void arch_domain_unpause(struct domain *d)
{
}

int arch_domain_soft_reset(struct domain *d)
{
    return -ENOSYS;
}

void arch_domain_creation_finished(struct domain *d)
{
    p2m_domain_creation_finished(d);
}

static int is_guest_pv32_psr(uint32_t psr)
{
    switch (psr & PSR_MODE_MASK)
    {
    case PSR_MODE_USR:
    case PSR_MODE_FIQ:
    case PSR_MODE_IRQ:
    case PSR_MODE_SVC:
    case PSR_MODE_ABT:
    case PSR_MODE_UND:
    case PSR_MODE_SYS:
        return 1;
    case PSR_MODE_MON:
    case PSR_MODE_HYP:
    default:
        return 0;
    }
}

static int is_guest_pv64_psr(uint64_t psr)
{
    if ( psr & PSR_MODE_BIT )
        return 0;

    switch (psr & PSR_MODE_MASK)
    {
    case PSR_MODE_EL1h:
    case PSR_MODE_EL1t:
    case PSR_MODE_EL0t:
        return 1;
    case PSR_MODE_EL3h:
    case PSR_MODE_EL3t:
    case PSR_MODE_EL2h:
    case PSR_MODE_EL2t:
    default:
        return 0;
    }
}

int arch_set_info_guest(
    struct vcpu *v, vcpu_guest_context_u c)
{
    struct vcpu_guest_context *ctxt = c.nat;
    struct vcpu_guest_core_regs *regs = &c.nat->user_regs;

    if ( is_32bit_domain(v->domain) )
    {
        if ( !is_guest_pv32_psr(regs->cpsr) )
            return -EINVAL;

        if ( regs->spsr_svc && !is_guest_pv32_psr(regs->spsr_svc) )
            return -EINVAL;
        if ( regs->spsr_abt && !is_guest_pv32_psr(regs->spsr_abt) )
            return -EINVAL;
        if ( regs->spsr_und && !is_guest_pv32_psr(regs->spsr_und) )
            return -EINVAL;
        if ( regs->spsr_irq && !is_guest_pv32_psr(regs->spsr_irq) )
            return -EINVAL;
        if ( regs->spsr_fiq && !is_guest_pv32_psr(regs->spsr_fiq) )
            return -EINVAL;
    }
#ifdef CONFIG_ARM_64
    else
    {
        if ( !is_guest_pv64_psr(regs->cpsr) )
            return -EINVAL;

        if ( regs->spsr_el1 && !is_guest_pv64_psr(regs->spsr_el1) )
            return -EINVAL;
    }
#endif

    vcpu_regs_user_to_hyp(v, regs);

    v->arch.sctlr = ctxt->sctlr;
    v->arch.ttbr0 = ctxt->ttbr0;
    v->arch.ttbr1 = ctxt->ttbr1;
    v->arch.ttbcr = ctxt->ttbcr;

    v->is_initialised = 1;

    if ( ctxt->flags & VGCF_online )
        clear_bit(_VPF_down, &v->pause_flags);
    else
        set_bit(_VPF_down, &v->pause_flags);

    return 0;
}

int arch_initialise_vcpu(struct vcpu *v, XEN_GUEST_HANDLE_PARAM(void) arg)
{
    ASSERT_UNREACHABLE();
    return -EOPNOTSUPP;
}

int arch_vcpu_reset(struct vcpu *v)
{
    vcpu_end_shutdown_deferral(v);
    return 0;
}

static int relinquish_memory(struct domain *d, struct page_list_head *list)
{
    struct page_info *page, *tmp;
    int               ret = 0;

    /* Use a recursive lock, as we may enter 'free_domheap_page'. */
    rspin_lock(&d->page_alloc_lock);

    page_list_for_each_safe( page, tmp, list )
    {
        if ( unlikely(!get_page(page, d)) )
            continue;

        put_page_alloc_ref(page);
        put_page(page);

        if ( hypercall_preempt_check() )
        {
            ret = -ERESTART;
            goto out;
        }
    }

  out:
    rspin_unlock(&d->page_alloc_lock);
    return ret;
}

enum {
    PROG_pci = 1,
    PROG_tee,
    PROG_xen,
    PROG_page,
    PROG_mapping,
    PROG_p2m_root,
    PROG_p2m,
    PROG_p2m_pool,
    PROG_done,
};

#define PROGRESS(x)                         \
    d->arch.rel_priv = PROG_ ## x;          \
    /* Fallthrough */                       \
    case PROG_ ## x

int domain_relinquish_resources(struct domain *d)
{
    int ret = 0;

    /*
     * This hypercall can take minutes of wallclock time to complete.  This
     * logic implements a co-routine, stashing state in struct domain across
     * hypercall continuation boundaries.
     */
    switch ( d->arch.rel_priv )
    {
    case 0:
        ret = iommu_release_dt_devices(d);
        if ( ret )
            return ret;

        /*
         * Release the resources allocated for vpl011 which were
         * allocated via a DOMCTL call XEN_DOMCTL_vuart_op.
         */
        domain_vpl011_deinit(d);

    PROGRESS(tee):
        ret = tee_relinquish_resources(d);
        if (ret )
            return ret;

    PROGRESS(xen):
        ret = relinquish_memory(d, &d->xenpage_list);
        if ( ret )
            return ret;

    PROGRESS(page):
        ret = relinquish_memory(d, &d->page_list);
        if ( ret )
            return ret;

    PROGRESS(mapping):
        ret = relinquish_p2m_mapping(d);
        if ( ret )
            return ret;

    PROGRESS(p2m_root):
        p2m_clear_root_pages(&d->arch.p2m);

    PROGRESS(p2m):
        ret = p2m_teardown(d);
        if ( ret )
            return ret;

    PROGRESS(p2m_pool):
        ret = p2m_teardown_allocation(d);
        if( ret )
            return ret;

    PROGRESS(done):
        break;

    default:
        BUG();
    }

    return 0;
}

#undef PROGRESS

void arch_dump_domain_info(struct domain *d)
{
    p2m_dump_info(d);
}


long do_vcpu_op(int cmd, unsigned int vcpuid, XEN_GUEST_HANDLE_PARAM(void) arg)
{
    struct domain *d = current->domain;
    struct vcpu *v;

    if ( (v = domain_vcpu(d, vcpuid)) == NULL )
        return -ENOENT;

    switch ( cmd )
    {
        case VCPUOP_register_vcpu_info:
        case VCPUOP_register_runstate_memory_area:
            return common_vcpu_op(cmd, v, arg);
        default:
            return -EINVAL;
    }
}

void arch_dump_vcpu_info(struct vcpu *v)
{
    gic_dump_info(v);
    gic_dump_vgic_info(v);
}

void vcpu_mark_events_pending(struct vcpu *v)
{
    bool already_pending = guest_test_and_set_bit(v->domain,
        0, (unsigned long *)&vcpu_info(v, evtchn_upcall_pending));

    if ( already_pending )
        return;

    vgic_inject_irq(v->domain, v, v->domain->arch.evtchn_irq, true);
}

void vcpu_update_evtchn_irq(struct vcpu *v)
{
    bool pending = vcpu_info(v, evtchn_upcall_pending);

    vgic_inject_irq(v->domain, v, v->domain->arch.evtchn_irq, pending);
}

void vcpu_block_unless_event_pending(struct vcpu *v)
{
    vcpu_block();
    if ( local_events_need_delivery_nomask() )
        vcpu_unblock(current);
}

void vcpu_kick(struct vcpu *v)
{
    bool running = v->is_running;

    vcpu_unblock(v);
    if ( running && v != current )
    {
        perfc_incr(vcpu_kick);
        smp_send_event_check_mask(cpumask_of(v->processor));
    }
}

// --------------------------------------------------------------

/* Linux config option: propageted to domain0 */
/* xen_processor_pmbits: xen control Cx, Px, ... */
unsigned int xen_processor_pmbits = XEN_PROCESSOR_PM_PX;

/* opt_dom0_vcpus_pin: If true, dom0 VCPUs are pinned. */
bool opt_dom0_vcpus_pin;
boolean_param("dom0_vcpus_pin", opt_dom0_vcpus_pin);

/* Protect updates/reads (resp.) of domain_list and domain_hash. */
DEFINE_SPINLOCK(domlist_update_lock);
DEFINE_RCU_READ_LOCK(domlist_read_lock);

#define DOMAIN_HASH_SIZE 256
#define DOMAIN_HASH(_id) ((int)(_id)&(DOMAIN_HASH_SIZE-1))
static struct domain *domain_hash[DOMAIN_HASH_SIZE];
struct domain *domain_list;

struct domain *hardware_domain __read_mostly;

#ifdef CONFIG_LATE_HWDOM
domid_t hardware_domid __read_mostly;
integer_param("hardware_dom", hardware_domid);
#endif

/* Private domain structs for DOMID_XEN, DOMID_IO, etc. */
struct domain *__read_mostly dom_xen;
struct domain *__read_mostly dom_io;
#ifdef CONFIG_MEM_SHARING
struct domain *__read_mostly dom_cow;
#endif

struct vcpu *idle_vcpu[NR_CPUS] __read_mostly;

vcpu_info_t dummy_vcpu_info;

bool __read_mostly vmtrace_available;

bool __read_mostly vpmu_is_available;

static void __domain_finalise_shutdown(struct domain *d)
{
    struct vcpu *v;

    BUG_ON(!spin_is_locked(&d->shutdown_lock));

    if ( d->is_shut_down )
        return;

    for_each_vcpu ( d, v )
        if ( !v->paused_for_shutdown )
            return;

    d->is_shut_down = 1;
    if ( (d->shutdown_code == SHUTDOWN_suspend) && d->suspend_evtchn )
        evtchn_send(d, d->suspend_evtchn);
    else
        send_global_virq(VIRQ_DOM_EXC);
}

static void vcpu_check_shutdown(struct vcpu *v)
{
    struct domain *d = v->domain;

    spin_lock(&d->shutdown_lock);

    if ( d->is_shutting_down )
    {
        if ( !v->paused_for_shutdown )
            vcpu_pause_nosync(v);
        v->paused_for_shutdown = 1;
        v->defer_shutdown = 0;
        __domain_finalise_shutdown(d);
    }

    spin_unlock(&d->shutdown_lock);
}

static void vcpu_info_reset(struct vcpu *v)
{
    struct domain *d = v->domain;

    v->vcpu_info_area.map =
        ((v->vcpu_id < XEN_LEGACY_MAX_VCPUS)
         ? (vcpu_info_t *)&shared_info(d, vcpu_info[v->vcpu_id])
         : &dummy_vcpu_info);
}

static void vmtrace_free_buffer(struct vcpu *v)
{
    const struct domain *d = v->domain;
    struct page_info *pg = v->vmtrace.pg;
    unsigned int i;

    if ( !pg )
        return;

    v->vmtrace.pg = NULL;

    for ( i = 0; i < (d->vmtrace_size >> PAGE_SHIFT); i++ )
    {
        put_page_alloc_ref(&pg[i]);
        put_page_and_type(&pg[i]);
    }
}

static int vmtrace_alloc_buffer(struct vcpu *v)
{
    struct domain *d = v->domain;
    struct page_info *pg;
    unsigned int i;

    if ( !d->vmtrace_size )
        return 0;

    pg = alloc_domheap_pages(d, get_order_from_bytes(d->vmtrace_size),
                             MEMF_no_refcount);
    if ( !pg )
        return -ENOMEM;

    for ( i = 0; i < (d->vmtrace_size >> PAGE_SHIFT); i++ )
        if ( unlikely(!get_page_and_type(&pg[i], d, PGT_writable_page)) )
            /*
             * The domain can't possibly know about this page yet, so failure
             * here is a clear indication of something fishy going on.
             */
            goto refcnt_err;

    /*
     * We must only let vmtrace_free_buffer() take any action in the success
     * case when we've taken all the refs it intends to drop.
     */
    v->vmtrace.pg = pg;
    return 0;

 refcnt_err:
    /*
     * We can theoretically reach this point if someone has taken 2^43 refs on
     * the frames in the time the above loop takes to execute, or someone has
     * made a blind decrease reservation hypercall and managed to pick the
     * right mfn.  Free the memory we safely can, and leak the rest.
     */
    while ( i-- )
    {
        put_page_alloc_ref(&pg[i]);
        put_page_and_type(&pg[i]);
    }

    return -ENODATA;
}

/*
 * Release resources held by a vcpu.  There may or may not be live references
 * to the vcpu, and it may or may not be fully constructed.
 *
 * If d->is_dying is DOMDYING_dead, this must not return non-zero.
 */
static int vcpu_teardown(struct vcpu *v)
{
    vmtrace_free_buffer(v);

    return 0;
}

/*
 * Destoy a vcpu once all references to it have been dropped.  Used either
 * from domain_destroy()'s RCU path, or from the vcpu_create() error path
 * before the vcpu is placed on the domain's vcpu list.
 */
static void vcpu_destroy(struct vcpu *v)
{
    free_vcpu_struct(v);
}

struct vcpu *vcpu_create(struct domain *d, unsigned int vcpu_id)
{
    struct vcpu *v;

    /*
     * Sanity check some input expectations:
     * - vcpu_id should be bounded by d->max_vcpus, and not previously
     *   allocated.
     * - VCPUs should be tightly packed and allocated in ascending order,
     *   except for the idle domain which may vary based on PCPU numbering.
     */
    if ( vcpu_id >= d->max_vcpus || d->vcpu[vcpu_id] ||
         (!is_idle_domain(d) && vcpu_id && !d->vcpu[vcpu_id - 1]) )
    {
        ASSERT_UNREACHABLE();
        return NULL;
    }

    if ( (v = alloc_vcpu_struct(d)) == NULL )
        return NULL;

    v->domain = d;
    v->vcpu_id = vcpu_id;
    v->dirty_cpu = VCPU_CPU_CLEAN;

    rwlock_init(&v->virq_lock);

    tasklet_init(&v->continue_hypercall_tasklet, NULL, NULL);

    grant_table_init_vcpu(v);

    if ( is_idle_domain(d) )
    {
        v->runstate.state = RUNSTATE_running;
        v->new_state = RUNSTATE_running;
    }
    else
    {
        v->runstate.state = RUNSTATE_offline;
        v->runstate.state_entry_time = NOW();
        set_bit(_VPF_down, &v->pause_flags);
        vcpu_info_reset(v);
        init_waitqueue_vcpu(v);
    }

    if ( sched_init_vcpu(v) != 0 )
        goto fail_wq;

    if ( vmtrace_alloc_buffer(v) != 0 )
        goto fail_wq;

    if ( arch_vcpu_create(v) != 0 )
        goto fail_sched;

    d->vcpu[vcpu_id] = v;
    if ( vcpu_id != 0 )
    {
        int prev_id = v->vcpu_id - 1;
        while ( (prev_id >= 0) && (d->vcpu[prev_id] == NULL) )
            prev_id--;
        BUG_ON(prev_id < 0);
        v->next_in_list = d->vcpu[prev_id]->next_in_list;
        d->vcpu[prev_id]->next_in_list = v;
    }

    /* Must be called after making new vcpu visible to for_each_vcpu(). */
    vcpu_check_shutdown(v);

    return v;

 fail_sched:
    sched_destroy_vcpu(v);
 fail_wq:
    destroy_waitqueue_vcpu(v);

    /* Must not hit a continuation in this context. */
    if ( vcpu_teardown(v) )
        ASSERT_UNREACHABLE();

    vcpu_destroy(v);

    return NULL;
}

static int late_hwdom_init(struct domain *d)
{
#ifdef CONFIG_LATE_HWDOM
    struct domain *dom0;
    int rv;

    if ( d != hardware_domain || d->domain_id == 0 )
        return 0;

    rv = xsm_init_hardware_domain(XSM_HOOK, d);
    if ( rv )
        return rv;

    printk("Initialising hardware domain %d\n", hardware_domid);

    dom0 = rcu_lock_domain_by_id(0);
    ASSERT(dom0 != NULL);
    /*
     * Hardware resource ranges for domain 0 have been set up from
     * various sources intended to restrict the hardware domain's
     * access.  Apply these ranges to the actual hardware domain.
     *
     * Because the lists are being swapped, a side effect of this
     * operation is that Domain 0's rangesets are cleared.  Since
     * domain 0 should not be accessing the hardware when it constructs
     * a hardware domain, this should not be a problem.  Both lists
     * may be modified after this hypercall returns if a more complex
     * device model is desired.
     */
    rangeset_swap(d->irq_caps, dom0->irq_caps);
    rangeset_swap(d->iomem_caps, dom0->iomem_caps);
#ifdef CONFIG_X86
    rangeset_swap(d->arch.ioport_caps, dom0->arch.ioport_caps);
    setup_io_bitmap(d);
    setup_io_bitmap(dom0);
#endif

    rcu_unlock_domain(dom0);

    iommu_hwdom_init(d);

    return rv;
#else
    return 0;
#endif
}

#ifdef CONFIG_HAS_PIRQ

static unsigned int __read_mostly extra_hwdom_irqs;
static unsigned int __read_mostly extra_domU_irqs = 32;

static int __init parse_extra_guest_irqs(const char *s)
{
    if ( isdigit(*s) )
        extra_domU_irqs = simple_strtoul(s, &s, 0);
    if ( *s == ',' && isdigit(*++s) )
        extra_hwdom_irqs = simple_strtoul(s, &s, 0);

    return *s ? -EINVAL : 0;
}
custom_param("extra_guest_irqs", parse_extra_guest_irqs);

#endif /* CONFIG_HAS_PIRQ */

static int __init parse_dom0_param(const char *s)
{
    const char *ss;
    int rc = 0;

    do {
        int ret;

        ss = strchr(s, ',');
        if ( !ss )
            ss = strchr(s, '\0');

        ret = parse_arch_dom0_param(s, ss);
        if ( ret && !rc )
            rc = ret;

        s = ss + 1;
    } while ( *ss );

    return rc;
}
custom_param("dom0", parse_dom0_param);

/*
 * Release resources held by a domain.  There may or may not be live
 * references to the domain, and it may or may not be fully constructed.
 *
 * d->is_dying differing between DOMDYING_dying and DOMDYING_dead can be used
 * to determine if live references to the domain exist, and also whether
 * continuations are permitted.
 *
 * If d->is_dying is DOMDYING_dead, this must not return non-zero.
 */
static int domain_teardown(struct domain *d)
{
    struct vcpu *v;
    int rc;

    BUG_ON(!d->is_dying);

    /*
     * This hypercall can take minutes of wallclock time to complete.  This
     * logic implements a co-routine, stashing state in struct domain across
     * hypercall continuation boundaries.
     */
    switch ( d->teardown.val )
    {
        /*
         * Record the current progress.  Subsequent hypercall continuations
         * will logically restart work from this point.
         *
         * PROGRESS() markers must not be in the middle of loops.  The loop
         * variable isn't preserved across a continuation.  PROGRESS_VCPU()
         * markers may be used in the middle of for_each_vcpu() loops, which
         * preserve v but no other loop variables.
         *
         * To avoid redundant work, there should be a marker before each
         * function which may return -ERESTART.
         */
#define PROGRESS(x)                             \
        d->teardown.val = PROG_ ## x;           \
        fallthrough;                            \
    case PROG_ ## x

#define PROGRESS_VCPU(x)                        \
        d->teardown.val = PROG_vcpu_ ## x;      \
        d->teardown.vcpu = v;                   \
        fallthrough;                            \
    case PROG_vcpu_ ## x:                       \
        v = d->teardown.vcpu

        enum {
            PROG_none,
            PROG_gnttab_mappings,
            PROG_vcpu_teardown,
            PROG_arch_teardown,
            PROG_done,
        };

    case PROG_none:
        BUILD_BUG_ON(PROG_none != 0);

    PROGRESS(gnttab_mappings):
        rc = gnttab_release_mappings(d);
        if ( rc )
            return rc;

        for_each_vcpu ( d, v )
        {
            /* SAF-5-safe MISRA C Rule 16.2: switch label enclosed by for loop */
            PROGRESS_VCPU(teardown);

            rc = vcpu_teardown(v);
            if ( rc )
                return rc;
        }

    PROGRESS(arch_teardown):
        rc = arch_domain_teardown(d);
        if ( rc )
            return rc;

    PROGRESS(done):
        break;

#undef PROGRESS_VCPU
#undef PROGRESS

    default:
        BUG();
    }

    return 0;
}

/*
 * Destroy a domain once all references to it have been dropped.  Used either
 * from the RCU path, or from the domain_create() error path before the domain
 * is inserted into the domlist.
 */
static void _domain_destroy(struct domain *d)
{
    BUG_ON(!d->is_dying);
    BUG_ON(atomic_read(&d->refcnt) != DOMAIN_DESTROYED);

    xfree(d->pbuf);

    argo_destroy(d);

    rangeset_domain_destroy(d);

    free_cpumask_var(d->dirty_cpumask);

    xsm_free_security_domain(d);

    lock_profile_deregister_struct(LOCKPROF_TYPE_PERDOM, d);

    free_domain_struct(d);
}

static int sanitise_domain_config(struct xen_domctl_createdomain *config)
{
    bool hvm = config->flags & XEN_DOMCTL_CDF_hvm;
    bool hap = config->flags & XEN_DOMCTL_CDF_hap;
    bool iommu = config->flags & XEN_DOMCTL_CDF_iommu;
    bool vpmu = config->flags & XEN_DOMCTL_CDF_vpmu;

    if ( config->flags &
         ~(XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap |
           XEN_DOMCTL_CDF_s3_integrity | XEN_DOMCTL_CDF_oos_off |
           XEN_DOMCTL_CDF_xs_domain | XEN_DOMCTL_CDF_iommu |
           XEN_DOMCTL_CDF_nested_virt | XEN_DOMCTL_CDF_vpmu) )
    {
        dprintk(XENLOG_INFO, "Unknown CDF flags %#x\n", config->flags);
        return -EINVAL;
    }

    if ( config->grant_opts & ~XEN_DOMCTL_GRANT_version_mask )
    {
        dprintk(XENLOG_INFO, "Unknown grant options %#x\n", config->grant_opts);
        return -EINVAL;
    }

    if ( config->max_vcpus < 1 )
    {
        dprintk(XENLOG_INFO, "No vCPUS\n");
        return -EINVAL;
    }

    if ( hap && !hvm )
    {
        dprintk(XENLOG_INFO, "HAP requested for non-HVM guest\n");
        return -EINVAL;
    }

    if ( iommu )
    {
        if ( config->iommu_opts & ~XEN_DOMCTL_IOMMU_no_sharept )
        {
            dprintk(XENLOG_INFO, "Unknown IOMMU options %#x\n",
                    config->iommu_opts);
            return -EINVAL;
        }

        if ( !iommu_enabled )
        {
            dprintk(XENLOG_INFO, "IOMMU requested but not available\n");
            return -EINVAL;
        }
    }
    else
    {
        if ( config->iommu_opts )
        {
            dprintk(XENLOG_INFO,
                    "IOMMU options specified but IOMMU not requested\n");
            return -EINVAL;
        }
    }

    if ( config->vmtrace_size && !vmtrace_available )
    {
        dprintk(XENLOG_INFO, "vmtrace requested but not available\n");
        return -EINVAL;
    }

    if ( vpmu && !vpmu_is_available )
    {
        dprintk(XENLOG_INFO, "vpmu requested but cannot be enabled this way\n");
        return -EINVAL;
    }

    return arch_sanitise_domain_config(config);
}

struct domain *domain_create(domid_t domid,
                             struct xen_domctl_createdomain *config,
                             unsigned int flags)
{
    struct domain *d, **pd, *old_hwdom = NULL;
    enum { INIT_watchdog = 1u<<1,
           INIT_evtchn = 1u<<3, INIT_gnttab = 1u<<4, INIT_arch = 1u<<5 };
    int err, init_status = 0;

    if ( config && (err = sanitise_domain_config(config)) )
        return ERR_PTR(err);

    if ( (d = alloc_domain_struct()) == NULL )
        return ERR_PTR(-ENOMEM);

    /* Sort out our idea of is_system_domain(). */
    d->domain_id = domid;

    /* Holding CDF_* internal flags. */
    d->cdf = flags;

    /* Debug sanity. */
    ASSERT(is_system_domain(d) ? config == NULL : config != NULL);

    if ( config )
    {
        d->options = config->flags;
        d->vmtrace_size = config->vmtrace_size;
    }

    /* Sort out our idea of is_control_domain(). */
    d->is_privileged = flags & CDF_privileged;

    /* Sort out our idea of is_hardware_domain(). */
    if ( domid == 0 || domid == hardware_domid )
    {
        if ( hardware_domid < 0 || hardware_domid >= DOMID_FIRST_RESERVED )
            panic("The value of hardware_dom must be a valid domain ID\n");

        old_hwdom = hardware_domain;
        hardware_domain = d;
    }

    TRACE_1D(TRC_DOM0_DOM_ADD, d->domain_id);

    lock_profile_register_struct(LOCKPROF_TYPE_PERDOM, d, domid);

    atomic_set(&d->refcnt, 1);
    RCU_READ_LOCK_INIT(&d->rcu_lock);
    rspin_lock_init_prof(d, domain_lock);
    rspin_lock_init_prof(d, page_alloc_lock);
    spin_lock_init(&d->hypercall_deadlock_mutex);
    INIT_PAGE_LIST_HEAD(&d->page_list);
    INIT_PAGE_LIST_HEAD(&d->extra_page_list);
    INIT_PAGE_LIST_HEAD(&d->xenpage_list);
#ifdef CONFIG_STATIC_MEMORY
    INIT_PAGE_LIST_HEAD(&d->resv_page_list);
#endif


    spin_lock_init(&d->node_affinity_lock);
    d->node_affinity = NODE_MASK_ALL;
    d->auto_node_affinity = 1;

    spin_lock_init(&d->shutdown_lock);
    d->shutdown_code = SHUTDOWN_CODE_INVALID;

    spin_lock_init(&d->pbuf_lock);

    rwlock_init(&d->vnuma_rwlock);

#ifdef CONFIG_HAS_PCI
    INIT_LIST_HEAD(&d->pdev_list);
    rwlock_init(&d->pci_lock);
#endif

    /* All error paths can depend on the above setup. */

    /*
     * Allocate d->vcpu[] and set ->max_vcpus up early.  Various per-domain
     * resources want to be sized based on max_vcpus.
     */
    if ( !is_system_domain(d) )
    {
        err = -ENOMEM;
        d->vcpu = xzalloc_array(struct vcpu *, config->max_vcpus);
        if ( !d->vcpu )
            goto fail;

        d->max_vcpus = config->max_vcpus;
    }

    if ( (err = xsm_alloc_security_domain(d)) != 0 )
        goto fail;

    err = -ENOMEM;
    if ( !zalloc_cpumask_var(&d->dirty_cpumask) )
        goto fail;

    rangeset_domain_initialise(d);

    /* DOMID_{XEN,IO,etc} (other than IDLE) are sufficiently constructed. */
    if ( is_system_domain(d) && !is_idle_domain(d) )
        return d;

#ifdef CONFIG_HAS_PIRQ
    if ( !is_idle_domain(d) )
    {
        if ( !is_hardware_domain(d) )
            d->nr_pirqs = nr_static_irqs + extra_domU_irqs;
        else
            d->nr_pirqs = extra_hwdom_irqs ? nr_static_irqs + extra_hwdom_irqs
                                           : arch_hwdom_irqs(domid);
        d->nr_pirqs = min(d->nr_pirqs, nr_irqs);

        radix_tree_init(&d->pirq_tree);
    }
#endif

    if ( (err = arch_domain_create(d, config, flags)) != 0 )
        goto fail;
    init_status |= INIT_arch;

    if ( !is_idle_domain(d) )
    {
        /*
         * The assertion helps static analysis tools infer that config cannot
         * be NULL in this branch, which in turn means that it can be safely
         * dereferenced. Therefore, this assertion is not redundant.
         */
        ASSERT(config);

        watchdog_domain_init(d);
        init_status |= INIT_watchdog;

        err = -ENOMEM;
        d->iomem_caps = rangeset_new(d, "I/O Memory", RANGESETF_prettyprint_hex);
        d->irq_caps   = rangeset_new(d, "Interrupts", 0);
        if ( !d->iomem_caps || !d->irq_caps )
            goto fail;

        if ( (err = xsm_domain_create(XSM_HOOK, d, config->ssidref)) != 0 )
            goto fail;

        d->controller_pause_count = 1;
        atomic_inc(&d->pause_count);

        if ( (err = evtchn_init(d, config->max_evtchn_port)) != 0 )
            goto fail;
        init_status |= INIT_evtchn;

        if ( (err = grant_table_init(d, config->max_grant_frames,
                                     config->max_maptrack_frames,
                                     config->grant_opts)) != 0 )
            goto fail;
        init_status |= INIT_gnttab;

        if ( (err = argo_init(d)) != 0 )
            goto fail;

        err = -ENOMEM;

        d->pbuf = xzalloc_array(char, DOMAIN_PBUF_SIZE);
        if ( !d->pbuf )
            goto fail;

        if ( (err = sched_init_domain(d, config->cpupool_id)) != 0 )
            goto fail;

        if ( (err = late_hwdom_init(d)) != 0 )
            goto fail;

        /*
         * Must not fail beyond this point, as our caller doesn't know whether
         * the domain has been entered into domain_list or not.
         */

        spin_lock(&domlist_update_lock);
        pd = &domain_list; /* NB. domain_list maintained in order of domid. */
        for ( pd = &domain_list; *pd != NULL; pd = &(*pd)->next_in_list )
            if ( (*pd)->domain_id > d->domain_id )
                break;
        d->next_in_list = *pd;
        d->next_in_hashbucket = domain_hash[DOMAIN_HASH(domid)];
        rcu_assign_pointer(*pd, d);
        rcu_assign_pointer(domain_hash[DOMAIN_HASH(domid)], d);
        spin_unlock(&domlist_update_lock);

        memcpy(d->handle, config->handle, sizeof(d->handle));
    }

    return d;

 fail:
    ASSERT(err < 0);      /* Sanity check paths leading here. */
    err = err ?: -EILSEQ; /* Release build safety. */

    d->is_dying = DOMDYING_dead;
    if ( hardware_domain == d )
        hardware_domain = old_hwdom;
    atomic_set(&d->refcnt, DOMAIN_DESTROYED);

    sched_destroy_domain(d);

    if ( d->max_vcpus )
    {
        d->max_vcpus = 0;
        XFREE(d->vcpu);
    }
    if ( init_status & INIT_arch )
        arch_domain_destroy(d);
    if ( init_status & INIT_gnttab )
        grant_table_destroy(d);
    if ( init_status & INIT_evtchn )
    {
        evtchn_destroy(d);
        evtchn_destroy_final(d);
#ifdef CONFIG_HAS_PIRQ
        radix_tree_destroy(&d->pirq_tree, free_pirq_struct);
#endif
    }
    if ( init_status & INIT_watchdog )
        watchdog_domain_destroy(d);

    /* Must not hit a continuation in this context. */
    if ( domain_teardown(d) )
        ASSERT_UNREACHABLE();

    _domain_destroy(d);

    return ERR_PTR(err);
}

void __init setup_system_domains(void)
{
    /*
     * Initialise our DOMID_XEN domain.
     * Any Xen-heap pages that we will allow to be mapped will have
     * their domain field set to dom_xen.
     * Hidden PCI devices will also be associated with this domain
     * (but be [partly] controlled by Dom0 nevertheless).
     */
    dom_xen = domain_create(DOMID_XEN, NULL, 0);
    if ( IS_ERR(dom_xen) )
        panic("Failed to create d[XEN]: %ld\n", PTR_ERR(dom_xen));

    /*
     * Initialise our DOMID_IO domain.
     * This domain owns I/O pages that are within the range of the page_info
     * array. Mappings occur at the priv of the caller.
     * Quarantined PCI devices will be associated with this domain.
     *
     * DOMID_IO is also the default owner of memory pre-shared among multiple
     * domains at boot time.
     */
    dom_io = domain_create(DOMID_IO, NULL, 0);
    if ( IS_ERR(dom_io) )
        panic("Failed to create d[IO]: %ld\n", PTR_ERR(dom_io));

#ifdef CONFIG_MEM_SHARING
    /*
     * Initialise our COW domain.
     * This domain owns sharable pages.
     */
    dom_cow = domain_create(DOMID_COW, NULL, 0);
    if ( IS_ERR(dom_cow) )
        panic("Failed to create d[COW]: %ld\n", PTR_ERR(dom_cow));
#endif
}

int domain_set_node_affinity(struct domain *d, const nodemask_t *affinity)
{
    /* Being disjoint with the system is just wrong. */
    if ( !nodes_intersects(*affinity, node_online_map) )
        return -EINVAL;

    spin_lock(&d->node_affinity_lock);

    /*
     * Being/becoming explicitly affine to all nodes is not particularly
     * useful. Let's take it as the `reset node affinity` command.
     */
    if ( nodes_full(*affinity) )
    {
        d->auto_node_affinity = 1;
        goto out;
    }

    d->auto_node_affinity = 0;
    d->node_affinity = *affinity;

out:
    spin_unlock(&d->node_affinity_lock);

    domain_update_node_affinity(d);

    return 0;
}

/* rcu_read_lock(&domlist_read_lock) must be held. */
static struct domain *domid_to_domain(domid_t dom)
{
    struct domain *d;

    for ( d = rcu_dereference(domain_hash[DOMAIN_HASH(dom)]);
          d != NULL;
          d = rcu_dereference(d->next_in_hashbucket) )
    {
        if ( d->domain_id == dom )
            return d;
    }

    return NULL;
}

struct domain *get_domain_by_id(domid_t dom)
{
    struct domain *d;

    rcu_read_lock(&domlist_read_lock);

    d = domid_to_domain(dom);
    if ( d && unlikely(!get_domain(d)) )
        d = NULL;

    rcu_read_unlock(&domlist_read_lock);

    return d;
}


struct domain *rcu_lock_domain_by_id(domid_t dom)
{
    struct domain *d;

    rcu_read_lock(&domlist_read_lock);

    d = domid_to_domain(dom);
    if ( d )
        rcu_lock_domain(d);

    rcu_read_unlock(&domlist_read_lock);

    return d;
}

struct domain *knownalive_domain_from_domid(domid_t dom)
{
    struct domain *d;

    rcu_read_lock(&domlist_read_lock);

    d = domid_to_domain(dom);

    rcu_read_unlock(&domlist_read_lock);

    return d;
}

struct domain *rcu_lock_domain_by_any_id(domid_t dom)
{
    if ( dom == DOMID_SELF )
        return rcu_lock_current_domain();
    return rcu_lock_domain_by_id(dom);
}

int rcu_lock_remote_domain_by_id(domid_t dom, struct domain **d)
{
    if ( (*d = rcu_lock_domain_by_id(dom)) == NULL )
        return -ESRCH;

    if ( *d == current->domain )
    {
        rcu_unlock_domain(*d);
        return -EPERM;
    }

    return 0;
}

int rcu_lock_live_remote_domain_by_id(domid_t dom, struct domain **d)
{
    int rv;
    rv = rcu_lock_remote_domain_by_id(dom, d);
    if ( rv )
        return rv;
    if ( (*d)->is_dying )
    {
        rcu_unlock_domain(*d);
        return -EINVAL;
    }

    return 0;
}

int domain_kill(struct domain *d)
{
    int rc = 0;
    struct vcpu *v;

    if ( d == current->domain )
        return -EINVAL;

    /* Protected by domctl_lock. */
    switch ( d->is_dying )
    {
    case DOMDYING_alive:
        domain_pause(d);
        d->is_dying = DOMDYING_dying;
        rspin_barrier(&d->domain_lock);
        argo_destroy(d);
        vnuma_destroy(d->vnuma);
        domain_set_outstanding_pages(d, 0);
        /* fallthrough */
    case DOMDYING_dying:
        rc = domain_teardown(d);
        if ( rc )
            break;
        rc = evtchn_destroy(d);
        if ( rc )
            break;
        rc = domain_relinquish_resources(d);
        if ( rc != 0 )
            break;
        if ( cpupool_move_domain(d, cpupool0) )
            return -ERESTART;
        for_each_vcpu ( d, v )
        {
            unmap_guest_area(v, &v->vcpu_info_area);
            unmap_guest_area(v, &v->runstate_guest_area);
        }
        d->is_dying = DOMDYING_dead;
        /* Mem event cleanup has to go here because the rings 
         * have to be put before we call put_domain. */
        vm_event_cleanup(d);
        put_domain(d);
        send_global_virq(VIRQ_DOM_EXC);
        /* fallthrough */
    case DOMDYING_dead:
        break;
    }

    return rc;
}


void __domain_crash(struct domain *d)
{
    if ( d->is_shutting_down )
    {
        /* Print nothing: the domain is already shutting down. */
    }
    else if ( d == current->domain )
    {
        printk("Domain %d (vcpu#%d) crashed on cpu#%d:\n",
               d->domain_id, current->vcpu_id, smp_processor_id());
        show_execution_state(guest_cpu_user_regs());
    }
    else
    {
        printk("Domain %d reported crashed by domain %d on cpu#%d:\n",
               d->domain_id, current->domain->domain_id, smp_processor_id());
    }

    domain_shutdown(d, SHUTDOWN_crash);
}


int domain_shutdown(struct domain *d, u8 reason)
{
    struct vcpu *v;

#ifdef CONFIG_X86
    if ( pv_shim )
        return pv_shim_shutdown(reason);
#endif

    spin_lock(&d->shutdown_lock);

    if ( d->shutdown_code == SHUTDOWN_CODE_INVALID )
        d->shutdown_code = reason;
    reason = d->shutdown_code;

    if ( is_hardware_domain(d) )
        hwdom_shutdown(reason);

    if ( d->is_shutting_down )
    {
        spin_unlock(&d->shutdown_lock);
        return 0;
    }

    d->is_shutting_down = 1;

    smp_mb(); /* set shutdown status /then/ check for per-cpu deferrals */

    for_each_vcpu ( d, v )
    {
        if ( reason == SHUTDOWN_crash )
            v->defer_shutdown = 0;
        else if ( v->defer_shutdown )
            continue;
        vcpu_pause_nosync(v);
        v->paused_for_shutdown = 1;
    }

    arch_domain_shutdown(d);

    __domain_finalise_shutdown(d);

    spin_unlock(&d->shutdown_lock);

    return 0;
}

void domain_resume(struct domain *d)
{
    struct vcpu *v;

    /*
     * Some code paths assume that shutdown status does not get reset under
     * their feet (e.g., some assertions make this assumption).
     */
    domain_pause(d);

    spin_lock(&d->shutdown_lock);

    d->is_shutting_down = d->is_shut_down = 0;
    d->shutdown_code = SHUTDOWN_CODE_INVALID;

    for_each_vcpu ( d, v )
    {
        if ( v->paused_for_shutdown )
            vcpu_unpause(v);
        v->paused_for_shutdown = 0;
    }

    spin_unlock(&d->shutdown_lock);

    domain_unpause(d);
}

int vcpu_start_shutdown_deferral(struct vcpu *v)
{
    if ( v->defer_shutdown )
        return 1;

    v->defer_shutdown = 1;
    smp_mb(); /* set deferral status /then/ check for shutdown */
    if ( unlikely(v->domain->is_shutting_down) )
        vcpu_check_shutdown(v);

    return v->defer_shutdown;
}

void vcpu_end_shutdown_deferral(struct vcpu *v)
{
    v->defer_shutdown = 0;
    smp_mb(); /* clear deferral status /then/ check for shutdown */
    if ( unlikely(v->domain->is_shutting_down) )
        vcpu_check_shutdown(v);
}

/* Complete domain destroy after RCU readers are not holding old references. */
static void complete_domain_destroy(struct rcu_head *head)
{
    struct domain *d = container_of(head, struct domain, rcu);
    struct vcpu *v;
    int i;

    /*
     * Flush all state for the vCPU previously having run on the current CPU.
     * This is in particular relevant for x86 HVM ones on VMX, so that this
     * flushing of state won't happen from the TLB flush IPI handler behind
     * the back of a vmx_vmcs_enter() / vmx_vmcs_exit() section.
     */
    sync_local_execstate();

    for ( i = d->max_vcpus - 1; i >= 0; i-- )
    {
        if ( (v = d->vcpu[i]) == NULL )
            continue;
        tasklet_kill(&v->continue_hypercall_tasklet);
        arch_vcpu_destroy(v);
        sched_destroy_vcpu(v);
        destroy_waitqueue_vcpu(v);
    }

    grant_table_destroy(d);

    arch_domain_destroy(d);

    watchdog_domain_destroy(d);

    sched_destroy_domain(d);

    /* Free page used by xen oprofile buffer. */
#ifdef CONFIG_XENOPROF
    free_xenoprof_pages(d);
#endif

#ifdef CONFIG_MEM_PAGING
    xfree(d->vm_event_paging);
#endif
    xfree(d->vm_event_monitor);
#ifdef CONFIG_MEM_SHARING
    xfree(d->vm_event_share);
#endif

    for ( i = d->max_vcpus - 1; i >= 0; i-- )
        if ( (v = d->vcpu[i]) != NULL )
            vcpu_destroy(v);

    if ( d->target != NULL )
        put_domain(d->target);

    evtchn_destroy_final(d);

#ifdef CONFIG_HAS_PIRQ
    radix_tree_destroy(&d->pirq_tree, free_pirq_struct);
#endif

    xfree(d->vcpu);

    _domain_destroy(d);

    send_global_virq(VIRQ_DOM_EXC);
}

/* Release resources belonging to task @p. */
void domain_destroy(struct domain *d)
{
    struct domain **pd;

    BUG_ON(!d->is_dying);

    /* May be already destroyed, or get_domain() can race us. */
    if ( atomic_cmpxchg(&d->refcnt, 0, DOMAIN_DESTROYED) != 0 )
        return;

    TRACE_1D(TRC_DOM0_DOM_REM, d->domain_id);

    /* Delete from task list and task hashtable. */
    spin_lock(&domlist_update_lock);
    pd = &domain_list;
    while ( *pd != d ) 
        pd = &(*pd)->next_in_list;
    rcu_assign_pointer(*pd, d->next_in_list);
    pd = &domain_hash[DOMAIN_HASH(d->domain_id)];
    while ( *pd != d ) 
        pd = &(*pd)->next_in_hashbucket;
    rcu_assign_pointer(*pd, d->next_in_hashbucket);
    spin_unlock(&domlist_update_lock);

    /* Schedule RCU asynchronous completion of domain destroy. */
    call_rcu(&d->rcu, complete_domain_destroy);
}

void vcpu_pause(struct vcpu *v)
{
    ASSERT(v != current);
    atomic_inc(&v->pause_count);
    vcpu_sleep_sync(v);
}

void vcpu_pause_nosync(struct vcpu *v)
{
    atomic_inc(&v->pause_count);
    vcpu_sleep_nosync(v);
}

void vcpu_unpause(struct vcpu *v)
{
    if ( atomic_dec_and_test(&v->pause_count) )
        vcpu_wake(v);
}

int vcpu_pause_by_systemcontroller(struct vcpu *v)
{
    int old, new, prev = v->controller_pause_count;

    do
    {
        old = prev;
        new = old + 1;

        if ( new > 255 )
            return -EOVERFLOW;

        prev = cmpxchg(&v->controller_pause_count, old, new);
    } while ( prev != old );

    vcpu_pause(v);

    return 0;
}

int vcpu_unpause_by_systemcontroller(struct vcpu *v)
{
    int old, new, prev = v->controller_pause_count;

    do
    {
        old = prev;
        new = old - 1;

        if ( new < 0 )
            return -EINVAL;

        prev = cmpxchg(&v->controller_pause_count, old, new);
    } while ( prev != old );

    vcpu_unpause(v);

    return 0;
}

static void _domain_pause(struct domain *d, bool sync)
{
    struct vcpu *v;

    atomic_inc(&d->pause_count);

    if ( sync )
        for_each_vcpu ( d, v )
            vcpu_sleep_sync(v);
    else
        for_each_vcpu ( d, v )
            vcpu_sleep_nosync(v);

    arch_domain_pause(d);
}

void domain_pause(struct domain *d)
{
    ASSERT(d != current->domain);
    _domain_pause(d, true /* sync */);
}

void domain_pause_nosync(struct domain *d)
{
    _domain_pause(d, false /* nosync */);
}

void domain_unpause(struct domain *d)
{
    struct vcpu *v;

    arch_domain_unpause(d);

    if ( atomic_dec_and_test(&d->pause_count) )
        for_each_vcpu( d, v )
            vcpu_wake(v);
}

static int _domain_pause_by_systemcontroller(struct domain *d, bool sync)
{
    int old, new, prev = d->controller_pause_count;

    do
    {
        old = prev;
        new = old + 1;

        /*
         * Limit the toolstack pause count to an arbitrary 255 to prevent the
         * toolstack overflowing d->pause_count with many repeated hypercalls.
         */
        if ( new > 255 )
            return -EOVERFLOW;

        prev = cmpxchg(&d->controller_pause_count, old, new);
    } while ( prev != old );

    _domain_pause(d, sync);

    return 0;
}

int domain_pause_by_systemcontroller(struct domain *d)
{
    return _domain_pause_by_systemcontroller(d, true /* sync */);
}

int domain_pause_by_systemcontroller_nosync(struct domain *d)
{
    return _domain_pause_by_systemcontroller(d, false /* nosync */);
}

int domain_unpause_by_systemcontroller(struct domain *d)
{
    int old, new, prev = d->controller_pause_count;

    do
    {
        old = prev;
        new = old - 1;

        if ( new < 0 )
            return -EINVAL;

        prev = cmpxchg(&d->controller_pause_count, old, new);
    } while ( prev != old );

    /*
     * d->controller_pause_count is initialised to 1, and the toolstack is
     * responsible for making one unpause hypercall when it wishes the guest
     * to start running.
     *
     * All other toolstack operations should make a pair of pause/unpause
     * calls and rely on the reference counting here.
     *
     * Creation is considered finished when the controller reference count
     * first drops to 0.
     */
    if ( new == 0 && !d->creation_finished )
    {
        d->creation_finished = true;
        arch_domain_creation_finished(d);
    }

    domain_unpause(d);

    return 0;
}

int domain_pause_except_self(struct domain *d)
{
    struct vcpu *v, *curr = current;

    if ( curr->domain == d )
    {
        /* Avoid racing with other vcpus which may want to be pausing us */
        if ( !spin_trylock(&d->hypercall_deadlock_mutex) )
            return -ERESTART;
        for_each_vcpu( d, v )
            if ( likely(v != curr) )
                vcpu_pause(v);
        spin_unlock(&d->hypercall_deadlock_mutex);
    }
    else
        domain_pause(d);

    return 0;
}

void domain_unpause_except_self(struct domain *d)
{
    struct vcpu *v, *curr = current;

    if ( curr->domain == d )
    {
        for_each_vcpu( d, v )
            if ( likely(v != curr) )
                vcpu_unpause(v);
    }
    else
        domain_unpause(d);
}

int domain_soft_reset(struct domain *d, bool resuming)
{
    struct vcpu *v;
    int rc;

    spin_lock(&d->shutdown_lock);
    for_each_vcpu ( d, v )
        if ( !v->paused_for_shutdown )
        {
            spin_unlock(&d->shutdown_lock);
            return -EINVAL;
        }
    spin_unlock(&d->shutdown_lock);

    rc = evtchn_reset(d, resuming);
    if ( rc )
        return rc;

    grant_table_warn_active_grants(d);

    argo_soft_reset(d);

    for_each_vcpu ( d, v )
    {
        set_xen_guest_handle(runstate_guest(v), NULL);
        unmap_guest_area(v, &v->vcpu_info_area);
        unmap_guest_area(v, &v->runstate_guest_area);
    }

    rc = arch_domain_soft_reset(d);
    if ( !rc )
        domain_resume(d);
    else
        domain_crash(d);

    return rc;
}

int vcpu_reset(struct vcpu *v)
{
    struct domain *d = v->domain;
    int rc;

    vcpu_pause(v);
    domain_lock(d);

    set_bit(_VPF_in_reset, &v->pause_flags);
    rc = arch_vcpu_reset(v);
    if ( rc )
        goto out_unlock;

    set_bit(_VPF_down, &v->pause_flags);

    clear_bit(v->vcpu_id, d->poll_mask);
    v->poll_evtchn = 0;

    v->fpu_initialised = 0;
    v->fpu_dirtied     = 0;
    v->is_initialised  = 0;
    if ( v->affinity_broken & VCPU_AFFINITY_OVERRIDE )
        vcpu_temporary_affinity(v, NR_CPUS, VCPU_AFFINITY_OVERRIDE);
    if ( v->affinity_broken & VCPU_AFFINITY_WAIT )
        vcpu_temporary_affinity(v, NR_CPUS, VCPU_AFFINITY_WAIT);
    clear_bit(_VPF_blocked, &v->pause_flags);
    clear_bit(_VPF_in_reset, &v->pause_flags);

 out_unlock:
    domain_unlock(v->domain);
    vcpu_unpause(v);

    return rc;
}

int map_guest_area(struct vcpu *v, paddr_t gaddr, unsigned int size,
                   struct guest_area *area,
                   void (*populate)(void *dst, struct vcpu *v))
{
    struct domain *d = v->domain;
    void *map = NULL;
    struct page_info *pg = NULL;
    int rc = 0;

    if ( ~gaddr ) /* Map (i.e. not just unmap)? */
    {
        unsigned long gfn = PFN_DOWN(gaddr);
        unsigned int align;
        p2m_type_t p2mt;

        if ( gfn != PFN_DOWN(gaddr + size - 1) )
            return -ENXIO;

#ifdef CONFIG_COMPAT
        if ( has_32bit_shinfo(d) )
            align = alignof(compat_ulong_t);
        else
#endif
            align = alignof(xen_ulong_t);
        if ( !IS_ALIGNED(gaddr, align) )
            return -ENXIO;

        rc = check_get_page_from_gfn(d, _gfn(gfn), false, &p2mt, &pg);
        if ( rc )
            return rc;

        if ( !get_page_type(pg, PGT_writable_page) )
        {
            put_page(pg);
            return -EACCES;
        }

        map = __map_domain_page_global(pg);
        if ( !map )
        {
            put_page_and_type(pg);
            return -ENOMEM;
        }
        map += PAGE_OFFSET(gaddr);
    }

    if ( v != current )
    {
        if ( !spin_trylock(&d->hypercall_deadlock_mutex) )
        {
            rc = -ERESTART;
            goto unmap;
        }

        vcpu_pause(v);

        spin_unlock(&d->hypercall_deadlock_mutex);
    }

    domain_lock(d);

    /* No re-registration of the vCPU info area. */
    if ( area != &v->vcpu_info_area || !area->pg )
    {
        if ( map && populate )
            populate(map, v);

        SWAP(area->pg, pg);
        SWAP(area->map, map);
    }
    else
        rc = -EBUSY;

    domain_unlock(d);

    /* Set pending flags /after/ new vcpu_info pointer was set. */
    if ( area == &v->vcpu_info_area && !rc )
    {
        /*
         * Mark everything as being pending just to make sure nothing gets
         * lost.  The domain will get a spurious event, but it can cope.
         */
#ifdef CONFIG_COMPAT
        if ( !has_32bit_shinfo(d) )
        {
            vcpu_info_t *info = area->map;

            /* For VCPUOP_register_vcpu_info handling in common_vcpu_op(). */
            BUILD_BUG_ON(sizeof(*info) != sizeof(info->compat));
            write_atomic(&info->native.evtchn_pending_sel, ~0);
        }
        else
#endif
            write_atomic(&vcpu_info(v, evtchn_pending_sel), ~0);
        vcpu_mark_events_pending(v);

        force_update_vcpu_system_time(v);
    }

    if ( v != current )
        vcpu_unpause(v);

 unmap:
    if ( pg )
    {
        unmap_domain_page_global((void *)((unsigned long)map & PAGE_MASK));
        put_page_and_type(pg);
    }

    return rc;
}

/*
 * This is only intended to be used for domain cleanup (or more generally only
 * with at least the respective vCPU, if it's not the current one, reliably
 * paused).
 */
void unmap_guest_area(struct vcpu *v, struct guest_area *area)
{
    struct domain *d = v->domain;
    void *map;
    struct page_info *pg;

    if ( v != current )
        ASSERT(atomic_read(&v->pause_count) | atomic_read(&d->pause_count));

    domain_lock(d);
    map = area->map;
    if ( area == &v->vcpu_info_area )
        vcpu_info_reset(v);
    else
        area->map = NULL;
    pg = area->pg;
    area->pg = NULL;
    domain_unlock(d);

    if ( pg )
    {
        unmap_domain_page_global((void *)((unsigned long)map & PAGE_MASK));
        put_page_and_type(pg);
    }
}

int default_initialise_vcpu(struct vcpu *v, XEN_GUEST_HANDLE_PARAM(void) arg)
{
    struct vcpu_guest_context *ctxt;
    struct domain *d = v->domain;
    int rc;

    if ( (ctxt = alloc_vcpu_guest_context()) == NULL )
        return -ENOMEM;

    if ( copy_from_guest(ctxt, arg, 1) )
    {
        free_vcpu_guest_context(ctxt);
        return -EFAULT;
    }

    domain_lock(d);
    rc = v->is_initialised ? -EEXIST : arch_set_info_guest(v, ctxt);
    domain_unlock(d);

    free_vcpu_guest_context(ctxt);

    return rc;
}

/* Update per-VCPU guest runstate shared memory area (if registered). */
bool update_runstate_area(struct vcpu *v)
{
    bool rc;
    struct guest_memory_policy policy = { };
    void __user *guest_handle = NULL;
    struct vcpu_runstate_info runstate = v->runstate;
    struct vcpu_runstate_info *map = v->runstate_guest_area.map;

    if ( map )
    {
        uint64_t *pset;
#ifdef CONFIG_COMPAT
        struct compat_vcpu_runstate_info *cmap = NULL;

        if ( v->runstate_guest_area_compat )
            cmap = (void *)map;
#endif

        /*
         * NB: No VM_ASSIST(v->domain, runstate_update_flag) check here.
         *     Always using that updating model.
         */
#ifdef CONFIG_COMPAT
        if ( cmap )
            pset = &cmap->state_entry_time;
        else
#endif
            pset = &map->state_entry_time;
        runstate.state_entry_time |= XEN_RUNSTATE_UPDATE;
        write_atomic(pset, runstate.state_entry_time);
        smp_wmb();

#ifdef CONFIG_COMPAT
        if ( cmap )
            XLAT_vcpu_runstate_info(cmap, &runstate);
        else
#endif
            *map = runstate;

        smp_wmb();
        runstate.state_entry_time &= ~XEN_RUNSTATE_UPDATE;
        write_atomic(pset, runstate.state_entry_time);

        return true;
    }

    if ( guest_handle_is_null(runstate_guest(v)) )
        return true;

    update_guest_memory_policy(v, &policy);

    if ( VM_ASSIST(v->domain, runstate_update_flag) )
    {
#ifdef CONFIG_COMPAT
        guest_handle = has_32bit_shinfo(v->domain)
            ? &v->runstate_guest.compat.p->state_entry_time + 1
            : &v->runstate_guest.native.p->state_entry_time + 1;
#else
        guest_handle = &v->runstate_guest.p->state_entry_time + 1;
#endif
        guest_handle--;
        runstate.state_entry_time |= XEN_RUNSTATE_UPDATE;
        __raw_copy_to_guest(guest_handle,
                            (void *)(&runstate.state_entry_time + 1) - 1, 1);
        smp_wmb();
    }

#ifdef CONFIG_COMPAT
    if ( has_32bit_shinfo(v->domain) )
    {
        struct compat_vcpu_runstate_info info;

        XLAT_vcpu_runstate_info(&info, &runstate);
        __copy_to_guest(v->runstate_guest.compat, &info, 1);
        rc = true;
    }
    else
#endif
        rc = __copy_to_guest(runstate_guest(v), &runstate, 1) !=
             sizeof(runstate);

    if ( guest_handle )
    {
        runstate.state_entry_time &= ~XEN_RUNSTATE_UPDATE;
        smp_wmb();
        __raw_copy_to_guest(guest_handle,
                            (void *)(&runstate.state_entry_time + 1) - 1, 1);
    }

    update_guest_memory_policy(v, &policy);

    return rc;
}

/*
 * This makes sure that the vcpu_info is always pointing at a valid piece of
 * memory, and it sets a pending event to make sure that a pending event
 * doesn't get missed.
 */
static void cf_check
vcpu_info_populate(void *map, struct vcpu *v)
{
    vcpu_info_t *info = map;

    if ( v->vcpu_info_area.map == &dummy_vcpu_info )
    {
        memset(info, 0, sizeof(*info));
#ifdef XEN_HAVE_PV_UPCALL_MASK
        __vcpu_info(v, info, evtchn_upcall_mask) = 1;
#endif
    }
    else
        memcpy(info, v->vcpu_info_area.map, sizeof(*info));
}

static void cf_check
runstate_area_populate(void *map, struct vcpu *v)
{
#ifdef CONFIG_PV
    if ( is_pv_vcpu(v) )
        v->arch.pv.need_update_runstate_area = false;
#endif

#ifdef CONFIG_COMPAT
    v->runstate_guest_area_compat = false;
#endif

    if ( v == current )
    {
        struct vcpu_runstate_info *info = map;

        *info = v->runstate;
    }
}

long common_vcpu_op(int cmd, struct vcpu *v, XEN_GUEST_HANDLE_PARAM(void) arg)
{
    long rc = 0;
    struct domain *d = v->domain;
    unsigned int vcpuid = v->vcpu_id;

    switch ( cmd )
    {
    case VCPUOP_initialise:
        if ( is_pv_domain(d) && v->vcpu_info_area.map == &dummy_vcpu_info )
            return -EINVAL;

        rc = arch_initialise_vcpu(v, arg);
        if ( rc == -ERESTART )
            rc = hypercall_create_continuation(__HYPERVISOR_vcpu_op, "iih",
                                               cmd, vcpuid, arg);

        break;

    case VCPUOP_up:
#ifdef CONFIG_X86
        if ( pv_shim )
            rc = continue_hypercall_on_cpu(0, pv_shim_cpu_up, v);
        else
#endif
        {
            bool wake = false;

            domain_lock(d);
            if ( !v->is_initialised )
                rc = -EINVAL;
            else
                wake = test_and_clear_bit(_VPF_down, &v->pause_flags);
            domain_unlock(d);
            if ( wake )
                vcpu_wake(v);
        }

        break;

    case VCPUOP_down:
        for_each_vcpu ( d, v )
            if ( v->vcpu_id != vcpuid && !test_bit(_VPF_down, &v->pause_flags) )
            {
               rc = 1;
               break;
            }

        if ( !rc ) /* Last vcpu going down? */
        {
            domain_shutdown(d, SHUTDOWN_poweroff);
            break;
        }

        rc = 0;
        v = d->vcpu[vcpuid];

#ifdef CONFIG_X86
        if ( pv_shim )
            rc = continue_hypercall_on_cpu(0, pv_shim_cpu_down, v);
        else
#endif
            if ( !test_and_set_bit(_VPF_down, &v->pause_flags) )
                vcpu_sleep_nosync(v);

        break;

    case VCPUOP_is_up:
        rc = !(v->pause_flags & VPF_down);
        break;

    case VCPUOP_get_runstate_info:
    {
        struct vcpu_runstate_info runstate;
        vcpu_runstate_get(v, &runstate);
        if ( copy_to_guest(arg, &runstate, 1) )
            rc = -EFAULT;
        break;
    }

    case VCPUOP_set_periodic_timer:
    {
        struct vcpu_set_periodic_timer set;

        if ( copy_from_guest(&set, arg, 1) )
            return -EFAULT;

        if ( set.period_ns < MILLISECS(1) )
            return -EINVAL;

        if ( set.period_ns > STIME_DELTA_MAX )
            return -EINVAL;

        vcpu_set_periodic_timer(v, set.period_ns);

        break;
    }

    case VCPUOP_stop_periodic_timer:
        vcpu_set_periodic_timer(v, 0);
        break;

    case VCPUOP_set_singleshot_timer:
    {
        struct vcpu_set_singleshot_timer set;

        if ( v != current )
            return -EINVAL;

        if ( copy_from_guest(&set, arg, 1) )
            return -EFAULT;

        if ( set.timeout_abs_ns < NOW() )
        {
            /*
             * Simplify the logic if the timeout has already expired and just
             * inject the event.
             */
            stop_timer(&v->singleshot_timer);
            send_timer_event(v);
            break;
        }

        migrate_timer(&v->singleshot_timer, smp_processor_id());
        set_timer(&v->singleshot_timer, set.timeout_abs_ns);

        break;
    }

    case VCPUOP_stop_singleshot_timer:
        if ( v != current )
            return -EINVAL;

        stop_timer(&v->singleshot_timer);

        break;

    case VCPUOP_register_vcpu_info:
    {
        struct vcpu_register_vcpu_info info;
        paddr_t gaddr;

        rc = -EFAULT;
        if ( copy_from_guest(&info, arg, 1) )
            break;

        rc = -EINVAL;
        gaddr = gfn_to_gaddr(_gfn(info.mfn)) + info.offset;
        if ( !~gaddr ||
             gfn_x(gaddr_to_gfn(gaddr)) != info.mfn )
            break;

        /* Preliminary check only; see map_guest_area(). */
        rc = -EBUSY;
        if ( v->vcpu_info_area.pg )
            break;

        /* See the BUILD_BUG_ON() in vcpu_info_populate(). */
        rc = map_guest_area(v, gaddr, sizeof(vcpu_info_t),
                            &v->vcpu_info_area, vcpu_info_populate);
        if ( rc == -ERESTART )
            rc = hypercall_create_continuation(__HYPERVISOR_vcpu_op, "iih",
                                               cmd, vcpuid, arg);

        break;
    }

    case VCPUOP_register_runstate_memory_area:
    {
        struct vcpu_register_runstate_memory_area area;
        struct vcpu_runstate_info runstate;

        rc = -EFAULT;
        if ( copy_from_guest(&area, arg, 1) )
            break;

        if ( !guest_handle_okay(area.addr.h, 1) )
            break;

        rc = 0;
        runstate_guest(v) = area.addr.h;

        if ( v == current )
        {
            __copy_to_guest(runstate_guest(v), &v->runstate, 1);
        }
        else
        {
            vcpu_runstate_get(v, &runstate);
            __copy_to_guest(runstate_guest(v), &runstate, 1);
        }

        break;
    }

    case VCPUOP_register_runstate_phys_area:
    {
        struct vcpu_register_runstate_memory_area area;

        rc = -ENOSYS;
        if ( 0 /* TODO: Dom's XENFEAT_runstate_phys_area setting */ )
            break;

        rc = -EFAULT;
        if ( copy_from_guest(&area.addr.p, arg, 1) )
            break;

        rc = map_guest_area(v, area.addr.p,
                            sizeof(struct vcpu_runstate_info),
                            &v->runstate_guest_area,
                            runstate_area_populate);
        if ( rc == -ERESTART )
            rc = hypercall_create_continuation(__HYPERVISOR_vcpu_op, "iih",
                                               cmd, vcpuid, arg);

        break;
    }

    default:
        rc = -ENOSYS;
        break;
    }

    return rc;
}

#ifdef arch_vm_assist_valid_mask
long do_vm_assist(unsigned int cmd, unsigned int type)
{
    struct domain *currd = current->domain;
    const unsigned long valid = arch_vm_assist_valid_mask(currd);

    if ( type >= BITS_PER_LONG || !test_bit(type, &valid) )
        return -EINVAL;

    switch ( cmd )
    {
    case VMASST_CMD_enable:
        set_bit(type, &currd->vm_assist);
        return 0;

    case VMASST_CMD_disable:
        clear_bit(type, &currd->vm_assist);
        return 0;
    }

    return -ENOSYS;
}
#endif

#ifdef CONFIG_HAS_PIRQ

struct pirq *pirq_get_info(struct domain *d, int pirq)
{
    struct pirq *info = pirq_info(d, pirq);

    if ( !info && (info = alloc_pirq_struct(d)) != NULL )
    {
        info->pirq = pirq;
        if ( radix_tree_insert(&d->pirq_tree, pirq, info) )
        {
            free_pirq_struct(info);
            info = NULL;
        }
    }

    return info;
}

static void _free_pirq_struct(struct rcu_head *head)
{
    xfree(container_of(head, struct pirq, rcu_head));
}

void free_pirq_struct(void *ptr)
{
    struct pirq *pirq = ptr;

    call_rcu(&pirq->rcu_head, _free_pirq_struct);
}

#endif /* CONFIG_HAS_PIRQ */

struct migrate_info {
    long (*func)(void *data);
    void *data;
    struct vcpu *vcpu;
    unsigned int cpu;
    unsigned int nest;
};

static DEFINE_PER_CPU(struct migrate_info *, continue_info);

static void continue_hypercall_tasklet_handler(void *data)
{
    struct migrate_info *info = data;
    struct vcpu *v = info->vcpu;
    long res = -EINVAL;

    /* Wait for vcpu to sleep so that we can access its register state. */
    vcpu_sleep_sync(v);

    this_cpu(continue_info) = info;

    if ( likely(info->cpu == smp_processor_id()) )
        res = info->func(info->data);

    arch_hypercall_tasklet_result(v, res);

    this_cpu(continue_info) = NULL;

    if ( info->nest-- == 0 )
    {
        xfree(info);
        vcpu_unpause(v);
        put_domain(v->domain);
    }
}

int continue_hypercall_on_cpu(
    unsigned int cpu, long (*func)(void *data), void *data)
{
    struct migrate_info *info;

    if ( (cpu >= nr_cpu_ids) || !cpu_online(cpu) )
        return -EINVAL;

    info = this_cpu(continue_info);
    if ( info == NULL )
    {
        struct vcpu *curr = current;

        info = xmalloc(struct migrate_info);
        if ( info == NULL )
            return -ENOMEM;

        info->vcpu = curr;
        info->nest = 0;

        tasklet_kill(&curr->continue_hypercall_tasklet);
        tasklet_init(&curr->continue_hypercall_tasklet,
                     continue_hypercall_tasklet_handler, info);

        get_knownalive_domain(curr->domain);
        vcpu_pause_nosync(curr);
    }
    else
    {
        BUG_ON(info->nest != 0);
        info->nest++;
    }

    info->func = func;
    info->data = data;
    info->cpu  = cpu;

    tasklet_schedule_on_cpu(&info->vcpu->continue_hypercall_tasklet, cpu);

    /* Dummy return value will be overwritten by tasklet. */
    return 0;
}
#endif

// --------------------------------------------------------------
