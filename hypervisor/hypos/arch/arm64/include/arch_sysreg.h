/**
 * Hustler's Project
 *
 * File:  arch_sysreg.h
 * Date:  2024/05/20
 * Usage: ARMv8-A system register related defines
 */

#ifndef _ARCH_SYSREG_H
#define _ARCH_SYSREG_H
// ------------------------------------------------------------------------

#include <arch_bitops.h>

/* SCTLR_EL2
 *
 * M bit[0]  - MMU enable
 * A bit[1]  - Alignment fault checking
 * C bit[2]  - Data access cacheability
 * SA bit[3] - SP alignment check enable
 * I bit[12] - Instruction access cacheability
 */
#define SCTLR_INITVAL             0x30C50878
#define SCTLR_TE_MASK             0x40000000
#define SCTLR_TE_SHIFT            30
#define SCTLR_EE_MASK             0x02000000
#define SCTLR_EE_SHIFT            25
#define SCTLR_FI_MASK             0x00200000
#define SCTLR_FI_SHIFT            21
#define SCTLR_WXN_MASK            0x00080000
#define SCTLR_WXN_SHIFT           19
#define SCTLR_I_MASK              0x00001000
#define SCTLR_I_SHIFT             12
#define SCTLR_SA0_MASK            0x00000010
#define SCTLR_SA0_SHIFT           4
#define SCTLR_SA_MASK             0x00000008
#define SCTLR_SA_SHIFT            3
#define SCTLR_C_MASK              0x00000004
#define SCTLR_C_SHIFT             2
#define SCTLR_A_MASK              0x00000002
#define SCTLR_A_SHIFT             1
#define SCTLR_M_MASK              0x00000001
#define SCTLR_M_SHIFT             0
#define ARCH_SCTLR_MMU_EN         BIT(SCTLR_M_SHIFT, UL)
#define ARCH_SCTLR_DCACHE_EN      BIT(SCTLR_C_SHIFT, UL)

/* MAIR_EL2
 */
#define MAIR(val, idx)            ((val) << ((idx) * 8))
#define MAIR_ATTR0                0
#define MAIR_ATTR1                1
#define MAIR_ATTR2                2
#define MAIR_ATTR3                3
#define MAIR_ATTR4                4
#define MAIR_ATTR5                5
#define MAIR_ATTR6                6
#define MAIR_ATTR7                7
#define ARCH_MAIR_DEVICE_nGnRnE   MAIR(0x00, MAIR_ATTR0)
#define ARCH_MAIR_DEVICE_nGnRE    MAIR(0x04, MAIR_ATTR1)
#define ARCH_MAIR_DEVICE_nGRE     MAIR(0x08, MAIR_ATTR2)
#define ARCH_MAIR_DEVICE_GRE      MAIR(0x0C, MAIR_ATTR3)
/* Normal Memory: Outer/Inner write-through non-transient */
#define ARCH_MAIR_NORMAL_WT       MAIR(0xBB, MAIR_ATTR4)
/* Normal Memory: Outer/Inner write-Back non-transient */
#define ARCH_MAIR_NORMAL_WB       MAIR(0xFF, MAIR_ATTR5)
/* Normal Memory: Outer/Inner non-cacheable */
#define ARCH_MAIR_NORMAL_NC       MAIR(0x44, MAIR_ATTR6)
#define ARCH_MAIR_EL2_SET \
                                  (ARCH_MAIR_DEVICE_nGnRnE | \
                                   ARCH_MAIR_DEVICE_nGnRE | \
                                   ARCH_MAIR_DEVICE_nGRE | \
                                   ARCH_MAIR_DEVICE_GRE | \
                                   ARCH_MAIR_NORMAL_WT | \
                                   ARCH_MAIR_NORMAL_WB | \
                                   ARCH_MAIR_NORMAL_NC)

/* TCR_EL2
 */
#define TCR_INITVAL               0x80800000
#define TCR_TBI_MASK              0x00100000
#define TCR_TBI_SHIFT             20
#define TCR_PS_MASK               0x00070000
#define TCR_PS_SHIFT              16
#define TCR_TG0_MASK              0x0000c000
#define TCR_TG0_SHIFT             14
#define TCR_SH0_MASK              0x00003000
#define TCR_SH0_SHIFT             12
#define TCR_ORGN0_MASK            0x00000C00
#define TCR_ORGN0_SHIFT           10
#define TCR_IRGN0_MASK            0x00000300
#define TCR_IRGN0_SHIFT           8
#define TCR_T0SZ_MASK             0x0000003F
#define TCR_T0SZ_SHIFT            0

