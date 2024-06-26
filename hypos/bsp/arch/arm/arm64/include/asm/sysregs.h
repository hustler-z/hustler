/**
 * Hustler's Project
 *
 * File:  sysregs.h
 * Date:  2024/05/20
 * Usage: ARMv8-A system register related defines
 */

#ifndef _ARCH_SYSREG_H
#define _ARCH_SYSREG_H
// --------------------------------------------------------------
#include <asm-generic/bitops.h>

/* MPIDR_EL1 - Multiprocessor Affinity Register
 *
 * In a multiprocessor system, provides an additional PE identification
 * mechanism for scheduling purposes.
 */
#define _MPIDR_UP                 (30)
#define MPIDR_UP                  (_AC(1, UL) << _MPIDR_UP)
#define _MPIDR_SMP                (31)
#define MPIDR_SMP                 (_AC(1, UL) << _MPIDR_SMP)
#define MPIDR_AFF0_SHIFT          (0)
#define MPIDR_AFF0_MASK           (_AC(0xFF, UL) << MPIDR_AFF0_SHIFT)
#define MPIDR_AFF1_SHIFT          (8)
#define MPIDR_AFF1_MASK           (_AC(0xFF, UL) << MPIDR_AFF1_SHIFT)
#define MPIDR_AFF2_SHIFT          (16)
#define MPIDR_AFF2_MASK           (_AC(0xFF, UL) << MPIDR_AFF2_SHIFT)
#define MPIDR_AFF3_SHIFT          (32)
#define MPIDR_AFF3_MASK           (_AC(0xFF, UL) << MPIDR_AFF3_SHIFT)
#define MPIDR_HWID_MASK           _AC(0xFF00FFFFFF, UL)
#define MPIDR_INVALID             (~MPIDR_HWID_MASK)
#define MPIDR_LEVEL_BITS          (8)
#define MPIDR_LEVEL_BITS_SHIFT    (3)
#define MPIDR_LEVEL_MASK          ((1 << MPIDR_LEVEL_BITS) - 1)
#define MPIDR_MT_SHIFT            (24)
#define MPIDR_MT_MASK             BIT(MPIDR_MT_SHIFT, UL)

#define MPIDR_LEVEL_SHIFT(level) \
        (((1 << (level)) >> 1) << MPIDR_LEVEL_BITS_SHIFT)

#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
        (((mpidr) >> MPIDR_LEVEL_SHIFT(level)) & MPIDR_LEVEL_MASK)

#define AFFINITY_MASK(level) \
        ~((_AC(0x1, UL) << MPIDR_LEVEL_SHIFT(level)) - 1)

/* SCTLR_EL2 - System Control Register
 *
 * Provides top level control of the system, including its
 * memory system, at EL2.
 *
 * M bit[0]  - MMU enable
 * A bit[1]  - Alignment fault checking
 * C bit[2]  - Data access cacheability
 * SA bit[3] - SP alignment check enable
 * I bit[12] - Instruction access cacheability
 */
#define SCTLR_A32_EL1_V           BIT(13, UL)
#define SCTLR_A32_ELx_TE          BIT(30, UL)
#define SCTLR_A32_ELx_FI          BIT(21, UL)
#define SCTLR_A64_ELx_SA          BIT(3,  UL)
#define SCTLR_Axx_ELx_EE          BIT(25, UL)
#define SCTLR_Axx_ELx_WXN         BIT(19, UL)
#define SCTLR_Axx_ELx_I           BIT(12, UL)
#define SCTLR_Axx_ELx_C           BIT(2,  UL)
#define SCTLR_Axx_ELx_A           BIT(1,  UL)
#define SCTLR_Axx_ELx_M           BIT(0,  UL)
#define SCTLR_EL2_RES1            (BIT( 4, UL) | BIT( 5, UL) | BIT(11, UL) |\
                                   BIT(16, UL) | BIT(18, UL) | BIT(22, UL) |\
                                   BIT(23, UL) | BIT(28, UL) | BIT(29, UL))

