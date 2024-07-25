/**
 * Hustler's Project
 *
 * File:  cpu.c
 * Date:  2024/07/15
 * Usage:
 */

#include <org/section.h>
#include <org/cpu.h>
#include <org/smp.h>
#include <asm/sysregs.h>
#include <asm/sve.h>
#include <bsp/config.h>
#include <bsp/panic.h>
#include <bsp/debug.h>
#include <bsp/type.h>
#include <bsp/bootcore.h>

// --------------------------------------------------------------
DECLARE_BITMAP(cpu_hwcaps, ARM_NCAPS);

struct arm_cpu __read_mostly core_cpu;

struct arm_cpu __read_mostly hypos_cpuinfo;

static bool
has_sb_instruction(const struct arm_cpu_capabilities *entry)
{
    return core_cpu.isa64.sb;
}

static const struct arm_cpu_capabilities arm_features[] = {
    {
        .desc = "Speculation barrier instruction (SB)",
        .capability = ARM_HAS_SB,
        .matches = has_sb_instruction,
    },

    {},
};

void update_cpu_capabilities(const struct arm_cpu_capabilities *caps,
                             const char *info)
{
    int i;

    for (i = 0; caps[i].matches; i++) {
        if (!caps[i].matches(&caps[i]))
            continue;

        if (!cpus_have_cap(caps[i].capability) && caps[i].desc)
            MSGH("%s: %s\n", info, caps[i].desc);
        cpus_set_cap(caps[i].capability);
    }
}

void __bootfunc
enable_cpu_capabilities(const struct arm_cpu_capabilities *caps)
{
    for ( ; caps->matches; caps++) {
        if (!cpus_have_cap(caps->capability))
            continue;

        if (caps->enable) {
            int ret;

            ret = halt_run(caps->enable, (void *)caps, NR_CPUS);
            BUG_ON(ret);
        }
    }
}

void check_local_cpu_features(void)
{
    update_cpu_capabilities(arm_features, "enabled support for");
}

void __bootfunc enable_cpu_features(void)
{
    enable_cpu_capabilities(arm_features);
}

int enable_nonboot_cpu_caps(const struct arm_cpu_capabilities *caps)
{
    int rc = 0;

    for ( ; caps->matches; caps++) {
        if (!cpus_have_cap(caps->capability))
            continue;

        if (caps->enable) {
            int ret = caps->enable((void *)caps);
            if (ret)
                rc = ret;
        }
    }

    return rc;
}