#define TCR_PS_32BITS             (0 << TCR_PS_SHIFT)
#define TCR_PS_36BITS             (1 << TCR_PS_SHIFT)
#define TCR_PS_40BITS             (2 << TCR_PS_SHIFT)
#define TCR_PS_42BITS             (3 << TCR_PS_SHIFT)
#define TCR_PS_44BITS             (4 << TCR_PS_SHIFT)
#define TCR_PS_48BITS             (5 << TCR_PS_SHIFT)
#define TCR_T0SZ_VAL(in_bits)     ((64 - (in_bits)) & TCR_T0SZ_MASK)
#define ARCH_TCR_EL2_SET          (TCR_T0SZ_VAL(39) | \
                                  TCR_PS_40BITS | \
                                  (0x0 << TCR_TG0_SHIFT) | \
                                  (0x3 << TCR_SH0_SHIFT) | \
                                  (0x1 << TCR_ORGN0_SHIFT) | \
                                  (0x1 << TCR_IRGN0_SHIFT))


/* VTCR_EL2
 */
#define VTCR_INITVAL              0x80000000
#define VTCR_PS_MASK              0x00070000
#define VTCR_PS_SHIFT             16
#define VTCR_TG0_MASK             0x0000C000
#define VTCR_TG0_SHIFT            14
#define VTCR_SH0_MASK             0x00003000
#define VTCR_SH0_SHIFT            12
#define VTCR_ORGN0_MASK           0x00000C00
#define VTCR_ORGN0_SHIFT          10
#define VTCR_IRGN0_MASK           0x00000300
#define VTCR_IRGN0_SHIFT          8
#define VTCR_SL0_MASK             0x000000C0
#define VTCR_SL0_SHIFT            6
#define VTCR_T0SZ_MASK            0x0000003F
#define VTCR_T0SZ_SHIFT           0

#define VTCR_PS_32BITS            (0 << VTCR_PS_SHIFT)
#define VTCR_PS_36BITS            (1 << VTCR_PS_SHIFT)
#define VTCR_PS_40BITS            (2 << VTCR_PS_SHIFT)
#define VTCR_PS_42BITS            (3 << VTCR_PS_SHIFT)
#define VTCR_PS_44BITS            (4 << VTCR_PS_SHIFT)
#define VTCR_PS_48BITS            (5 << VTCR_PS_SHIFT)
#define VTCR_SL0_L2               (0 << VTCR_SL0_SHIFT) /* Starting-level: 2 */
#define VTCR_SL0_L1               (1 << VTCR_SL0_SHIFT) /* Starting-level: 1 */
#define VTCR_SL0_L0               (2 << VTCR_SL0_SHIFT) /* Starting-level: 0 */
#define VTCR_T0SZ_VAL(in_bits)    ((64 - (in_bits)) & VTCR_T0SZ_MASK)
#define ARCH_VTCR_EL2_SET         (VTCR_SL0_L1 | \
                                  VTCR_T0SZ_VAL(39) | \
                                  VTCR_PS_40BITS | \
                                  (0x0 << VTCR_TG0_SHIFT) | \
                                  (0x3 << VTCR_SH0_SHIFT) | \
                                  (0x1 << VTCR_ORGN0_SHIFT) | \
                                  (0x1 << VTCR_IRGN0_SHIFT))

/* HCR_EL2
 */
