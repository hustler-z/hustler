/**
 * Hustler's Project
 *
 * File:  gic.h
 * Date:  2024/07/10
 * Usage:
 */

#ifndef _ORG_GIC_H
#define _ORG_GIC_H
// --------------------------------------------------------------
#include <bsp/size.h>

/* CPU Interfaces
 * -----------------------------------------------------------------------
 * Physical CPU Interface | Virtualization Control | Virtual CPU Interface
 *        ICC_*_ELx       |      ICH_*_EL2         |       ICV_*_EL1
 * -----------------------------------------------------------------------
 *            |IRQ               |FIQ                  |vIRQ      |vFIQ
 *            ▼                  ▼                     ▼          ▼
 * +----------------------------------------------+ +--------------------+
 * |                Physical PE                   | |     Virtual PE     |
 * +----------------------------------------------+ +--------------------+
 *
 * The hypervisor executing at EL2 uses the regular ICC_*_ELx registers to
 * handle physical interrupts.
 *
 * The hypervisor has access to additional registers to control the
 * virtualization features via ICH_*_EL2:
 * (a) Enabling and disabling the virtual CPU interface.
 * (b) Accessing virtual register state to enable context switching.
 * (c) Configuring maintenance interrupts.
 * (d) Controlling virtual interrupts for the currently scheduled vPE.
 *
 * Software executing in a virtualized environment uses the ICV_*_EL1
 * registers to handle virtual interrupts.
 * -----------------------------------------------------------------------
 */
#define NR_GIC_LOCAL_IRQS  NR_LOCAL_IRQS
#define NR_GIC_SGI         16

#define GICD_CTLR          (0x000)
#define GICD_TYPER         (0x004)
#define GICD_IIDR          (0x008)
#define GICD_IGROUPR       (0x080)
#define GICD_IGROUPRN      (0x0FC)
#define GICD_ISENABLER     (0x100)
#define GICD_ISENABLERN    (0x17C)
#define GICD_ICENABLER     (0x180)
#define GICD_ICENABLERN    (0x1fC)
#define GICD_ISPENDR       (0x200)
#define GICD_ISPENDRN      (0x27C)
#define GICD_ICPENDR       (0x280)
#define GICD_ICPENDRN      (0x2FC)
#define GICD_ISACTIVER     (0x300)
#define GICD_ISACTIVERN    (0x37C)
#define GICD_ICACTIVER     (0x380)
#define GICD_ICACTIVERN    (0x3FC)
#define GICD_IPRIORITYR    (0x400)
#define GICD_IPRIORITYRN   (0x7F8)
#define GICD_ITARGETSR     (0x800)
#define GICD_ITARGETSR7    (0x81C)
#define GICD_ITARGETSR8    (0x820)
#define GICD_ITARGETSRN    (0xBF8)
#define GICD_ICFGR         (0xC00)
#define GICD_ICFGR1        (0xC04)
#define GICD_ICFGR2        (0xC08)
#define GICD_ICFGRN        (0xCFC)
#define GICD_NSACR         (0xE00)
#define GICD_NSACRN        (0xEFC)
#define GICD_SGIR          (0xF00)
#define GICD_CPENDSGIR     (0xF10)
#define GICD_CPENDSGIRN    (0xF1C)
#define GICD_SPENDSGIR     (0xF20)
#define GICD_SPENDSGIRN    (0xF2C)
#define GICD_ICPIDR2       (0xFE8)

#define GICD_SGI_TARGET_LIST_SHIFT   (24)
#define GICD_SGI_TARGET_LIST_MASK    (0x3UL << GICD_SGI_TARGET_LIST_SHIFT)
#define GICD_SGI_TARGET_LIST         (0UL<<GICD_SGI_TARGET_LIST_SHIFT)
#define GICD_SGI_TARGET_LIST_VAL     (0)
#define GICD_SGI_TARGET_OTHERS       (1UL<<GICD_SGI_TARGET_LIST_SHIFT)
#define GICD_SGI_TARGET_OTHERS_VAL   (1)
#define GICD_SGI_TARGET_SELF         (2UL<<GICD_SGI_TARGET_LIST_SHIFT)
#define GICD_SGI_TARGET_SELF_VAL     (2)
#define GICD_SGI_TARGET_SHIFT        (16)
#define GICD_SGI_TARGET_MASK         (0xFFUL<<GICD_SGI_TARGET_SHIFT)
#define GICD_SGI_GROUP1              (1UL<<15)
#define GICD_SGI_INTID_MASK          (0xFUL)