#define SCTLR_EL2_RES0            (BIT( 6, UL) | BIT( 7, UL) | BIT( 8, UL) |\
                                   BIT( 9, UL) | BIT(10, UL) | BIT(13, UL) |\
                                   BIT(14, UL) | BIT(15, UL) | BIT(17, UL) |\
                                   BIT(20, UL) | BIT(21, UL) | BIT(24, UL) |\
                                   BIT(26, UL) | BIT(27, UL) | BIT(30, UL) |\
                                   BIT(31, UL) | (0xFFFFFFFFULL << 32))
#define SCTLR_EL2_SET              SCTLR_EL2_RES1
#define SCTLR_EL2_CLEAR           (SCTLR_EL2_RES0    | SCTLR_Axx_ELx_M   |\
                                   SCTLR_Axx_ELx_A   | SCTLR_Axx_ELx_C   |\
                                   SCTLR_Axx_ELx_WXN | SCTLR_Axx_ELx_EE)

/* MAIR_EL2
 */
#define MT_DEVICE_nGnRnE          0x0
#define MT_NORMAL_NC              0x1
#define MT_NORMAL_WT              0x2
#define MT_NORMAL_WB              0x3
#define MT_DEVICE_nGnRE           0x4
#define MT_NORMAL                 0x7

#define _MAIR0(attr, mt)          (_AC(attr, ULL) << ((mt) * 8))
#define _MAIR1(attr, mt)          (_AC(attr, ULL) << (((mt) * 8) - 32))

#define MAIR0VAL                  (_MAIR0(0x00, MT_DEVICE_nGnRnE)| \
                                   _MAIR0(0x44, MT_NORMAL_NC)    | \
                                   _MAIR0(0xAA, MT_NORMAL_WT)    | \
                                   _MAIR0(0xEE, MT_NORMAL_WB))
#define MAIR1VAL                  (_MAIR1(0x04, MT_DEVICE_nGnRE) | \
                                   _MAIR1(0xFF, MT_NORMAL))
#define MAIR_EL2_SET              (MAIR1VAL << 32 | MAIR0VAL)

#define MDCR_TDRA       (_AC(1,U) << 11)   /* Trap Debug ROM access */
#define MDCR_TDOSA      (_AC(1,U) << 10)   /* Trap Debug-OS-related register access */
#define MDCR_TDA        (_AC(1,U) << 9)    /* Trap Debug Access */
#define MDCR_TDE        (_AC(1,U) << 8)    /* Route Soft Debug exceptions from EL1/EL1 to EL2 */
#define MDCR_TPM        (_AC(1,U) << 6)    /* Trap Performance Monitors accesses */
#define MDCR_TPMCR      (_AC(1,U) << 5)    /* Trap PMCR accesses */
#define MDCR_EL2_SET    (MDCR_TDRA | MDCR_TDOSA | MDCR_TDA | MDCR_TPM | MDCR_TPMCR)

#define CPTR_TAM       ((_AC(1,U) << 30))
#define CPTR_TTA       ((_AC(1,U) << 20))  /* Trap trace registers */
#define CPTR_CP(x)     ((_AC(1,U) << (x))) /* Trap Coprocessor x */
#define CPTR_CP_MASK   ((_AC(1,U) << 14) - 1)
#define CPTR_EL2_SET   ((CPTR_CP_MASK & ~(CPTR_CP(10) | CPTR_CP(11))) |\
                         CPTR_TTA | CPTR_TAM)

#define HSTR(x)        ((_AC(1, U) << (x)))

