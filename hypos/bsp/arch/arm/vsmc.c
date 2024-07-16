/**
 * Hustler's Project
 *
 * File:  vsmc.c
 * Date:  2024/07/12
 * Usage:
 */

#include <bsp/config.h>

#if IS_IMPLEMENTED(__VSMC_IMPL)
// --------------------------------------------------------------

/* Number of functions currently supported by Hypervisor Service.
 */
#define SMCCC_FUNCTION_COUNT 3

/* Number of functions currently supported by Standard Service
 * Service Calls.
 */
#define SSSC_SMCCC_FUNCTION_COUNT \
    (3 + VPSCI_NR_FUNCS + FFA_NR_FUNCS)

static bool fill_uid(struct hcpu_regs *regs, xen_uuid_t uuid)
{
    int n;

    for (n = 0; n < 4; n++) {
        const u8 *bytes = uuid.a + n * 4;
        u32 r;

        r = bytes[0];
        r |= bytes[1] << 8;
        r |= bytes[2] << 16;
        r |= bytes[3] << 24;

        set_user_reg(regs, n, r);
    }

    return true;
}

static bool fill_revision(struct hcpu_regs *regs, u32 major,
                         u32 minor)
{
    set_user_reg(regs, 0, major);
    set_user_reg(regs, 1, minor);

    return true;
}

static bool fill_function_call_count(struct hcpu_regs *regs, u32 cnt)
{
    set_user_reg(regs, 0, cnt);

    return true;
}

static bool handle_arch(struct hcpu_regs *regs)
{
    u32 fid = (u32)get_user_reg(regs, 0);

    switch (fid) {
    case ARM_SMCCC_VERSION_FID:
        set_user_reg(regs, 0, ARM_SMCCC_VERSION_1_2);
        return true;

    case ARM_SMCCC_ARCH_FEATURES_FID:
    {
        u32 arch_func_id = get_user_reg(regs, 1);
        int ret = ARM_SMCCC_NOT_SUPPORTED;

        switch (arch_func_id) {
        case ARM_SMCCC_ARCH_WORKAROUND_1_FID:
            if (cpus_have_cap(ARM_HARDEN_BRANCH_PREDICTOR) ||
                cpus_have_cap(ARM_WORKAROUND_BHB_SMCC_3))
                ret = ARM_SMCCC_SUCCESS;
            break;
        case ARM_SMCCC_ARCH_WORKAROUND_2_FID:
            switch (get_ssbd_state()) {
            case ARM_SSBD_UNKNOWN:
            case ARM_SSBD_FORCE_DISABLE:
                break;
            case ARM_SSBD_RUNTIME:
                ret = ARM_SMCCC_SUCCESS;
                break;
            case ARM_SSBD_FORCE_ENABLE:
            case ARM_SSBD_MITIGATED:
                ret = ARM_SMCCC_NOT_REQUIRED;
                break;
            }
            break;
        case ARM_SMCCC_ARCH_WORKAROUND_3_FID:
            if (cpus_have_cap(ARM_WORKAROUND_BHB_SMCC_3))
                ret = ARM_SMCCC_SUCCESS;
            break;
        }

        set_user_reg(regs, 0, ret);

        return true;
    }
    case ARM_SMCCC_ARCH_WORKAROUND_1_FID:
    case ARM_SMCCC_ARCH_WORKAROUND_3_FID:
        /* No return value */
        return true;
    case ARM_SMCCC_ARCH_WORKAROUND_2_FID:
    {
        bool enable = (u32)get_user_reg(regs, 1);

        if (unlikely(get_ssbd_state() != ARM_SSBD_RUNTIME))
            return true;
        if (enable)
            get_cpu_info()->flags |= CPUINFO_WORKAROUND_2_FLAG;
        else
            get_cpu_info()->flags &= ~CPUINFO_WORKAROUND_2_FLAG;

        return true;
    }
    }

    return false;
}

/* SMCCC interface for hypervisor. Tell about itself. */
static bool handle_hypervisor(struct hcpu_regs *regs)
{
    u32 fid = (u32)get_user_reg(regs, 0);

    switch (fid) {
    case ARM_SMCCC_CALL_COUNT_FID(HYPERVISOR):
        return fill_function_call_count(regs,
                                        SMCCC_FUNCTION_COUNT);
    case ARM_SMCCC_CALL_UID_FID(HYPERVISOR):
        return fill_uid(regs, SMCCC_UID);
    case ARM_SMCCC_REVISION_FID(HYPERVISOR):
        return fill_revision(regs, SMCCC_MAJOR_REVISION,
                             SMCCC_MINOR_REVISION);
    default:
        return false;
    }
}