#define GICC_CTLR          (0x0000)
#define GICC_PMR           (0x0004)
#define GICC_BPR           (0x0008)
#define GICC_IAR           (0x000C)
#define GICC_EOIR          (0x0010)
#define GICC_RPR           (0x0014)
#define GICC_HPPIR         (0x0018)
#define GICC_APR           (0x00D0)
#define GICC_NSAPR         (0x00E0)
#define GICC_IIDR          (0x00FC)
#define GICC_DIR           (0x1000)

#define GICH_HCR           (0x00)
#define GICH_VTR           (0x04)
#define GICH_VMCR          (0x08)
#define GICH_MISR          (0x10)
#define GICH_EISR0         (0x20)
#define GICH_EISR1         (0x24)
#define GICH_ELSR0         (0x30)
#define GICH_ELSR1         (0x34)
#define GICH_APR           (0xF0)
#define GICH_LR            (0x100)

/* Register bits */
#define GICD_CTL_ENABLE      0x1

#define GICD_TYPE_LINES      0x01F
#define GICD_TYPE_CPUS_SHIFT 5
#define GICD_TYPE_CPUS       0x0E0
#define GICD_TYPE_SEC        0x400
#define GICD_TYPER_DVIS      (1U << 18)

#define GICC_CTL_ENABLE      0x1
#define GICC_CTL_EOI         (0x1 << 9)

#define GICC_IA_IRQ          0x03FF
#define GICC_IA_CPU_MASK     0x1C00
#define GICC_IA_CPU_SHIFT    10

#define GICH_HCR_EN          (1 << 0)
#define GICH_HCR_UIE         (1 << 1)
#define GICH_HCR_LRENPIE     (1 << 2)
#define GICH_HCR_NPIE        (1 << 3)
#define GICH_HCR_VGRP0EIE    (1 << 4)
#define GICH_HCR_VGRP0DIE    (1 << 5)
#define GICH_HCR_VGRP1EIE    (1 << 6)
#define GICH_HCR_VGRP1DIE    (1 << 7)

#define GICH_MISR_EOI        (1 << 0)
#define GICH_MISR_U          (1 << 1)
#define GICH_MISR_LRENP      (1 << 2)
#define GICH_MISR_NP         (1 << 3)
#define GICH_MISR_VGRP0E     (1 << 4)
#define GICH_MISR_VGRP0D     (1 << 5)
#define GICH_MISR_VGRP1E     (1 << 6)
#define GICH_MISR_VGRP1D     (1 << 7)

/*
 * The minimum GICC_BPR is required to be in the range 0-3. We set
 * GICC_BPR to 0 but we must expect that it might be 3. This means we
 * can rely on premption between the following ranges:
 * 0xf0..0xff
 * 0xe0..0xdf
 * 0xc0..0xcf
 * 0xb0..0xbf
 * 0xa0..0xaf
 * 0x90..0x9f
 * 0x80..0x8f
 *
 * Priorities within a range will not preempt each other.
 *
 * A GIC must support a mimimum of 16 priority levels.
 */
#define GIC_PRI_LOWEST       0xF0U
#define GIC_PRI_IRQ          0xa0U
#define GIC_PRI_IPI          0x90U
#define GIC_PRI_HIGHEST      0x80U
#define GIC_PRI_IRQ_ALL ((GIC_PRI_IRQ << 24) | (GIC_PRI_IRQ << 16) |\
                         (GIC_PRI_IRQ << 8) | GIC_PRI_IRQ)