/* TCR_EL2 - Translation Control Register
 *
 * The control register for stage 1 of the EL2, or EL2&0, translation regime:
 * (a) When the Effective value of HCR_EL2.E2H is 0, this register controls
 * stage 1 of the EL2 translation regime, that supports a single VA range,
 * translated using TTBR0_EL2.
 * (b) When the value of HCR_EL2.E2H is 1, this register controls stage 1 of
 * the EL2&0 translation regime, that supports both:
 *     (1) A lower VA range, translated using TTBR0_EL2.
 *     (2) A higher VA range, translated using TTBR1_EL2.
 */
#define TCR_T0SZ_SHIFT            (0)
#define TCR_T1SZ_SHIFT            (16)
#define TCR_T0SZ(x)               ((x) << TCR_T0SZ_SHIFT)
#define TCR_SZ_MASK               (_AC(0x3F, UL))

#define TCR_EPD0                  (_AC(0x1, UL) << 7)
#define TCR_EPD1                  (_AC(0x1, UL) << 23)

#define TCR_IRGN0_NC              (_AC(0x0, UL) << 8)
#define TCR_IRGN0_WBWA            (_AC(0x1, UL) << 8)
#define TCR_IRGN0_WT              (_AC(0x2, UL) << 8)
#define TCR_IRGN0_WB              (_AC(0x3, UL) << 8)

#define TCR_ORGN0_NC              (_AC(0x0, UL) << 10)
#define TCR_ORGN0_WBWA            (_AC(0x1, UL) << 10)
#define TCR_ORGN0_WT              (_AC(0x2, UL) << 10)
#define TCR_ORGN0_WB              (_AC(0x3, UL) << 10)

#define TCR_SH0_NS                (_AC(0x0, UL) << 12)
#define TCR_SH0_OS                (_AC(0x2, UL) << 12)
#define TCR_SH0_IS                (_AC(0x3, UL) << 12)

#define TCR_TG0_SHIFT             (14)
#define TCR_TG0_MASK              (_AC(0x3, UL) << TCR_TG0_SHIFT)
#define TCR_TG0_4K                (_AC(0x0, UL) << TCR_TG0_SHIFT)
#define TCR_TG0_64K               (_AC(0x1, UL) << TCR_TG0_SHIFT)
#define TCR_TG0_16K               (_AC(0x2, UL) << TCR_TG0_SHIFT)

#define TCR_EL1_TG1_SHIFT         (30)
#define TCR_EL1_TG1_MASK          (_AC(0x3, UL) << TCR_EL1_TG1_SHIFT)
#define TCR_EL1_TG1_16K           (_AC(0x1, UL) << TCR_EL1_TG1_SHIFT)
#define TCR_EL1_TG1_4K            (_AC(0x2, UL) << TCR_EL1_TG1_SHIFT)
#define TCR_EL1_TG1_64K           (_AC(0x3, UL) << TCR_EL1_TG1_SHIFT)

#define TCR_EL1_IPS_SHIFT         (32)
#define TCR_EL1_IPS_MASK          (_AC(0x7, ULL) << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_32_BIT        (_AC(0x0, ULL) << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_36_BIT        (_AC(0x1, ULL) << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_40_BIT        (_AC(0x2, ULL) << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_42_BIT        (_AC(0x3, ULL) << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_44_BIT        (_AC(0x4, ULL) << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_48_BIT        (_AC(0x5, ULL) << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_52_BIT        (_AC(0x6, ULL) << TCR_EL1_IPS_SHIFT)

#define TCR_EL1_IPS_32_BIT_VAL    (32)
#define TCR_EL1_IPS_36_BIT_VAL    (36)
#define TCR_EL1_IPS_40_BIT_VAL    (40)
#define TCR_EL1_IPS_42_BIT_VAL    (42)
#define TCR_EL1_IPS_44_BIT_VAL    (44)
#define TCR_EL1_IPS_48_BIT_VAL    (48)
#define TCR_EL1_IPS_52_BIT_VAL    (52)
#define TCR_EL1_IPS_MIN_VAL       (25)
#define TCR_EL1_TBI0              (_AC(0x1, ULL) << 37)
#define TCR_EL1_TBI1              (_AC(0x1, ULL) << 38)
#define TCR_PS(x)                 ((x) << 16)
#define TCR_TBI                   (_AC(0x1, UL) << 20)