static bool handle_existing_apis(struct hcpu_regs *regs)
{
    u32 fid = (u32)get_user_reg(regs, 0);

    return do_vpsci_0_1_call(regs, fid);
}

static bool is_psci_fid(u32 fid)
{
    u32 fn = fid & ARM_SMCCC_FUNC_MASK;

    return fn >= PSCI_FNUM_MIN_VALUE && fn <= PSCI_FNUM_MAX_VALUE;
}

static bool handle_sssc(struct hcpu_regs *regs)
{
    u32 fid = (u32)get_user_reg(regs, 0);

    if (is_psci_fid(fid))
        return do_vpsci_0_2_call(regs, fid);

    if (is_ffa_fid(fid))
        return tee_handle_call(regs);

    switch (fid) {
    case ARM_SMCCC_CALL_COUNT_FID(STANDARD):
        return fill_function_call_count(regs,
                                        SSSC_SMCCC_FUNCTION_COUNT);
    case ARM_SMCCC_CALL_UID_FID(STANDARD):
        return fill_uid(regs, SSSC_SMCCC_UID);

    case ARM_SMCCC_REVISION_FID(STANDARD):
        return fill_revision(regs, SSSC_SMCCC_MAJOR_REVISION,
                             SSSC_SMCCC_MINOR_REVISION);
    default:
        return false;
    }
}

static bool vsmccc_handle_call(struct hcpu_regs *regs)
{
    bool handled = false;
    const union hcpu_esr hsr = { .bits = regs->hsr };
    u32 funcid = get_user_reg(regs, 0);

    switch (hsr.ec) {
    case HSR_EC_HVC32:
    case HSR_EC_HVC64:
    case HSR_EC_SMC64:
        if ((hsr.iss & HSR_XXC_IMM_MASK) != 0)
            return false;
        break;
    case HSR_EC_SMC32:
        break;
    default:
        return false;
    }

    if (smccc_is_conv_64(funcid) && is_32bit_hypos(current->hypos)) {
        set_user_reg(regs, 0, ARM_SMCCC_ERR_UNKNOWN_FUNCTION);
        return true;
    }

    if (funcid >= ARM_SMCCC_RESERVED_RANGE_START &&
        funcid <= ARM_SMCCC_RESERVED_RANGE_END)
        handled = handle_existing_apis(regs);
    else {
        switch (smccc_get_owner(funcid)) {
        case ARM_SMCCC_OWNER_ARCH:
            handled = handle_arch(regs);
            break;
        case ARM_SMCCC_OWNER_HYPERVISOR:
            handled = handle_hypervisor(regs);
            break;
        case ARM_SMCCC_OWNER_STANDARD:
            handled = handle_sssc(regs);
            break;
        case ARM_SMCCC_OWNER_SIP:
            handled = platform_smc(regs);
            break;
        case ARM_SMCCC_OWNER_TRUSTED_APP ... ARM_SMCCC_OWNER_TRUSTED_APP_END:
        case ARM_SMCCC_OWNER_TRUSTED_OS ... ARM_SMCCC_OWNER_TRUSTED_OS_END:
            handled = tee_handle_call(regs);
            break;
        }
    }

    if (!handled) {
        MSGE("Unhandled SMC/HVC: %#x\n", funcid);

        set_user_reg(regs, 0, ARM_SMCCC_ERR_UNKNOWN_FUNCTION);
    }

    return true;
}

void do_trap_smc(struct hcpu_regs *regs, const union hcpu_esr hsr)
{
    int rc = 0;

    if (!check_conditional_instr(regs, hsr)) {
        advance_pc(regs, hsr);
        return;
    }

    if (current->hypos->arch.monitor.privileged_call_enabled)
        rc = monitor_smc();

    if (rc == 1)
        return;

    if (vsmccc_handle_call(regs))
        advance_pc(regs, hsr);
    else
        inject_undef_exception(regs, hsr);
}

void do_trap_hvc_smccc(struct hcpu_regs *regs)
{
    const union hcpu_esr hsr = { .bits = regs->hsr };

    if (!vsmccc_handle_call(regs))
        inject_undef_exception(regs, hsr);
}

// --------------------------------------------------------------
#endif