#define HCR_INITVAL               0x000000000
#define HCR_FWB_MASK              _AC(0x400000000000, UL)
#define HCR_FWB_SHIFT             46
#define HCR_APL_MASK              _AC(0x20000000000, UL)
#define HCR_APL_SHIFT             41
#define HCR_APK_MASK              _AC(0x10000000000, UL)
#define HCR_APK_SHIFT             40
#define HCR_TEA_MASK              _AC(0x2000000000, UL)
#define HCR_TEA_SHIFT             37
#define HCR_TERR_MASK             _AC(0x1000000000, UL)
#define HCR_TERR_SHIFT            36
#define HCR_TLOR_MASK             _AC(0x800000000, UL)
#define HCR_TLOR_SHIFT            35
#define HCR_E2H_MASK              _AC(0x400000000, UL)
#define HCR_E2H_SHIFT             34
#define HCR_ID_MASK               _AC(0x200000000, UL)
#define HCR_ID_SHIFT              33
#define HCR_CD_MASK               _AC(0x100000000, UL)
#define HCR_CD_SHIFT              32
#define HCR_RW_MASK               0x080000000
#define HCR_RW_SHIFT              31
#define HCR_TRVM_MASK             0x040000000
#define HCR_TRVM_SHIFT            30
#define HCR_HCD_MASK              0x020000000
#define HCR_HCD_SHIFT             29
#define HCR_TDZ_MASK              0x010000000
#define HCR_TDZ_SHIFT             28
#define HCR_TGE_MASK              0x008000000
#define HCR_TGE_SHIFT             27
#define HCR_TVM_MASK              0x004000000
#define HCR_TVM_SHIFT             26
#define HCR_TTLB_MASK             0x002000000
#define HCR_TTLB_SHIFT            25
#define HCR_TPU_MASK              0x001000000
#define HCR_TPU_SHIFT             24
#define HCR_TPC_MASK              0x000800000
#define HCR_TPC_SHIFT             23
#define HCR_TSW_MASK              0x000400000
#define HCR_TSW_SHIFT             22
#define HCR_TACR_MASK             0x000200000
#define HCR_TACR_SHIFT            21
#define HCR_TIDCP_MASK            0x000100000
#define HCR_TIDCP_SHIFT           20
#define HCR_TSC_MASK              0x000080000
#define HCR_TSC_SHIFT             19
#define HCR_TID3_MASK             0x000040000
#define HCR_TID3_SHIFT            18
#define HCR_TID2_MASK             0x000020000
#define HCR_TID2_SHIFT            17
#define HCR_TID1_MASK             0x000010000
#define HCR_TID1_SHIFT            16
#define HCR_TID0_MASK             0x000008000
#define HCR_TID0_SHIFT            15
#define HCR_TWE_MASK              0x000004000
#define HCR_TWE_SHIFT             14
#define HCR_TWI_MASK              0x000002000
#define HCR_TWI_SHIFT             13
#define HCR_DC_MASK               0x000001000
#define HCR_DC_SHIFT              12
#define HCR_BSU_MASK              0x000000C00
#define HCR_BSU_SHIFT             10
#define HCR_FB_MASK               0x000000200
#define HCR_FB_SHIFT              9
/* Virtual SError interrupt pending
 */
#define HCR_VSE_MASK              0x000000100
#define HCR_VSE_SHIFT             8
/* Virtual IRQ interrupt pending
 */
#define HCR_VI_MASK               0x000000080
#define HCR_VI_SHIFT              7
/* Virtual FIQ interrupt pending
 */
#define HCR_VF_MASK               0x000000040
#define HCR_VF_SHIFT              6

/* Physical SError interrupt Routing
 * When executing at any Exception level, and EL2 is
 * enabled in the current Security state:
 * (a) Physical SError interrupts are taken to EL2, unless
 * they are routed to EL3.
 * (b) When the value of HCR_EL2.TGE is 0, then Virtual
 * SError interrupts are enabled.
 */
#define HCR_AMO_MASK              0x000000020
#define HCR_AMO_SHIFT             5

/* Physical IRQ Routing
 * When executing at any Exception level, and EL2 is
 * enabled in the current Security state:
 * (a) Physical IRQ interrupts are taken to EL2, unless
 * they are routed to EL3.
 * (b) When the value of HCR_EL2.TGE is 0, then Virtual
 * IRQ interrupts are enabled.
 */
#define HCR_IMO_MASK              0x000000010
#define HCR_IMO_SHIFT             4

/* Physical FIQ Routing
 * When executing at any Exception level, and EL2 is
 * enabled in the current Security state:
 * (a) Physical FIQ interrupts are taken to EL2, unless
 * they are routed to EL3.
 * (b) When HCR_EL2.TGE is 0, then Virtual FIQ interrupts
 * are enabled.
 */
#define HCR_FMO_MASK              0x000000008
#define HCR_FMO_SHIFT             3
#define HCR_PTW_MASK              0x000000004
#define HCR_PTW_SHIFT             2
#define HCR_SWIO_MASK             0x000000002
#define HCR_SWIO_SHIFT            1

/* Virtualization enable,  Enables stage 2 address
 * translation for the EL1&0 translation regime,
 * when EL2 is enabled in the current Security state.
 */
#define HCR_VM_MASK               0x000000001
#define HCR_VM_SHIFT              0
#define ARCH_HCR_EL2_SET          (HCR_AMO_MASK | \
                                  HCR_IMO_MASK |  \
                                  HCR_FMO_MASK |  \
                                  HCR_VM_MASK)
// ------------------------------------------------------------------------
#define __stringify(x...)    #x

#define WRITE_SYSREG64(v, name) do {                          \
    uint64_t _r = (v);\
    asm volatile("msr "__stringify(name)", %0" : : "r" (_r)); \
} while (0)

#define READ_SYSREG64(name) ({                                \
    uint64_t _r;                                              \
    asm volatile("mrs  %0, "__stringify(name) : "=r" (_r));   \
    _r; })

#define READ_SYSREG(name)          READ_SYSREG64(name)
#define WRITE_SYSREG(v, name)      WRITE_SYSREG64(v, name)

#endif /* _ARCH_SYSREG_H */
// ------------------------------------------------------------------------