#define GIC_PRI_IPI_ALL ((GIC_PRI_IPI << 24) | (GIC_PRI_IPI << 16) |\
                         (GIC_PRI_IPI << 8) | GIC_PRI_IPI)

/* GICH_LR and GICH_VMCR only support
 * 5 bits for guest irq priority
 */
#define GIC_PRI_TO_GUEST(pri) ((pri) >> 3)

#define GICH_LR_PENDING         1
#define GICH_LR_ACTIVE          2

// --------------------------------------------------------------

/* GICv3 Defines */

/*
 * Additional registers defined in GIC v3.
 * Common GICD registers are defined in gic.h
 */
#define GICD_STATUSR                 (0x010)
#define GICD_SETSPI_NSR              (0x040)
#define GICD_CLRSPI_NSR              (0x048)
#define GICD_SETSPI_SR               (0x050)
#define GICD_CLRSPI_SR               (0x058)
#define GICD_IGRPMODR                (0xD00)
#define GICD_IGRPMODRN               (0xD7C)
#define GICD_IROUTER                 (0x6000)
#define GICD_IROUTER32               (0x6100)
#define GICD_IROUTER1019             (0x7FD8)
#define GICD_PIDR2                   (0xFFE8)

/* Common between GICD_PIDR2 and GICR_PIDR2 */
#define GIC_PIDR2_ARCH_MASK          (0xf0)
#define GIC_PIDR2_ARCH_GICv3         (0x30)
#define GIC_PIDR2_ARCH_GICv4         (0x40)

#define GICC_SRE_EL2_SRE             (1UL << 0)
#define GICC_SRE_EL2_DFB             (1UL << 1)
#define GICC_SRE_EL2_DIB             (1UL << 2)
#define GICC_SRE_EL2_ENEL1           (1UL << 3)

#define GICC_IAR_INTID_MASK          (0xFFFFFF)

/* Additional bits in GICD_TYPER defined by GICv3 */
#define GICD_TYPE_ID_BITS_SHIFT 19
#define GICD_TYPE_ID_BITS(r) \
    ((((r) >> GICD_TYPE_ID_BITS_SHIFT) & 0x1F) + 1)

#define GICD_TYPE_LPIS               (1U << 17)

#define GICD_CTLR_RWP                (1UL << 31)
#define GICD_CTLR_ARE_NS             (1U << 4)
#define GICD_CTLR_ENABLE_G1A         (1U << 1)
#define GICD_CTLR_ENABLE_G1          (1U << 0)
#define GICD_IROUTER_SPI_MODE_ANY    (1UL << 31)

#define GICC_CTLR_EL1_EOImode_drop   (1U << 1)

#define GICR_WAKER_ProcessorSleep    (1U << 1)
#define GICR_WAKER_ChildrenAsleep    (1U << 2)

#define GICR_SYNCR_NOT_BUSY          1
/*
 * Implementation defined value JEP106?
 * use physical hw value for now
 */
#define GICV3_GICD_IIDR_VAL          0x34C
#define GICV3_GICR_IIDR_VAL          GICV3_GICD_IIDR_VAL

/* Two pages for the RD_base and SGI_base register frame. */
#define GICV3_GICR_SIZE              (2 * KB(64))

#define GICR_CTLR                    (0x0000)
#define GICR_IIDR                    (0x0004)
#define GICR_TYPER                   (0x0008)
#define GICR_STATUSR                 (0x0010)
#define GICR_WAKER                   (0x0014)
#define GICR_SETLPIR                 (0x0040)
#define GICR_CLRLPIR                 (0x0048)
#define GICR_PROPBASER               (0x0070)
#define GICR_PENDBASER               (0x0078)
#define GICR_INVLPIR                 (0x00A0)
#define GICR_INVALLR                 (0x00B0)
#define GICR_SYNCR                   (0x00C0)
#define GICR_PIDR2                   GICD_PIDR2

