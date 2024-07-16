/**
 * Hustler's Project
 *
 * File:  psci.c
 * Date:  2024/07/12
 * Usage: Power State Coordination Interface
 */

#include <org/section.h>
#include <org/psci.h>
#include <org/smp.h>
#include <asm/at.h>
#include <bsp/errno.h>
#include <bsp/config.h>
#include <bsp/percpu.h>
#include <bsp/debug.h>
#include <bsp/type.h>
#include <bsp/cpu.h>

#if IS_IMPLEMENTED(__PSCI_IMPL)
// --------------------------------------------------------------
extern void smpboot_cpu(void);
extern register_t cpu_logical_map[NR_CPUS];

#define PSCI_0_2_FN_NATIVE(name)    PSCI_0_2_FN64_##name

u32 psci_ver;
u32 smccc_ver;

static u32 psci_cpu_on_nr;

#define PSCI_RET(res)   ((s32)(res).a0)

int call_psci_cpu_on(int cpu)
{
    struct arm_smccc_res res;

    arm_smccc_smc(psci_cpu_on_nr, cpu_logical_map(cpu),
                  __pa(smpboot_cpu),
                  &res);

    return PSCI_RET(res);
}

void call_psci_cpu_off(void)
{
    if (psci_ver > PSCI_VERSION(0, 1)) {
        struct arm_smccc_res res;

        /* If successfull the PSCI cpu_off call doesn't return */
        arm_smccc_smc(PSCI_0_2_FN32_CPU_OFF, &res);
        exec_panic("PSCI cpu off failed for CPU%d err=%d\n",
                   smp_processor_id(),
                   PSCI_RET(res));
    }
}

void call_psci_system_off(void)
{
    if (psci_ver > PSCI_VERSION(0, 1))
        arm_smccc_smc(PSCI_0_2_FN32_SYSTEM_OFF, NULL);
}

void call_psci_system_reset(void)
{
    if (psci_ver > PSCI_VERSION(0, 1))
        arm_smccc_smc(PSCI_0_2_FN32_SYSTEM_RESET, NULL);
}

static int __bootfunc psci_features(u32 psci_func_id)
{
    struct arm_smccc_res res;

    if (psci_ver < PSCI_VERSION(1, 0))
        return PSCI_NOT_SUPPORTED;

    arm_smccc_smc(PSCI_1_0_FN32_PSCI_FEATURES, psci_func_id, &res);

    return PSCI_RET(res);
}

static void __bootfunc psci_setup_smccc(void)
{
    /* PSCI is using at least SMCCC 1.0 calling convention. */
    smccc_ver = ARM_SMCCC_VERSION_1_0;

    if (psci_features(ARM_SMCCC_VERSION_FID) != PSCI_NOT_SUPPORTED) {
        struct arm_smccc_res res;

        arm_smccc_smc(ARM_SMCCC_VERSION_FID, &res);
        if (PSCI_RET(res) != ARM_SMCCC_NOT_SUPPORTED)
            smccc_ver = PSCI_RET(res);
    }

    if (smccc_ver >= SMCCC_VERSION(1, 1))
        cpus_set_cap(ARM_SMCCC_1_1);

    MSGH("Using SMC Calling Convention v%u.%u\n",
         SMCCC_VERSION_MAJOR(smccc_ver),
         SMCCC_VERSION_MINOR(smccc_ver));
}

static int __bootfunc psci_setup_0_1(void)
{
    int ret;

    psci_ver = PSCI_VERSION(0, 1);

    return 0;
}

static int __bootfunc psci_setup_0_2(void)
{
    int ret;
    struct arm_smccc_res res;

    arm_smccc_smc(PSCI_0_2_FN32_PSCI_VERSION, &res);
    psci_ver = PSCI_RET(res);

    /* For the moment, we only support PSCI 0.2 and PSCI 1.x */
    if (psci_ver != PSCI_VERSION(0, 2) &&
        PSCI_VERSION_MAJOR(psci_ver) != 1) {
        MSGH("Error: Unrecognized PSCI version %u.%u\n",
             PSCI_VERSION_MAJOR(psci_ver),
             PSCI_VERSION_MINOR(psci_ver));
        return -ENOTSUP;
    }

    psci_cpu_on_nr = PSCI_0_2_FN_NATIVE(CPU_ON);

    return 0;
}

int __bootfunc psci_setup(void)
{
    int ret;

    ret = psci_setup_0_2();
    if (ret)
        ret = psci_setup_0_1();

    if (ret)
        return ret;

    psci_setup_smccc();

    MSGH("Using PSCI v%u.%u\n",
         PSCI_VERSION_MAJOR(psci_ver),
         PSCI_VERSION_MINOR(psci_ver));

    return 0;
}
// --------------------------------------------------------------
#endif
