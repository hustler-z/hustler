/**
 * hustler's project
 *
 * file:  gicv3.h
 * date:  2024/05/22
 * usage:
 */

#ifndef _COMMON_GICV3_H
#define _COMMON_GICV3_H
// --------------------------------------------------------------
#include <common/type.h>

#define GICV3_ROUTE_MODE_ANY          (0x80000000)
#define GICV3_ROUTE_MODE_COORDINATE   (0)
#define GICV3_CONFIG_LEVEL            (0)
#define GICV3_CONFIG_EDGE             (2)
#define GICV3_GROUP0                  (0)
#define GICV3_GROUP1_SECURE           (1)
#define GICV3_GROUP1_NON_SECURE       (2)

/* SGI
 */
#define GICV3_SGI_AFF3_SHIFT          (48)
#define GICV3_SGI_AFF2_SHIFT          (32)
#define GICV3_SGI_AFF1_SHIFT          (16)

#define GICV3_SGI_ROUTING_ALL         ((u64)1 << 40)
#define GICV3_SGI_ROUTING_LIST        (0)

#define GICV3_SGI_TARGET_GENERIC0         (0x0001)
#define GICV3_SGI_TARGET_GENERIC1         (0x0002)
#define GICV3_SGI_TARGET_GENERIC2         (0x0004)
#define GICV3_SGI_TARGET_GENERIC3         (0x0008)
#define GICV3_SGI_TARGET_GENERIC4         (0x0010)
#define GICV3_SGI_TARGET_GENERIC5         (0x0020)
#define GICV3_SGI_TARGET_GENERIC6         (0x0040)
#define GICV3_SGI_TARGET_GENERIC7         (0x0080)
#define GICV3_SGI_TARGET_GENERIC8         (0x0100)
#define GICV3_SGI_TARGET_GENERIC9         (0x0200)
#define GICV3_SGI_TARGET_GENERIC10        (0x0400)
#define GICV3_SGI_TARGET_GENERIC11        (0x0800)
#define GICV3_SGI_TARGET_GENERIC12        (0x1000)
#define GICV3_SGI_TARGET_GENERIC13        (0x2000)
#define GICV3_SGI_TARGET_GENERIC14        (0x4000)
#define GICV3_SGI_TARGET_GENERIC15        (0x8000)

#define GICV3_SGI_ID0                 (0x0 << 24)
#define GICV3_SGI_ID1                 (0x1 << 24)
#define GICV3_SGI_ID2                 (0x2 << 24)
#define GICV3_SGI_ID3                 (0x3 << 24)
#define GICV3_SGI_ID4                 (0x4 << 24)
#define GICV3_SGI_ID5                 (0x5 << 24)
#define GICV3_SGI_ID6                 (0x6 << 24)
#define GICV3_SGI_ID7                 (0x7 << 24)
#define GICV3_SGI_ID8                 (0x8 << 24)
#define GICV3_SGI_ID9                 (0x9 << 24)
#define GICV3_SGI_ID10                (0xA << 24)
#define GICV3_SGI_ID11                (0xB << 24)
#define GICV3_SGI_ID12                (0xC << 24)
#define GICV3_SGI_ID13                (0xD << 24)
#define GICV3_SGI_ID14                (0xE << 24)
#define GICV3_SGI_ID15                (0xF << 24)

#define GICV3_SGI_NO_NS_ACCESS             (0)
#define GICV3_SGI_NS_ACCESS_GROUP0         (0x1)
#define GICV3_SGI_NS_ACCESS_GROUP1         (0x2)

/* LPI
 */
#define GICV3_LPI_INNER_SHARED             (0x1 << 10)
#define GICV3_LPI_OUTER_SHARED             (0x2 << 10)
#define GICV3_LPI_NON_SHARED               (0)

#define GICV3_LPI_NORMAL_WBWA              (((u64)0x5 << 56) | (u64)(0x5 << 7))
#define GICV3_LPI_DEVICE_nGnRnE            (0)
#define GICV3_LPI_ENABLE                   (1)
#define GICV3_LPI_DISABLE                  (0)
#define GICV3_ITS_CQUEUE_VALID             ((u64)1 << 63)
#define GICV3_ITS_CQUEUE_INVALID           (0)
#define GICV3_ITS_TABLE_TYPE_UNIMPLEMENTED (0x0)
#define GICV3_ITS_TABLE_TYPE_DEVICE        (0x1)
#define GICV3_ITS_TABLE_TYPE_VIRTUAL       (0x2)
#define GICV3_ITS_TABLE_TYPE_COLLECTION    (0x4)