/* GICR for SGI's & PPI's */
#define GICR_IGROUPR0                (0x0080)
#define GICR_ISENABLER0              (0x0100)
#define GICR_ICENABLER0              (0x0180)
#define GICR_ISPENDR0                (0x0200)
#define GICR_ICPENDR0                (0x0280)
#define GICR_ISACTIVER0              (0x0300)
#define GICR_ICACTIVER0              (0x0380)
#define GICR_IPRIORITYR0             (0x0400)
#define GICR_IPRIORITYR7             (0x041C)
#define GICR_ICFGR0                  (0x0C00)
#define GICR_ICFGR1                  (0x0C04)
#define GICR_IGRPMODR0               (0x0D00)
#define GICR_NSACR                   (0x0E00)

#define GICR_CTLR_ENABLE_LPIS        (1U << 0)
#define GICR_TYPER_PLPIS             (1U << 0)
#define GICR_TYPER_VLPIS             (1U << 1)
#define GICR_TYPER_LAST              (1U << 4)
#define GICR_TYPER_PROC_NUM_SHIFT    8
#define GICR_TYPER_PROC_NUM_MASK     (0xFFFF << GICR_TYPER_PROC_NUM_SHIFT)

/* For specifying the inner cacheability type only */
#define GIC_BASER_CACHE_nCnB         0ULL
/* For specifying the outer cacheability type only */
#define GIC_BASER_CACHE_SameAsInner  0ULL
#define GIC_BASER_CACHE_nC           1ULL
#define GIC_BASER_CACHE_RaWt         2ULL
#define GIC_BASER_CACHE_RaWb         3ULL
#define GIC_BASER_CACHE_WaWt         4ULL
#define GIC_BASER_CACHE_WaWb         5ULL
#define GIC_BASER_CACHE_RaWaWt       6ULL
#define GIC_BASER_CACHE_RaWaWb       7ULL
#define GIC_BASER_CACHE_MASK         7ULL

#define GIC_BASER_NonShareable       0ULL
#define GIC_BASER_InnerShareable     1ULL
#define GIC_BASER_OuterShareable     2ULL

#define GICR_PROPBASER_OUTER_CACHEABILITY_SHIFT         56
#define GICR_PROPBASER_OUTER_CACHEABILITY_MASK               \
        (7ULL << GICR_PROPBASER_OUTER_CACHEABILITY_SHIFT)
#define GICR_PROPBASER_SHAREABILITY_SHIFT               10
#define GICR_PROPBASER_SHAREABILITY_MASK                     \
        (3ULL << GICR_PROPBASER_SHAREABILITY_SHIFT)
#define GICR_PROPBASER_INNER_CACHEABILITY_SHIFT         7
#define GICR_PROPBASER_INNER_CACHEABILITY_MASK               \
        (7ULL << GICR_PROPBASER_INNER_CACHEABILITY_SHIFT)
#define GICR_PROPBASER_RES0_MASK                             \
        (GENMASK_ULL(63, 59) | GENMASK_ULL(55, 52) | GENMASK_ULL(6, 5))

#define GICR_PENDBASER_SHAREABILITY_SHIFT               10
#define GICR_PENDBASER_INNER_CACHEABILITY_SHIFT         7
#define GICR_PENDBASER_OUTER_CACHEABILITY_SHIFT         56
#define GICR_PENDBASER_SHAREABILITY_MASK                     \
	(3UL << GICR_PENDBASER_SHAREABILITY_SHIFT)
#define GICR_PENDBASER_INNER_CACHEABILITY_MASK               \
	(7UL << GICR_PENDBASER_INNER_CACHEABILITY_SHIFT)
#define GICR_PENDBASER_OUTER_CACHEABILITY_MASK               \
        (7ULL << GICR_PENDBASER_OUTER_CACHEABILITY_SHIFT)
#define GICR_PENDBASER_PTZ           BIT(62, ULL)
#define GICR_PENDBASER_RES0_MASK                             \
        (BIT(63, ULL) | GENMASK_ULL(61, 59) | GENMASK_ULL(55, 52) |  \
         GENMASK_ULL(15, 12) | GENMASK_ULL(6, 0))