/* -----------------------------------------------------------
 * When the AF bit is being used, the translation tables are
 * created with the AF bit initially clear. When a page is
 * accessed, its AF bit is set. Software can parse the tables
 * to check whether the AF bits are set or clear. A page with
 * AF==0 cannot have been accessed and is potentially a better
 * candidate for being paged-out.
 *
 * Software Update: Accessing the page causes a synchronous
 * exception (Access Flag fault). In the exception handler,
 * software is responsible for setting the AF bit in the
 * relevant translation table entry and returns.
 *
 * Hardware Update: Accessing the page causes hardware to
 * automatically set the AF bit without needing to generate
 * an exception. This behavior needs to be enabled using the
 * hardware access update bit of the Translation Control
 * Register (TCR_ELx.HA) that was added in Armv8.1-A.
 *
 * bit[21] - FEAT_HAFDBS is implemented: Hardware access flag
 * update in stage 1 translation from EL2.
 * -----------------------------------------------------------
 * 0b0                     Stage 1 Access flag update disabled
 * 0b1                     Stage 1 Access flag update enabled
 * -----------------------------------------------------------
 */
#define TCR_HA_EN                 (_AC(1, UL) << 21)
#define TCR_RES1                  (_AC(1, UL) << 31 | _AC(1, UL) << 23)
#define TCR_EL2_SET               (TCR_RES1 | TCR_SH0_IS | TCR_ORGN0_WBWA |\
                                   TCR_HA_EN | TCR_IRGN0_WBWA | TCR_T0SZ(64-48))

/* VTCR_EL2 - Virtualization Translation Control Register
 *
 * The control register for stage 2 of the EL1&0 translation regime.
 */
#define VTCR_T0SZ(x)              ((x) << 0)
/* Starting level of the stage 2 translation lookup, meaning
 * of this field depends on the value of VTCR_EL2.TG0
 *
 * here start at level 1
 */
#define VTCR_SL0(x)               ((x) << 6)
#define VTCR_IRGN0_NC             (_AC(0x0, UL) << 8)
#define VTCR_IRGN0_WBWA           (_AC(0x1, UL) << 8)
#define VTCR_IRGN0_WT             (_AC(0x2, UL) << 8)
#define VTCR_IRGN0_WB             (_AC(0x3, UL) << 8)
#define VTCR_ORGN0_NC             (_AC(0x0, UL) << 10)
#define VTCR_ORGN0_WBWA           (_AC(0x1, UL) << 10)
#define VTCR_ORGN0_WT             (_AC(0x2, UL) << 10)
#define VTCR_ORGN0_WB             (_AC(0x3, UL) << 10)
#define VTCR_SH0_NS               (_AC(0x0, UL) << 12)
#define VTCR_SH0_OS               (_AC(0x2, UL) << 12)
#define VTCR_SH0_IS               (_AC(0x3, UL) << 12)
/* Granule size for the VTTBR_EL2
 */
#define VTCR_TG0_4K               (_AC(0x0, UL) << 14)
#define VTCR_TG0_64K              (_AC(0x1, UL) << 14)
#define VTCR_TG0_16K              (_AC(0x2, UL) << 14)
/* Physical address size for the 2nd stage of translation
 * [18:16]
 *         0b000          32 bits, 4GB
 *         0b001          36 bits, 64GB
 *         0b010          40 bits, 1TB
 *         0b011          42 bits, 4TB
 *         0b100          44 bits, 16TB
 *         0b101          48 bits, 256YB
 *         0b110          52 bits, 4PB
 */