#define GICV3_ITS_TABLE_PAGE_SIZE_4K       (0x0)
#define GICV3_ITS_TABLE_PAGE_SIZE_16K      (0x1)
#define GICV3_ITS_TABLE_PAGE_SIZE_64K      (0x2)
#define GICV3_ITS_TABLE_PAGE_VALID         ((u64)1 << 63)
#define GICV3_ITS_TABLE_PAGE_INVALID       (0)

#define GICV3_ITS_TABLE_PAGE_DIRECT        (0)
#define GICV3_ITS_TABLE_PAGE_INDIRECT      (1 << 62)

#define GICV3_ITS_TABLE_PAGE_DEVICE_nGnRnE (0)
#define GICV3_ITS_TABLE_PAGE_NORM_WBWA     (((u64)0x5 << 59) | ((u64)0x5 << 53))

#define GICV3_ITS_TABLE_INNER_SHAREABLE    (0x1 << 10)
#define GICV3_ITS_TABLE_OUTER_SHAREABLE    (0x2 << 10)
#define GICV3_ITS_TABLE_NON_SHAREABLE      (0)
#define GICV3_ITS_PTA_ADDRESS              (1)
#define GICV3_ITS_PTA_ID                   (0)

#define GICV3_v3X                          (30)
#define GICV3_v40                          (40)
#define GICV3_v41                          (41)