#define DEFAULT_PMR_VALUE            0xFF

#define LPI_PROP_PRIO_MASK           0xFC
#define LPI_PROP_RES1                (1 << 1)
#define LPI_PROP_ENABLED             (1 << 0)

#define ICH_VMCR_EOI                 (1 << 9)
#define ICH_VMCR_VENG1               (1 << 1)
#define ICH_VMCR_PRIORITY_MASK       0xFF
#define ICH_VMCR_PRIORITY_SHIFT      24

#define ICH_LR_VIRTUAL_MASK          0xFFFF
#define ICH_LR_VIRTUAL_SHIFT         0
#define ICH_LR_CPUID_MASK            0x7
#define ICH_LR_CPUID_SHIFT           10
#define ICH_LR_PHYSICAL_MASK         0x3FF
#define ICH_LR_PHYSICAL_SHIFT        32
#define ICH_LR_STATE_MASK            0x3
#define ICH_LR_STATE_SHIFT           62
#define ICH_LR_STATE_PENDING         (1ULL << 62)
#define ICH_LR_STATE_ACTIVE          (1ULL << 63)
#define ICH_LR_PRIORITY_MASK         0xFF
#define ICH_LR_PRIORITY_SHIFT        48
#define ICH_LR_HW_MASK               0x1
#define ICH_LR_HW_SHIFT              61
#define ICH_LR_GRP_MASK              0x1
#define ICH_LR_GRP_SHIFT             60
#define ICH_LR_MAINTENANCE_IRQ       (1ULL << 41)
#define ICH_LR_GRP1                  (1ULL << 60)
#define ICH_LR_HW                    (1ULL << 61)

#define ICH_VTR_NRLRGS               0x3F
#define ICH_VTR_PRIBITS_MASK         0x7
#define ICH_VTR_PRIBITS_SHIFT        29

#define ICH_SGI_IRQMODE_SHIFT        40
#define ICH_SGI_IRQMODE_MASK         0x1
#define ICH_SGI_TARGET_OTHERS        1ULL
#define ICH_SGI_TARGET_LIST          0
#define ICH_SGI_IRQ_SHIFT            24
#define ICH_SGI_IRQ_MASK             0xF
#define ICH_SGI_TARGETLIST_MASK      0xFFFF
#define ICH_SGI_AFFx_MASK            0xFF
#define ICH_SGI_AFFINITY_LEVEL(x)    (16 * (x))
// --------------------------------------------------------------

/* GICv3 ITS */
#define GITS_CTLR                       0x000
#define GITS_IIDR                       0x004
#define GITS_TYPER                      0x008
#define GITS_CBASER                     0x080
#define GITS_CWRITER                    0x088
#define GITS_CREADR                     0x090
#define GITS_BASER_NR_REGS              8
#define GITS_BASER0                     0x100
#define GITS_BASER1                     0x108
#define GITS_BASER2                     0x110
#define GITS_BASER3                     0x118
#define GITS_BASER4                     0x120
#define GITS_BASER5                     0x128
#define GITS_BASER6                     0x130
#define GITS_BASER7                     0x138
#define GITS_PIDR2                      GICR_PIDR2

/* Register bits */
#define GITS_VALID_BIT                  BIT(63, UL)

#define GITS_CTLR_QUIESCENT             BIT(31, UL)
#define GITS_CTLR_ENABLE                BIT(0, UL)

#define GITS_TYPER_PTA                  BIT(19, UL)
#define GITS_TYPER_DEVIDS_SHIFT         13
#define GITS_TYPER_DEVIDS_MASK       \
    (0x1FUL << GITS_TYPER_DEVIDS_SHIFT)
#define GITS_TYPER_DEVICE_ID_BITS(r) \
    ((((r) & GITS_TYPER_DEVIDS_MASK) >> \
        GITS_TYPER_DEVIDS_SHIFT) + 1)