#define VTCR_PS(x)                ((x) << 16)
#define VTCR_VS    	              (_AC(0x1, UL) << 19)
#define VTCR_RES1                 (_AC(1, UL) << 31)
#define VTCR_EL2_SET              (VTCR_SL0(1) | VTCR_T0SZ(64-48) | VTCR_TG0_4K |\
                                   VTCR_ORGN0_WBWA | VTCR_IRGN0_WBWA | VTCR_PS(0x2))

/* HCR_EL2 - Hypervisor Configuration Register
 *
 * Provides configuration controls for virtualization, including defining whether
 * various operations are trapped to EL2.
 */

/* Register Width, ARM64 only */
#define HCR_RW                    (_AC(1, UL) << 31)
/* Trap General Exceptions */
#define HCR_TGE                   (_AC(1, UL) << 27)
/* Trap Virtual Memory Controls */
#define HCR_TVM                   (_AC(1, UL) << 26)
/* Trap TLB Maintenance Operations */
#define HCR_TTLB                  (_AC(1, UL) << 25)
/* Trap Cache Maintenance Operations to PoU */
#define HCR_TPU                   (_AC(1, UL) << 24)
/* Trap Cache Maintenance Operations to PoC */
#define HCR_TPC                   (_AC(1, UL) << 23)
/* Trap Set/Way Cache Maintenance Operations */
#define HCR_TSW                   (_AC(1, UL) << 22)
/* Trap ACTLR Accesses */
#define HCR_TAC                   (_AC(1, UL) << 21)
/* Trap lockdown */
#define HCR_TIDCP                 (_AC(1, UL) << 20)
/* Trap SMC instruction */
#define HCR_TSC                   (_AC(1, UL) << 19)
/* Trap ID Register Group 3 */
#define HCR_TID3                  (_AC(1, UL) << 18)
/* Trap ID Register Group 2 */
#define HCR_TID2                  (_AC(1, UL) << 17)
/* Trap ID Register Group 1 */
#define HCR_TID1                  (_AC(1, UL) << 16)
/* Trap ID Register Group 0 */
#define HCR_TID0                  (_AC(1, UL) << 15)
/* Trap WFE instruction */
#define HCR_TWE                   (_AC(1, UL) << 14)
/* Trap WFI instruction */
#define HCR_TWI                   (_AC(1, UL) << 13)
/* Default cacheable */
#define HCR_DC                    (_AC(1, UL) << 12)
/* Barrier Shareability Upgrade */
#define HCR_BSU_MASK              (_AC(3, UL) << 10)
#define HCR_BSU_NONE              (_AC(0, UL) << 10)
#define HCR_BSU_INNER             (_AC(1, UL) << 10)
#define HCR_BSU_OUTER             (_AC(2, UL) << 10)
#define HCR_BSU_FULL              (_AC(3, UL) << 10)
/* Force Broadcast of Cache/BP/TLB operations */
#define HCR_FB                    (_AC(1, UL) << 9)
/* Virtual Asynchronous Abort */
#define HCR_VA                    (_AC(1, UL) << 8)
/* Virtual IRQ */
#define HCR_VI                    (_AC(1, UL) << 7)
/* Virtual FIQ */
#define HCR_VF                    (_AC(1, UL) << 6)
/* Physical SError interrupt Routing
 * When executing at any Exception level, and EL2 is
 * enabled in the current Security state:
 * (a) Physical SError interrupts are taken to EL2, unless
 * they are routed to EL3.
 * (b) When the value of HCR_EL2.TGE is 0, then Virtual
 * SError interrupts are enabled.
 *
 * Override CPSR.A
 */
#define HCR_AMO                   (_AC(1, UL) << 5)
/* Physical IRQ Routing
 * When executing at any Exception level, and EL2 is
 * enabled in the current Security state:
 * (a) Physical IRQ interrupts are taken to EL2, unless
 * they are routed to EL3.
 * (b) When the value of HCR_EL2.TGE is 0, then Virtual
 * IRQ interrupts are enabled.
 *
 * Override CPSR.I
 */