struct gicv3_dist_if
{
    __rw u32 gicd_ctlr;              // +0x0000 - rw - distributor control register
    __ro u32 gicd_typer;             // +0x0004 - ro - interrupt controller type register
    __ro u32 gicd_iidr;              // +0x0008 - RO - Distributor Implementer Identification Register
    __ro u32 padding0;               // +0x000C - RESERVED
    __rw u32 gicd_statusr;           // +0x0010 - RW - Status register
    __ro u32 padding1[3];            // +0x0014 - RESERVED
    __rw u32 imp_def[8];             // +0x0020 - RW - Implementation defined registers
    __wo u32 gicd_setspi_nsr;        // +0x0040 - WO - Non-secure Set SPI Pending (Used when SPI is signalled using MSI)
    __ro u32 padding2;               // +0x0044 - RESERVED
    __wo u32 gicd_clrspi_nsr;        // +0x0048 - WO - Non-secure Clear SPI Pending (Used when SPI is signalled using MSI)
    __ro u32 padding3;               // +0x004C - RESERVED
    __wo u32 gicd_setspi_sr;         // +0x0050 - WO - Secure Set SPI Pending (Used when SPI is signalled using MSI)
    __ro u32 padding4;               // +0x0054 - RESERVED
    __wo u32 gicd_clrspi_sr;         // +0x0058 - WO - Secure Clear SPI Pending (Used when SPI is signalled using MSI)
    __ro u32 padding5[3];            // +0x005C - RESERVED
    __wo u32 gicd_seir;              // +0x0068 - WO - System Error Interrupt Register (Note: This was recently removed from the spec)
    __ro u32 padding6[5];            // +0x006C - RESERVED
    __rw u32 gicd_igroupr[32];       // +0x0080 - RW - Interrupt Group Registers (Note: In GICv3, need to look at IGROUPR and IGRPMODR)
    __rw u32 gicd_isenabler[32];     // +0x0100 - RW - Interrupt Set-Enable Registers
    __rw u32 gicd_icenabler[32];     // +0x0180 - RW - Interrupt Clear-Enable Registers
    __rw u32 gicd_ispendr[32];       // +0x0200 - RW - Interrupt Set-Pending Registers
    __rw u32 gicd_icpendr[32];       // +0x0280 - RW - Interrupt Clear-Pending Registers
    __rw u32 gicd_isactiver[32];     // +0x0300 - RW - Interrupt Set-Active Register
    __rw u32 gicd_icactiver[32];     // +0x0380 - RW - Interrupt Clear-Active Register
    __rw u8  gicd_ipriorityr[1024];  // +0x0400 - RW - Interrupt Priority Registers
    __rw u32 gicd_itargetsr[256];    // +0x0800 - RW - Interrupt Processor Targets Registers
    __rw u32 gicd_icfgr[64];         // +0x0C00 - RW - Interrupt Configuration Registers
    __rw u32 gicd_grpmodr[32];       // +0x0D00 - RW - Interrupt Group Modifier (Note: In GICv3, need to look at IGROUPR and IGRPMODR)
    __ro u32 padding7[32];           // +0x0D80 - RESERVED
    __rw u32 gicd_nsacr[64];         // +0x0E00 - RW - Non-secure Access Control Register
    __wo u32 gicd_sgir;              // +0x0F00 - WO - Software Generated Interrupt Register
    __ro u32 padding8[3];            // +0x0F04 - RESERVED
    __rw u32 gicd_cpendsgir[4];      // +0x0F10 - RW - Clear pending for SGIs
    __rw u32 gicd_spendsgir[4];      // +0x0F20 - RW - Set pending for SGIs
    __ro u32 padding9[52];           // +0x0F30 - RESERVED
    // GICv3.1 extended SPI range
    __rw u32 gicd_igroupre[128];     // +0x1000 - RW - Interrupt Group Registers (GICv3.1)
    __rw u32 gicd_isenablere[128];   // +0x1200 - RW - Interrupt Set-Enable Registers (GICv3.1)
    __rw u32 gicd_icenablere[128];   // +0x1400 - RW - Interrupt Clear-Enable Registers (GICv3.1)
    __rw u32 gicd_ispendre[128];     // +0x1600 - RW - Interrupt Set-Pending Registers (GICv3.1)
    __rw u32 gicd_icpendre[128];     // +0x1800 - RW - Interrupt Clear-Pending Registers (GICv3.1)
    __rw u32 gicd_isactivere[128];   // +0x1A00 - RW - Interrupt Set-Active Register (GICv3.1)
    __rw u32 gicd_icactivere[128];   // +0x1C00 - RW - Interrupt Clear-Active Register (GICv3.1)
    __ro u32 padding10[128];         // +0x1E00 - RESERVED
    __rw u8  gicd_ipriorityre[4096]; // +0x2000 - RW - Interrupt Priority Registers (GICv3.1)
    __rw u32 gicd_icfgre[256];       // +0x3000 - RW - Interrupt Configuration Registers (GICv3.1)
    __rw u32 gicd_igrpmodre[128];    // +0x3400 - RW - Interrupt Group Modifier (GICv3.1)
    __rw u32 gicd_nsacre[256];       // +0x3600 - RW - Non-secure Access Control Register (GICv3.1)
    __ro u32 padding11[2432];        // +0x3A00 - RESERVED
    // GICv3.0
    __rw u64 gicd_router[1024];      // +0x6000 - RW - Controls SPI routing when ARE=1
    // GICv3.1
    __rw u64 gicd_routere[1024];     // +0x8000 - RW - Controls SPI routing when ARE=1 (GICv3.1)
};