#define GITS_TYPER_IDBITS_SHIFT         8
#define GITS_TYPER_IDBITS_MASK       \
    (0x1FUL << GITS_TYPER_IDBITS_SHIFT)
#define GITS_TYPER_EVENT_ID_BITS(r)  \
    ((((r) & GITS_TYPER_IDBITS_MASK) >> \
        GITS_TYPER_IDBITS_SHIFT) + 1)

#define GITS_TYPER_ITT_SIZE_SHIFT       4
#define GITS_TYPER_ITT_SIZE_MASK     \
    (0xFUL << GITS_TYPER_ITT_SIZE_SHIFT)
#define GITS_TYPER_ITT_SIZE(r) \
    ((((r) & GITS_TYPER_ITT_SIZE_MASK) >> \
        GITS_TYPER_ITT_SIZE_SHIFT) + 1)
#define GITS_TYPER_PHYSICAL             (1U << 0)

#define GITS_BASER_INDIRECT             BIT(62, UL)
#define GITS_BASER_INNER_CACHEABILITY_SHIFT        59
#define GITS_BASER_TYPE_SHIFT           56
#define GITS_BASER_TYPE_MASK            (7ULL << GITS_BASER_TYPE_SHIFT)
#define GITS_BASER_OUTER_CACHEABILITY_SHIFT        53
#define GITS_BASER_TYPE_NONE            0UL
#define GITS_BASER_TYPE_DEVICE          1UL
#define GITS_BASER_TYPE_VCPU            2UL
#define GITS_BASER_TYPE_CPU             3UL
#define GITS_BASER_TYPE_COLLECTION      4UL
#define GITS_BASER_TYPE_RESERVED5       5UL
#define GITS_BASER_TYPE_RESERVED6       6UL
#define GITS_BASER_TYPE_RESERVED7       7UL
#define GITS_BASER_ENTRY_SIZE_SHIFT     48
#define GITS_BASER_ENTRY_SIZE(reg)   \
    ((((reg) >> GITS_BASER_ENTRY_SIZE_SHIFT) & 0x1F) + 1)
#define GITS_BASER_SHAREABILITY_SHIFT   10
#define GITS_BASER_PAGE_SIZE_SHIFT      8
#define GITS_BASER_SIZE_MASK            0xFF
#define GITS_BASER_SHAREABILITY_MASK \
    (0x3ULL << GITS_BASER_SHAREABILITY_SHIFT)
#define GITS_BASER_OUTER_CACHEABILITY_MASK \
    (0x7ULL << GITS_BASER_OUTER_CACHEABILITY_SHIFT)
#define GITS_BASER_INNER_CACHEABILITY_MASK \
    (0x7ULL << GITS_BASER_INNER_CACHEABILITY_SHIFT)

#define GITS_CBASER_SIZE_MASK           0xff

/* ITS command definitions */
#define ITS_CMD_SIZE                    32

#define GITS_CMD_MOVI                   0x01
#define GITS_CMD_INT                    0x03
#define GITS_CMD_CLEAR                  0x04
#define GITS_CMD_SYNC                   0x05
#define GITS_CMD_MAPD                   0x08
#define GITS_CMD_MAPC                   0x09
#define GITS_CMD_MAPTI                  0x0A
#define GITS_CMD_MAPI                   0x0B
#define GITS_CMD_INV                    0x0C
#define GITS_CMD_INVALL                 0x0D
#define GITS_CMD_MOVALL                 0x0E
#define GITS_CMD_DISCARD                0x0F

#define ITS_DOORBELL_OFFSET             0x10040
#define GICV3_ITS_SIZE                  KB(128)

#ifndef __ASSEMBLY__
// --------------------------------------------------------------
#include <org/smp.h>
#include <org/vcpu.h>
#include <bsp/irq.h>
#include <lib/list.h>

struct rdist_region {
    hpa_t base;
    hpa_t size;
    void __iomem *map_base;
    bool single_rdist;
};