void identify_cpu(struct arm_cpu *c)
{
    bool aarch32_el0 = true;
    /* MIDR_EL1 Main ID Register
     * Provides identification infos for the PE, including
     * an implementer code for the device and a device ID
     * number.
     */
    c->midr.bits     = READ_SYSREG(MIDR_EL1);
    /* MPIDR_EL1
     */
    c->mpidr.bits    = READ_SYSREG(MPIDR_EL1);
    /* ID_PFRx_EL1 AArch64 Processor Feature Registers
     */
    c->pfr64.bits[0] = READ_SYSREG(ID_AA64PFR0_EL1);
    c->pfr64.bits[1] = READ_SYSREG(ID_AA64PFR1_EL1);
    /* ID_AA64DFRx_EL1 AArch64 Debug Feature Registers
     */
    c->dbg64.bits[0] = READ_SYSREG(ID_AA64DFR0_EL1);
    c->dbg64.bits[1] = READ_SYSREG(ID_AA64DFR1_EL1);
    /* ID_AA64AFRx_EL1 AArch64 Auxiliary Feature Registers
     */
    c->aux64.bits[0] = READ_SYSREG(ID_AA64AFR0_EL1);
    c->aux64.bits[1] = READ_SYSREG(ID_AA64AFR1_EL1);
    /* ID_AA64MMFRx_EL1 AArch64 Memory Model Features Registers
     */
    c->mm64.bits[0]  = READ_SYSREG(ID_AA64MMFR0_EL1);
    c->mm64.bits[1]  = READ_SYSREG(ID_AA64MMFR1_EL1);
    c->mm64.bits[2]  = READ_SYSREG(ID_AA64MMFR2_EL1);
    /* ID_AA64ISARx_EL1 AArch64 Instruction Set Attribute Registers
     */
    c->isa64.bits[0] = READ_SYSREG(ID_AA64ISAR0_EL1);
    c->isa64.bits[1] = READ_SYSREG(ID_AA64ISAR1_EL1);
    c->isa64.bits[2] = READ_SYSREG(ID_AA64ISAR2_EL1);
#if IS_SUPPORTED(SUP__ID_AA64ZFR0_EL1)
    /* ID_AA64ZFR0_EL1 SVE Feature Register.
     * Provides additional infos about the implemented features
     * of the AArch64 Scalable Vector Extension, when the
     * ID_AA64ZFR0_EL1.SVE field is not zero.
     */
    c->zfr64.bits[0] = READ_SYSREG(ID_AA64ZFR0_EL1);
#endif
    if (cpu_has_sve)
        c->zcr64.bits[0] = compute_max_zcr();

    c->dczid.bits[0] = READ_SYSREG(DCZID_EL0);

    c->ctr.bits[0] = READ_SYSREG(CTR_EL0);

    aarch32_el0 = cpu_feature64_has_el0_32(c);

    if (aarch32_el0) {
        c->pfr32.bits[0] = READ_SYSREG(ID_PFR0_EL1);
        c->pfr32.bits[1] = READ_SYSREG(ID_PFR1_EL1);
#if IS_SUPPORTED(SUP__ID_PFR2_EL1)
        c->pfr32.bits[2] = READ_SYSREG(ID_PFR2_EL1);
#endif
        c->dbg32.bits[0] = READ_SYSREG(ID_DFR0_EL1);
        c->dbg32.bits[1] = READ_SYSREG(ID_DFR1_EL1);
        c->aux32.bits[0] = READ_SYSREG(ID_AFR0_EL1);

        c->mm32.bits[0]  = READ_SYSREG(ID_MMFR0_EL1);
        c->mm32.bits[1]  = READ_SYSREG(ID_MMFR1_EL1);
        c->mm32.bits[2]  = READ_SYSREG(ID_MMFR2_EL1);
        c->mm32.bits[3]  = READ_SYSREG(ID_MMFR3_EL1);
        c->mm32.bits[4]  = READ_SYSREG(ID_MMFR4_EL1);
        c->mm32.bits[5]  = READ_SYSREG(ID_MMFR5_EL1);

        c->isa32.bits[0] = READ_SYSREG(ID_ISAR0_EL1);
        c->isa32.bits[1] = READ_SYSREG(ID_ISAR1_EL1);
        c->isa32.bits[2] = READ_SYSREG(ID_ISAR2_EL1);
        c->isa32.bits[3] = READ_SYSREG(ID_ISAR3_EL1);
        c->isa32.bits[4] = READ_SYSREG(ID_ISAR4_EL1);
        c->isa32.bits[5] = READ_SYSREG(ID_ISAR5_EL1);
        c->isa32.bits[6] = READ_SYSREG(ID_ISAR6_EL1);

        c->mvfr.bits[0]  = READ_SYSREG(MVFR0_EL1);
        c->mvfr.bits[1]  = READ_SYSREG(MVFR1_EL1);
        c->mvfr.bits[2]  = READ_SYSREG(MVFR2_EL1);
    }
}

static int __bootfunc create_hypos_cpuinfo(void)
{
    hypos_cpuinfo = core_cpu;

    hypos_cpuinfo.pfr64.mpam = 0;
    hypos_cpuinfo.pfr64.mpam_frac = 0;

    hypos_cpuinfo.pfr64.sve = 0;
    hypos_cpuinfo.zfr64.bits[0] = 0;

    hypos_cpuinfo.pfr64.mte = 0;

    hypos_cpuinfo.isa64.apa = 0;
    hypos_cpuinfo.isa64.api = 0;
    hypos_cpuinfo.isa64.gpa = 0;
    hypos_cpuinfo.isa64.gpi = 0;

    hypos_cpuinfo.pfr64.amu = 0;
    hypos_cpuinfo.pfr32.amu = 0;

    hypos_cpuinfo.pfr64.ras = 0;
    hypos_cpuinfo.pfr64.ras_frac = 0;
    hypos_cpuinfo.pfr32.ras = 0;
    hypos_cpuinfo.pfr32.ras_frac = 0;

    return 0;
}
__bootcall(create_hypos_cpuinfo);

// --------------------------------------------------------------

/* XXX: Get to know more about processors.
 */