struct gicv3_rdist_lpis_if
{
    __rw u32 gicr_ctlr;             // +0x0000 - RW - Redistributor Control Register
    __ro u32 gicr_iidr;             // +0x0004 - RO - Redistributor Implementer Identification Register
    __ro u32 gicr_typer[2];         // +0x0008 - RO - Redistributor Type Register
    __rw u32 gicr_statusr;          // +0x0010 - RW - Redistributor Status register
    __rw u32 gicr_waker;            // +0x0014 - RW - Wake Request Registers
    __ro u32 gicr_mpamidr;          // +0x0018 - RO - Reports maximum PARTID and PMG (GICv3.1)
    __rw u32 gicr_partid;           // +0x001C - RW - Set PARTID and PMG used for Redistributor memory accesses (GICv3.1)
    __ro u32 padding1[8];           // +0x0020 - RESERVED
    __wo u64 gicr_setlpir;          // +0x0040 - WO - Set LPI pending (Note: IMP DEF if ITS present)
    __wo u64 gicr_clrlpir;          // +0x0048 - WO - Set LPI pending (Note: IMP DEF if ITS present)
    __ro u32 padding2[6];           // +0x0058 - RESERVED
    __wo u32 gicr_seir;             // +0x0068 - WO - (Note: This was removed from the spec)
    __ro u32 padding3;              // +0x006C - RESERVED
    __rw u64 gicr_propbaser;        // +0x0070 - RW - Sets location of the LPI configuration table
    __rw u64 gicr_pendbaser;        // +0x0078 - RW - Sets location of the LPI pending table
    __ro u32 padding4[8];           // +0x0080 - RESERVED
    __wo u64 gicr_invlpir;          // +0x00A0 - WO - Invalidates cached LPI config (Note: In GICv3.x: IMP DEF if ITS present)
    __ro u32 padding5[2];           // +0x00A8 - RESERVED
    __wo u64 gicr_invallr;          // +0x00B0 - WO - Invalidates cached LPI config (Note: In GICv3.x: IMP DEF if ITS present)
    __ro u32 padding6[2];           // +0x00B8 - RESERVED
    __wo u64 gicr_syncr;            // +0x00C0 - WO - Redistributor Sync
    __ro u32 padding7[2];           // +0x00C8 - RESERVED
    __ro u32 padding8[12];          // +0x00D0 - RESERVED
    __wo u64 gicr_movlpir;          // +0x0100 - WO - IMP DEF
    __ro u32 padding9[2];           // +0x0108 - RESERVED
    __wo u64 gicr_movallr;          // +0x0110 - WO - IMP DEF
    __ro u32 padding10[2];          // +0x0118 - RESERVED
};

struct gicv3_rdist_sgis_if
{
    __ro u32 padding1[32];          // +0x0000 - RESERVED
    __rw u32 gicr_igroupr[3];       // +0x0080 - RW - Interrupt Group Registers (Security Registers in GICv1)
    __ro u32 padding2[29];          // +0x008C - RESERVED
    __rw u32 gicr_isenabler[3];     // +0x0100 - RW - Interrupt Set-Enable Registers
    __ro u32 padding3[29];          // +0x010C - RESERVED
    __rw u32 gicr_icenabler[3];     // +0x0180 - RW - Interrupt Clear-Enable Registers
    __ro u32 padding4[29];          // +0x018C - RESERVED
    __rw u32 gicr_ispendr[3];       // +0x0200 - RW - Interrupt Set-Pending Registers
    __ro u32 padding5[29];          // +0x020C - RESERVED
    __rw u32 gicr_icpendr[3];       // +0x0280 - RW - Interrupt Clear-Pending Registers
    __ro u32 padding6[29];          // +0x028C - RESERVED
    __rw u32 gicr_isactiver[3];     // +0x0300 - RW - Interrupt Set-Active Register
    __ro u32 padding7[29];          // +0x030C - RESERVED
    __rw u32 gicr_icactiver[3];     // +0x0380 - RW - Interrupt Clear-Active Register
    __ro u32 padding8[29];          // +0x018C - RESERVED
    __rw u8  gicr_ipriorityr[96];   // +0x0400 - RW - Interrupt Priority Registers
    __ro u32 padding9[488];         // +0x0460 - RESERVED
    __rw u32 gicr_icfgr[6];         // +0x0C00 - RW - Interrupt Configuration Registers
    __ro u32 padding10[58];         // +0x0C18 - RESERVED
    __rw u32 gicr_igrpmodr[3];      // +0x0D00 - RW - Interrupt Group Modifier Register
    __ro u32 padding11[61];         // +0x0D0C - RESERVED
    __rw u32 gicr_nsacr;            // +0x0E00 - RW - Non-secure Access Control Register
};

struct gicv3_rdist_vlpis_if
{
    __ro u32 padding1[28];          // +0x0000 - RESERVED
    __rw u64 gicr_vpropbaser;       // +0x0070 - RW - Sets location of the LPI vPE Configuration table
    __rw u64 gicr_vpendbaser;       // +0x0078 - RW - Sets location of the LPI Pending table
};

struct gicv3_rdist_res_if
{
    __ro u32 padding1[32];          // +0x0000 - RESERVED
};

struct gicv3_rdist_if
{
    struct gicv3_rdist_lpis_if   lpis  __attribute__((aligned(0x10000)));
    struct gicv3_rdist_sgis_if   sgis  __attribute__((aligned(0x10000)));
    struct gicv3_rdist_vlpis_if  vlpis __attribute__((aligned(0x10000)));
    struct gicv3_rdist_res_if    res   __attribute__((aligned(0x10000)));

};