#define HOST_ITS_FLUSH_CMD_QUEUE        (1U << 0)
#define HOST_ITS_USES_PTA               (1U << 1)

/* We allocate LPIs on the hosts in chunks of 32 to
 * reduce handling overhead.
 */
#define LPI_BLOCK                       32U

/* Data structure for each hardware ITS */
struct host_its {
    struct list_head entry;
    hpa_t addr;
    hpa_t size;
    void __iomem *its_base;
    unsigned int devid_bits;
    unsigned int evid_bits;
    unsigned int itte_size;
    spinlock_t   cmd_lock;
    void         *cmd_buf;
    unsigned int flags;
};

struct gic_v3 {
    register_t vmcr, sre_el1;
    register_t apr0[4];
    register_t apr1[4];
    u64        lr[16];
};

struct gic_v2 {
    u32 hcr;
    u32 vmcr;
    u32 apr;
    u32 lr[64];
};

union gic_state_data {
    struct gic_v2 v2;
    struct gic_v3 v3;
};

struct gic_lr {
    u32  virq;
    u8   priority;
    bool active;
    bool pending;
    bool hw_status;

    union {
       struct {
           u32 pirq;
       } hw;

       struct {
           bool eoi;
           u8   source;
       } virt;
    };
};

enum gic_version {
    GIC_INVALID = 0,
    GIC_V2,
    GIC_V3,
};

/* SGI (AKA IPIs) */
enum gic_sgi {
    GIC_SGI_EVENT_CHECK,
    GIC_SGI_DUMP_STATE,
    GIC_SGI_CALL_FUNCTION,
    GIC_SGI_MAX,
};

/* SGI irq mode types */
enum gic_sgi_mode {
    SGI_TARGET_LIST,
    SGI_TARGET_OTHERS,
    SGI_TARGET_SELF,
};

struct gic_info {
    enum gic_version hw_version;
    unsigned int nr_lines;
    u8 nr_lrs;
    unsigned int maintenance_irq;
};

void send_sgi_mask(const cpumask_t *cpumask, enum gic_sgi sgi);
void send_sgi_one(unsigned int cpu, enum gic_sgi sgi);
void send_sgi_self(enum gic_sgi sgi);
void send_sgi_allbutself(enum gic_sgi sgi);

// --------------------------------------------------------------
struct hypos;
struct vcpu;

/* XXX: GIC Hardware Operations Setups
 */
struct gic_hw_operations {
    /* Hold GIC HW information */
    const struct gic_info *info;
    /* Initialize the GIC and the boot CPU */
    int (*init)(void);
    /* Save GIC registers */
    void (*save_state)(struct vcpu *v);
    /* Restore GIC registers */
    void (*restore_state)(const struct vcpu *v);
    /* Dump GIC LR register information */
    void (*dump_state)(const struct vcpu *v);
    /* hw_irq_controller to enable/disable/eoi host irq */
    hw_irq_controller *gic_host_irq_type;
    /* hw_irq_controller to enable/disable/eoi guest irq */
    hw_irq_controller *gic_guest_irq_type;
    /* End of Interrupt */
    void (*eoi_irq)(struct irq_desc *irqd);
    /* Deactivate/reduce priority of irq */
    void (*deactivate_irq)(struct irq_desc *irqd);
    /* Read IRQ ID and Ack */
    unsigned int (*read_irq)(void);
    /* Force the active state of an IRQ by accessing
     * the distributor.
     */
    void (*set_active_state)(struct irq_desc *irqd,
                             bool state);
    /* Force the pending state of an IRQ by accessing
     * the distributor.
     */
    void (*set_pending_state)(struct irq_desc *irqd,
                              bool state);
    /* Set IRQ type */
    void (*set_irq_type)(struct irq_desc *desc,
                         unsigned int type);
    /* Set IRQ priority */
    void (*set_irq_priority)(struct irq_desc *desc,
                             unsigned int priority);
    /* Send SGI */
    void (*send_sgi)(enum gic_sgi sgi,
                     enum gic_sgi_mode irqmode,
                     const cpumask_t *online_mask);
    /* Disable CPU physical and virtual interfaces */
    void (*disable_interface)(void);
    /* Update LR register with state and priority */
    void (*update_lr)(int lr, unsigned int virq,
                      u8 priority, unsigned int hw_irq,
                      unsigned int state);
    /* Update HCR status register */
    void (*update_hcr_status)(u32 flag, bool set);
    /* Clear LR register */
    void (*clear_lr)(int lr);
    /* Read LR register and populate gic_lr structure */
    void (*read_lr)(int lr, struct gic_lr *lr_reg);
    /* Write LR register from gic_lr structure */
    void (*write_lr)(int lr, const struct gic_lr *lr_reg);
    /* Read VMCR priority */
    unsigned int (*read_vmcr_priority)(void);
    /* Read APRn register */
    unsigned int (*read_apr)(int apr_reg);
    /* Query the pending state of an interrupt at the
     * distributor level.
     */
    bool (*read_pending_state)(struct irq_desc *irqd);
    /* Secondary CPU init */
    int (*secondary_init)(void);
    int (*iomem_deny_access)(struct hypos *h);
    /* Handle LPIs, which require special handling */
    void (*do_lpi)(unsigned int lpi);
};