int __bootfunc processor_setup(void)
{
    const char *implementer = "Unknown";
    struct arm_cpu *c = &core_cpu;

    identify_cpu(c);
    current_cpu_data = *c;

    MSGI(BLANK_ALIGN"----------------------------------------------------------\n");

    if (c->midr.architecture != 0xF)
        MSGE("Huh, cpu architecture %x, expected 0xf (defined by cpuid)\n",
             c->midr.architecture);

    MSGI("[cores] Processor: %016lx, variant: 0x%x, part 0x%03x, "
         "rev 0x%x\n", c->midr.bits, c->midr.variant,
         c->midr.part_number, c->midr.revision);

    MSGI(BLANK_ALIGN"64-bit Execution @_@\n");
    MSGI(BLANK_ALIGN"Processor Features:      %016lx %016lx\n",
         core_cpu.pfr64.bits[0], core_cpu.pfr64.bits[1]);
    MSGI(BLANK_ALIGN"Exception Levels: EL3:%s EL2:%s EL1:%s EL0:%s\n",
         cpu_has_el3_32 ? "64+32" : cpu_has_el3_64 ? "64" : "No",
         cpu_has_el2_32 ? "64+32" : cpu_has_el2_64 ? "64" : "No",
         cpu_has_el1_32 ? "64+32" : cpu_has_el1_64 ? "64" : "No",
         cpu_has_el0_32 ? "64+32" : cpu_has_el0_64 ? "64" : "No");
    MSGI(BLANK_ALIGN"Extensions:%s%s%s%s\n",
         cpu_has_fp ? " Floating-Point" : "",
         cpu_has_simd ? " Advanced-SIMD" : "",
         cpu_has_gicv3 ? " GICv3-SysReg" : "",
         cpu_has_sve ? " SVE" : "");

    if (cpu_has_fp && (boot_cpu_feature64(fp) >= 2))
        MSGE("Unknown Floating-point ID:%d, "
             "this may result in corruption on the platform\n",
             boot_cpu_feature64(fp));

    if (cpu_has_simd && (boot_cpu_feature64(simd) >= 2))
        MSGE("Unknown Advanced-SIMD ID:%d, "
             "this may result in corruption on the platform\n",
             boot_cpu_feature64(simd));

    MSGI(BLANK_ALIGN"Debug Features:          %016lx %016lx\n",
         core_cpu.dbg64.bits[0], core_cpu.dbg64.bits[1]);
    MSGI(BLANK_ALIGN"Auxiliary Features:      %016lx %016lx\n",
         core_cpu.aux64.bits[0], core_cpu.aux64.bits[1]);
    MSGI(BLANK_ALIGN"Memory Model Features:   %016lx %016lx\n",
         core_cpu.mm64.bits[0], core_cpu.mm64.bits[1]);
    MSGI(BLANK_ALIGN"ISA Features:            %016lx %016lx\n",
         core_cpu.isa64.bits[0], core_cpu.isa64.bits[1]);

    if (cpu_has_aarch32) {
        MSGI(BLANK_ALIGN"32-bit Execution @_@\n");
        MSGI(BLANK_ALIGN"Processor Features:      %016lx %016lx\n",
               core_cpu.pfr32.bits[0], core_cpu.pfr32.bits[1]);
        MSGI(BLANK_ALIGN"Instruction Sets:%s%s%s%s%s%s\n",
             cpu_has_aarch32 ? " AArch32" : "",
             cpu_has_arm ? " A32" : "",
             cpu_has_thumb ? " Thumb" : "",
             cpu_has_thumb2 ? " Thumb-2" : "",
             cpu_has_thumbee ? " ThumbEE" : "",
             cpu_has_jazelle ? " Jazelle" : "");
        MSGI(BLANK_ALIGN"Extensions:%s%s\n",
             cpu_has_gentimer ? " GenericTimer" : "",
             cpu_has_security ? " Security" : "");

        MSGI(BLANK_ALIGN"Debug Features:          %016lx\n",
             core_cpu.dbg32.bits[0]);
        MSGI(BLANK_ALIGN"Auxiliary Features:      %016lx\n",
             core_cpu.aux32.bits[0]);
        MSGI(BLANK_ALIGN"Memory Model Features:   %016lx %016lx\n"
             BLANK_ALIGN"                         %016lx %016lx\n",
             core_cpu.mm32.bits[0], core_cpu.mm32.bits[1],
             core_cpu.mm32.bits[2], core_cpu.mm32.bits[3]);
        MSGI(BLANK_ALIGN"ISA Features:            %016lx %016lx\n"
             BLANK_ALIGN"                         %016lx %016lx\n"
             BLANK_ALIGN"                         %016lx %016lx\n",
             core_cpu.isa32.bits[0], core_cpu.isa32.bits[1],
             core_cpu.isa32.bits[2], core_cpu.isa32.bits[3],
             core_cpu.isa32.bits[4], core_cpu.isa32.bits[5]);
    } else {
        MSGE("32-bit Execution: Unsupported\n");
    }

    MSGI(BLANK_ALIGN"----------------------------------------------------------\n");

    return 0;
}

// --------------------------------------------------------------