// +0 from ITS_BASE
struct gicv3_its_ctlr_if
{
    __rw u32 gits_ctlr;             // +0x0000 - RW - ITS Control Register
    __ro u32 gits_iidr;             // +0x0004 - RO - Implementer Identification Register
    __ro u64 gits_typer;            // +0x0008 - RO - ITS Type register
    __ro u32 gits_mpamidr;          // +0x0010 - RO - Reports maxmimum PARTID and PMG (GICv3.1)
    __rw u32 gits_partidr;          // +0x0004 - RW - Sets the PARTID and PMG used for ITS memory accesses (GICv3.1)
    __ro u32 gits_mpidr;            // +0x0018 - RO - ITS affinity, used for shared vPE table
    __ro u32 padding5;              // +0x001C - RESERVED
    __rw u32 gits_impdef[8];        // +0x0020 - RW - IMP DEF registers
    __ro u32 padding2[16];          // +0x0040 - RESERVED
    __rw u64 gits_cbaser;           // +0x0080 - RW - Sets base address of ITS command queue
    __rw u64 gits_cwriter;          // +0x0088 - RW - Points to next enrty to add command
    __rw u64 gits_creadr;           // +0x0090 - RW - Points to command being processed
    __ro u32 padding3[2];           // +0x0098 - RESERVED
    __ro u32 padding4[24];          // +0x00A0 - RESERVED
    __rw u64 gits_baser[8];         // +0x0100 - RW - Sets base address of Device and Collection tables
};

// +0x010000 from ITS_BASE
struct gicv3_its_int_if
{
    __ro u32 padding1[16];          // +0x0000 - RESERVED
    __rw u32 gits_translater;       // +0x0040 - RW - Written by peripherals to generate LPI
};

// +0x020000 from ITS_BASE
struct gicv3_its_sgi_if
{
    __ro u32 padding1[8];           // +0x0000 - RESERVED
    __rw u64 gits_sgir;             // +0x0020 - RW - Written by peripherals to generate vSGI (GICv4.1)
};

// ------------------------------------------------------------------------

/* Basic GICv3 APIs
 */
void set_gic_base(void *dist, void *rdist);
u32 get_ext_ppi(u32 rd);
u32 get_spi(void);
u32 get_ext_spi(void);
u32 enable_gic(void);
u32 get_rdist_id(u32 affinity);
u32 wakeup_rdist(u32 rd);
u32 enable_interrupt(u32 id, u32 rd);
u32 disable_interrupt(u32 id, u32 rd);
u32 set_interrupt_priority(u32 id, u32 rd, u8 priority);
u32 set_interrupt_type(u32 id, u32 rd, u32 type);
u32 set_interrupt_grp(u32 id, u32 rd, u32 security);
u32 set_interrupt_route(u32 id, u32 mode, u32 affinity);
u32 set_interrupt_pending(u32 id, u32 rd);
u32 clear_interrupt_pending(u32 id, u32 rd);

/* LPI APIs
 */
void set_its_base(void* its_base);
u32 set_lpi_config_table_addr(u32 rd, u64 addr, u64 attributes,
        u32 idbits);
u32 set_lpi_pending_table_addr(u32 rd, u64 addr, u64 attributes,
        u32 idbits);
void enable_lpis(u32 rd);
u32 get_rd_proc_nr(u32 rd);
u32 get_max_lpi(u32 rd);
u32 configure_lpi(u32 rd, u32 id, u32 enable, u32 priority);
u32 init_its_cmd_queue(u64 addr, u64 attributes, u32 num_pages);
u64 get_its_table_type(u32 index, u32* type, u32* entry_size);
u32 set_its_table_addr(u32 index, u64 addr, u64 attributes,
        u32 page_size, u32 num_pages);
u32 get_its_pta(void);
void its_add_cmd(u8* command);
void its_mapd(u32 device_id, u64 table, u32 size);
void its_mapc(u32 target, u32 collection);
void its_mapti(u32 device_id, u32 event_id, u32 intid, u32 cid);
void its_invall(u32 cid);
void its_inv(u32 device_id, u32 event_id);
void its_sync(u64 target);
void its_int(u32 device_id, u32 event_id);

int gicv3_setup(void);

// --------------------------------------------------------------
#endif /* _COMMON_GICV3_H */