void gic_disable_cpu(void);

extern const struct gic_hw_operations *gic_hw_ops;

static inline unsigned int gic_get_nr_lrs(void)
{
    return gic_hw_ops->info->nr_lrs;
}

/*
 * Set the active state of an IRQ. This should be used with care,
 * as this directly forces the active bit, without considering
 * the GIC state machine.
 * For private IRQs this only works for those of the current CPU.
 * This function should only be called for interrupts routed to
 * the guest. The flow of interrupts routed to Xen is not able
 * cope with software changes to the active state.
 */
static inline void gic_set_active_state(struct irq_desc *irqd,
                                        bool state)
{
    ASSERT(test_bit(_IRQ_GUEST, &irqd->status));
    gic_hw_ops->set_active_state(irqd, state);
}

/*
 * Set the pending state of an IRQ. This should be used with care,
 * as this directly forces the pending bit, without considering the
 * GIC state machine.
 * For private IRQs this only works for those of the current CPU.
 */
static inline void gic_set_pending_state(struct irq_desc *irqd,
                                         bool state)
{
    gic_hw_ops->set_pending_state(irqd, state);
}

/*
 * Read the pending state of an interrupt from the distributor.
 * For private IRQs this only works for those of the current CPU.
 */
static inline bool gic_read_pending_state(struct irq_desc *irqd)
{
    return gic_hw_ops->read_pending_state(irqd);
}

void register_gic_ops(const struct gic_hw_operations *ops);
void gic_route_irq_to_hypos(struct irq_desc *desc,
                            unsigned int priority);

void gic_interrupt(struct hcpu_regs *regs, int is_fiq);
// --------------------------------------------------------------

int  gicv3_lpi_init_host_lpis(unsigned int host_lpi_bits);
void gicv3_set_redist_address(hpa_t address,
                              unsigned int redist_id);
int  gicv3_lpi_init_rdist(void __iomem * rdist_base);
bool gicv3_its_host_has_its(void);
int  gicv3_its_setup_collection(unsigned int cpu);
int  gicv3_its_deny_access(struct hypos *d);
int  gicv3_its_init(void);
void gicv3_do_lpi(unsigned int lpi);
u64  gicv3_get_redist_address(unsigned int cpu, bool use_pta);
void gicv3_free_host_lpi_block(u32 first_lpi);
int  gicv3_ate_host_lpi_block(struct hypos *d, u32 *first_lpi);
void gicv3_lpi_update_host_entry(u32 host_lpi, int hypos_id,
                                 u32 virt_lpi);
int  gic_setup(void);

// --------------------------------------------------------------
#endif /* !__ASSEMBLY__ */

// --------------------------------------------------------------
#endif /* _ORG_GIC_H */