#define HCR_IMO                   (_AC(1, UL) << 4)
/* Physical FIQ Routing
 * When executing at any Exception level, and EL2 is
 * enabled in the current Security state:
 * (a) Physical FIQ interrupts are taken to EL2, unless
 * they are routed to EL3.
 * (b) When HCR_EL2.TGE is 0, then Virtual FIQ interrupts
 * are enabled.
 *
 * Override CPSR.F
 */
#define HCR_FMO                   (_AC(1, UL) << 3)
/* Protected Walk */
#define HCR_PTW                   (_AC(1, UL) << 2)
/* Set/Way Invalidation Override */
#define HCR_SWIO                  (_AC(1, UL) << 1)
/* Virtualization enable,  Enables stage 2 address
 * translation for the EL1&0 translation regime,
 * when EL2 is enabled in the current Security state.
 *
 * Virtual MMU Enable
 */
#define HCR_VM                    (_AC(1, UL) << 0)
#define HCR_EL2_SET               (HCR_AMO | HCR_IMO | HCR_FMO)

/* All ARMv8-A processors require the SMPEN bit to be set before
 * enabling the MMU and cache to support hardware coherency.
 */
#define SMP_ENABLE_BIT            BIT(6, UL)

/* Physical Address Register */
#define PAR_F           (_AC(1, U) << 0)

/* If F == 1 */
#define PAR_FSC_SHIFT   (1)
#define PAR_FSC_MASK    (_AC(0x3F, U) << PAR_FSC_SHIFT)
#define PAR_STAGE21     (_AC(1, U) << 8)     /* Stage 2 Fault During Stage 1 Walk */
#define PAR_STAGE2      (_AC(1, U) << 9)     /* Stage 2 Fault */

/* If F == 0 */
#define PAR_MAIR_SHIFT  56                       /* Memory Attributes */
#define PAR_MAIR_MASK   (0xFFLL << PAR_MAIR_SHIFT)
#define PAR_NS          (_AC(1, U) << 9)         /* Non-Secure */
#define PAR_SH_SHIFT    7                        /* Shareability */
#define PAR_SH_MASK     (_AC(3, U) << PAR_SH_SHIFT)

// --------------------------------------------------------------

/* The area of the instruction set space is reserved for
 * IMPLEMENTATION DEFINED registers.
 */
#define IMPL_REGS(op1, cn, cm, op2) \
    S3_ ## op1 ## _ ## cn ## _ ## cm ## _ ## op2

// --------------------------------------------------------------
#ifndef __ASSEMBLY__

#include <asm/alternative.h>
#include <common/stringify.h>
#include <common/type.h>

#define WRITE_SYSREG64(v, name) do {                          \
    unsigned long _r = (v);                                   \
    asm volatile("msr "__stringify(name)", %0" : : "r" (_r)); \
} while (0)

#define READ_SYSREG64(name) ({                                \
    unsigned long _r;                                         \
    asm volatile("mrs  %0, "__stringify(name) : "=r" (_r));   \
    _r; })

#define READ_SYSREG(name)          READ_SYSREG64(name)
#define WRITE_SYSREG(v, name)      WRITE_SYSREG64(v, name)
// --------------------------------------------------------------
#define ARM64_WORKAROUND_1508412   (17)

static inline u64 read_sysreg_par(void)
{
    u64 par_el1;
    asm volatile(ALTERNATIVE("nop", "dmb sy", ARM64_WORKAROUND_1508412,
                ARM64_WORKAROUND_1508412));
    par_el1 = READ_SYSREG64(PAR_EL1);
    asm volatile(ALTERNATIVE("nop", "dmb sy", ARM64_WORKAROUND_1508412,
                ARM64_WORKAROUND_1508412));

    return par_el1;
}

#endif /* !__ASSEMBLY__ */

#endif /* _ARCH_SYSREG_H */
// --------------------------------------------------------------
