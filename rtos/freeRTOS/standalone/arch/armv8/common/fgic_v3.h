/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: fgic_v3.h
 * Date: 2023-10-7 09:30:29
 * LastEditTime: 2023-10-7 09:30:29
 * Description: This file is for detailed description of the device and driver.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2023/10/7   init commit
 */

#ifndef FGIC_V3_H
#define FGIC_V3_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fio.h"
#include "faarch.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FGIC_RSGI_AFF1_OFFSET 16
#define FGIC_RSGI_AFF2_OFFSET 32
#define FGIC_RSGI_AFF3_OFFSET 48

#define FGIC_SUCCESS            FT_SUCCESS
#define FGIC_CTLR_ERR_TYPE      FT_MAKE_ERRCODE(ErrModBsp, ErrGic, 1) /* 错误选择CTLR 寄存器 */
#define FGIC_CTLR_ERR_NUM       FT_MAKE_ERRCODE(ErrModBsp, ErrGic, 2) /* 当前控制器不支持此中断id */
#define FGIC_CTLR_ERR_IN_SET    FT_MAKE_ERRCODE(ErrModBsp, ErrGic, 3) /* 在设置过程中出现的异常 */
#define FGIC_CTLR_ERR_IN_GET    FT_MAKE_ERRCODE(ErrModBsp, ErrGic, 4) /* 在获取过程中出现的异常 */
#define FGIC_ERR_IN_TIMEOUT     FT_MAKE_ERRCODE(ErrModBsp, ErrGic, 5)  /* 超时退出 */

/**************************** Reg R/W Definitions *******************************/

#define FGIC_READREG8(addr, reg_offset) FtIn8(addr + (u8)reg_offset)
#define FGIC_WRITEREG8(addr, reg_offset, reg_value) FtOut8(addr + (u8)reg_offset, (u8)reg_value)

#define FGIC_READREG32(addr, reg_offset) FtIn32(addr + (u32)reg_offset)
#define FGIC_WRITEREG32(addr, reg_offset, reg_value) FtOut32(addr + (u32)reg_offset, (u32)reg_value)

#ifdef __aarch64__

#define FGIC_READREG64(addr, reg_offset) FtIn64(addr + (u64)reg_offset)
#define FGIC_WRITEREG64(addr, reg_offset, reg_value) FtOut64(addr +(u64)reg_offset, (u64)reg_value)

#else

#define FGIC_READREG64(addr, reg_offset)                            \
({                                                                  \
        u64 reg_val;                                                \
        reg_val = FtIn32(addr + (u32)reg_offset + 4);               \
        reg_val = (reg_val << 32) | FtIn32(addr + (u32)reg_offset); \
        reg_val;                                                    \
})
            
#define FGIC_WRITEREG64(addr, reg_offset, reg_value)                \
({                                                                  \
        FtOut32(addr + (u32)reg_offset, (u32)reg_value);            \
        FtOut32(addr + (u32)reg_offset + 4, (u32)(((u64)reg_value) >> 32));  \
})

#endif 


#define FGIC_SETBIT(base_addr, reg_offset, data) \
    FtSetBit32((base_addr) + (u32)(reg_offset), (u32)(data))

#define FGIC_CLEARBIT(base_addr, reg_offset, data) \
    FtClearBit32((base_addr) + (u32)(reg_offset), (u32)(data))


/**************************** Type Definitions *******************************/

typedef enum
{
    TRIGGER_BY_LEVEL_SENSITIVE = 0,  /* Corresponding interrupt is level-sensitive. */
    TRIGGER_BY_LEVEL_EDGE,           /* Corresponding interrupt is edge-triggered. */
} TRIGGER_LEVEL;

typedef enum
{
    TWO_SECURITY_STATE = 0,          
    ONE_NS_SECURITY_STATE,           
} SECURITY_STATE;

/*  Interrupt Routing Mode. */
typedef enum
{
    SGI_ROUTING_TO_SPECIFIC = 0,        /* sgi interrupts routed to the PEs specified by affinity level. */
    SGI_ROUTING_TO_ANY = (1ULL << 40)   /* sgi interrupts routed to all PEs in the system, excluding "self". */
} SGI_ROUTING_MODE;

typedef enum
{
    SPI_ROUTING_TO_SPECIFIC = 0,  /* spi interrupts routed to the PE specified by affinity level. */
    SPI_ROUTING_TO_ANY = (1U << 31) /* spi interrupts routed to any PE defined as a participating node. */
} SPI_ROUTING_MODE;

typedef struct
{
    u32 instance_id; /* Id of device*/
    uintptr dis_base; /* Distributor base address      */

} FGicConfig;

typedef struct
{
    FGicConfig config; /* Configuration data structure  */
    u32 is_ready;       /* Device is ininitialized and ready*/
    uintptr redis_base; /* Redistributor base address for each core  */
    SECURITY_STATE security ;
    s32 max_spi_num;    /* Max value of spi priority */
} FGic;

/* Initialization */
FGicConfig *FGicLookupConfig(u32 instance_id);
FError FGicCfgInitialize(FGic *instance_p, const FGicConfig *input_config_p, uintptr redis_base);
uintptr FGicRedistrubutiorIterate(void);
void FGicDistrubutiorInit(FGic *instance_p);
FError FGicRedistrubutiorInit(FGic *instance_p);
void FGicCpuInterfaceInit(void);

/* Operation interface */
FError FGicIntEnable(FGic *instance_p, s32 int_id);
FError FGicIntDisable(FGic *instance_p, s32 int_id);
FError FGicSetPriority(FGic *instance_p, s32 int_id, u32 priority);
u32 FGicGetPriority(FGic *instance_p, s32 int_id);
FError FGicSetTriggerLevel(FGic *instance_p, s32 int_id, TRIGGER_LEVEL trigger_way);
u32 FGicGetTriggerLevel(FGic *instance_p, s32 int_id);
FError FGicSetSpiAffinityRouting(FGic *instance_p, s32 int_id, SPI_ROUTING_MODE route_mode, u64 affinity);
FError FGicGetAffinityRouting(FGic *instance_p, s32 int_id, SPI_ROUTING_MODE *route_mode_p, u64 *affinity_p);
FError FGicGenerateSgi(FGic *instance_p, s32 int_id, u32 target_list, SGI_ROUTING_MODE routing_mode, u64 affinity);
void FGicDeactionInterrupt(FGic *instance_p, s32 int_id);
s32 FGicAcknowledgeInt(FGic *instance_p);
void FGicSetPriorityFilter(FGic *instance_p, u32 priority_mask);
void FGicSetPriorityGroup(FGic *instance_p, u32 binary_point);
u32 FGicGetPriorityFilter(FGic *instance_p);
u32 FGicGetPriorityGroup(FGic *instance_p);


/***************************** cpu interface *********************************/

#define FGIC_ICC_SGI1R_INTID_MASK 0xFULL  /* The INTID of the SGI. */

#define GICC_SGIR_IRM_BITS SGI_ROUTING_MODE

typedef enum
{
    GICC_SRE_SRE = (1 << 0),
    GICC_SRE_DFB = (1 << 1),
    GICC_SRE_DIB = (1 << 2),
    GICC_SRE_ENABLE = (1 << 3)
} GICC_SRE_BITS;

typedef enum
{
    GICC_CTLR_CBPR = (1 << 0),
    GICC_CTLR_CBPR_EL1S = (1 << 0),
    GICC_CTLR_EOIMODE = (1 << 1),
    GICC_CTLR_CBPR_EL1NS = (1 << 1),
    GICC_CTLR_EOIMODE_EL3 = (1 << 2),
    GICC_CTLR_EOIMODE_EL1S = (1 << 3),
    GICC_CTLR_EOIMODE_EL1NS = (1 << 4), /* GICC_EOIR and GICC_AEOIR provide priority drop functionality only. GICC_DIR provides interrupt deactivation functionality.  */
    GICC_CTLR_RM = (1 << 5),
    GICC_CTLR_PMHE = (1 << 6)
} GICC_CTLR_BITS;


#ifdef __aarch64__

#define ICC_SRE_EL1 "S3_0_C12_C12_5"
#define ICC_SRE_EL2 "S3_4_C12_C9_5"
#define ICC_SRE_EL3 "S3_6_C12_C12_5"

#define ICC_IGRPEN0_EL1 "S3_0_C12_C12_6"
#define ICC_IGRPEN1_EL1 "S3_0_C12_C12_7"
#define ICC_IGRPEN1_EL3 "S3_6_C12_C12_7"

#define ICC_CTLR_EL1 "S3_0_C12_C12_4"
#define ICC_CTLR_EL3 "S3_6_C12_C12_4"

#define ICC_IAR0_EL1 "S3_0_C12_C8_0"
#define ICC_IAR1_EL1 "S3_0_C12_C12_0"

#define ICC_EOIR0_EL1 "S3_0_C12_C8_1"
#define ICC_EOIR1_EL1 "S3_0_C12_C12_1"

#define ICC_DIR_EL1 "S3_0_C12_C11_1"

#define ICC_BPR0_EL1 "S3_0_C12_C8_3"
#define ICC_BPR1_EL1 "S3_0_C12_C12_3"

#define ICC_HPPIR0_EL1 "S3_0_C12_C8_2"
#define ICC_HPPIR1_EL1 "S3_0_C12_C12_2"

#define ICC_PMR_EL1 "S3_0_C4_C6_0"
#define ICC_RPR_EL1 "S3_0_C12_C11_3"

#define ICC_SGI0R_EL1 "S3_0_C12_C11_7"
#define ICC_SGI1R_EL1 "S3_0_C12_C11_5"
#define ICC_ASGI1R_EL1 "S3_0_C12_C11_6"

#define AARCH64_READ_GIC_SYSREG(reg) (          \
{                                       \
    uint64_t val;                       \
    __asm__ __volatile__("mrs %0," reg \
                         : "=r"(val));  \
    val;                                \
})

#define AARCH64_WRITE_GIC_SYSREG(reg, val) (           \
{                                              \
    __asm__ __volatile__("msr " reg ",%0" \
                         :                 \
                         : "r"(val));      \
})

/* Interrupt Controller System Enable Register */
static inline void FGicSetICC_SRE_EL3(GICC_SRE_BITS bits)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_SRE_EL3, bits);
    ISB();
}

static inline void FGicSetICC_SRE_EL2(GICC_SRE_BITS bits)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_SRE_EL2, bits);
    ISB();
}

static inline void FGicSetICC_SRE_EL1(GICC_SRE_BITS bits)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_SRE_EL1, bits);
    ISB();
}

static inline u32 FGicGetICC_SRE_EL3(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_SRE_EL3);
    return reg_val;
}

static inline u32 FGicGetICC_SRE_EL2(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_SRE_EL2);
    return reg_val;
}

static inline u32 FGicGetICC_SRE_EL1(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_SRE_EL1);
    return reg_val;
}

static inline void FGicEnableGroup0(void)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_IGRPEN0_EL1, 1);
    ISB();
}

static inline void FGicDisableGroup0(void)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_IGRPEN0_EL1, 0);
    ISB();
}

static inline void FGicEnableGroup1_EL1(void)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_IGRPEN1_EL1, 1);
    ISB();  
}

static inline void FGicDisableGroup1_EL1(void)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_IGRPEN1_EL1, 0);
    ISB();  
}

static inline void FGicEnableGroup1_EL3(void)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_IGRPEN1_EL3, 1);
    ISB();
}

static inline void FGicDisableGroup1_EL3(void)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_IGRPEN1_EL3, 0);
    ISB();
}

static inline void FGicSetICC_CTLR_EL1(GICC_CTLR_BITS reg_bits)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_CTLR_EL1, reg_bits);
    ISB();
}

static inline void FGicSetICC_CTLR_EL3(GICC_CTLR_BITS reg_bits)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_CTLR_EL3, reg_bits);
    ISB();
}

static inline u32 FGicGetICC_CTLR_EL1(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_CTLR_EL1);
    return reg_val;
}

/*Controls aspects of the behavior of the GIC CPU interface and provides information about the features implemented.*/
static inline u32 FGicGetICC_CTLR_EL3(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_CTLR_EL3);
    return reg_val;
}

/*The PE reads this register to obtain the INTID of the signaled Group 0 interrupt.*/
static inline u32 FGicGetICC_IAR0(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_IAR0_EL1);
    return reg_val;
}

/*The PE reads this register to obtain the INTID of the signaled Group 1 interrupt.*/
static inline u32 FGicGetICC_IAR1(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_IAR1_EL1);
    return reg_val;
}

/*A PE writes to this register to inform the CPU interface that it has completed the processing of the specified Group 0 interrupt*/
static inline void FGicSetICC_EOIR0(u32 int_num)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_EOIR0_EL1, int_num);
    ISB();
}

/*A PE writes to this register to inform the CPU interface that it has completed the processing of the specified Group 1 interrupt*/
static inline void FGicSetICC_EOIR1(u32 int_num)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_EOIR1_EL1, int_num);
    ISB();
}

/*When interrupt priority drop is separated from interrupt deactivation, a write to this register deactivates the specified interrupt.*/
static inline void FGicSetICC_DIR(u32 int_num)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_DIR_EL1, int_num);
    ISB();
}

/*Defines the point at which the priority value fields split into two parts, the group priority field and the subpriority field. The group priority field determines Group 0 interrupt preemption.*/
static inline void FGicSetICC_BPR0(u32 binary_point)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_BPR0_EL1, binary_point);
    ISB();
}

static inline u32 FGicGetICC_BPR0(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_BPR0_EL1);
    return reg_val;
}

/*Defines the point at which the priority value fields split into two parts, the group priority field and the subpriority field. The group priority field determines Group 1 interrupt preemption.*/
static inline void FGicSetICC_BPR1(u32 binary_point)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_BPR1_EL1, binary_point);
    ISB();
}

static inline u32 FGicGetICC_BPR1(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_BPR1_EL1);
    return reg_val;
}

/*Indicates the highest priority pending Group 0 interrupt on the CPU interface.*/
static inline void FGicSetICC_HPPIR0(u32 binary_point)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_HPPIR0_EL1, binary_point);
    ISB();
}

static inline u32 FGicGetICC_HPPIR0(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_HPPIR0_EL1);
    return reg_val;
}

/*Indicates the highest priority pending Group 1 interrupt on the CPU interface.*/
static inline void FGicSetICC_HPPIR1(u32 binary_point)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_HPPIR1_EL1, binary_point);
    ISB();
}

static inline u32 FGicGetICC_HPPIR1(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_HPPIR1_EL1);
    return reg_val;
}

/*Provides an interrupt priority filter.*/
static inline void FGicSetICC_PMR(u32 priority_mask)
{
    AARCH64_WRITE_GIC_SYSREG(ICC_PMR_EL1, priority_mask);
    ISB();
}

static inline u32 FGicGetICC_PMR(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_PMR_EL1);
    return reg_val;
}


/*Indicates the Running priority of the CPU interface.*/
static inline u32 FGicGetICC_RPR(void)
{
    u32 reg_val;
    reg_val = AARCH64_READ_GIC_SYSREG(ICC_RPR_EL1);
    return reg_val;
}

/* SGI interface */
static inline void FGicSetICC_SGI0R(u32 int_num, u32 target_list, GICC_SGIR_IRM_BITS irm_bit, u64 affinity_list)
{
    u64 sgi_val;
    sgi_val = int_num; /* The INTID of the SGI. */
    sgi_val |= affinity_list ; /* Aff3.Aff2.Aff1 */
    sgi_val |= irm_bit ; /* Interrupt Routing Mode. */
    sgi_val |= target_list; /* Target List. The set of PEs for which SGI interrupts will be generated. */

    AARCH64_WRITE_GIC_SYSREG(ICC_SGI0R_EL1, sgi_val);
    ISB();
}

static inline void FGicSetICC_SGI1R(u32 int_num, u32 target_list, GICC_SGIR_IRM_BITS irm_bit, u64 affinity_list)
{
    u64 sgi_val;
    sgi_val = int_num; /* The INTID of the SGI. */
    sgi_val |= affinity_list ; /* Aff3.Aff2.Aff1 */
    sgi_val |= irm_bit ; /* Interrupt Routing Mode. */
    sgi_val |= target_list; /* Target List. The set of PEs for which SGI interrupts will be generated. */

    AARCH64_WRITE_GIC_SYSREG(ICC_SGI1R_EL1, sgi_val);
    ISB();
}

static inline void FGicSetICC_ASGI1R(u32 int_num, u32 target_list, GICC_SGIR_IRM_BITS irm_bit, u64 affinity_list)
{
    u64 sgi_val;
    sgi_val = int_num; /* The INTID of the SGI. */
    sgi_val |= affinity_list ; /* Aff3.Aff2.Aff1 */
    sgi_val |= irm_bit ; /* Interrupt Routing Mode. */
    sgi_val |= target_list; /* Target List. The set of PEs for which SGI interrupts will be generated. */

    AARCH64_WRITE_GIC_SYSREG(ICC_ASGI1R_EL1, sgi_val);
    ISB();
}

#else /* aarch32 */

/* GICv3 CPU Interface system register defines. The format is: coproc, opc1, CRn, CRm, opc2 */
#define ICC_BPR0	15, 0, 12, 8, 3     /* Interrupt Controller Binary Point Register 0*/
#define ICC_BPR1	15, 0, 12, 12, 3    /* Interrupt Controller Binary Point Register 1*/
#define ICC_CTLR	15, 0, 12, 12, 4    /* Interrupt Controller Control Register*/
#define ICC_DIR		15, 0, 12, 11, 1    /* Interrupt Controller Deactivate Interrupt Register*/
#define ICC_EOIR0	15, 0, 12, 8, 1     /* Interrupt Controller End Of Interrupt Register 0*/
#define ICC_EOIR1	15, 0, 12, 12, 1    /* Interrupt Controller End Of Interrupt Register 1*/
#define ICC_HPPIR0	15, 0, 12, 8, 2     /* Interrupt Controller Highest Priority Pending Interrupt Register 0*/
#define ICC_HPPIR1	15, 0, 12, 12, 2    /* Interrupt Controller Highest Priority Pending Interrupt Register 1*/
#define ICC_HSRE	15, 4, 12, 9, 5     /* Interrupt Controller Hyp System Register Enable register*/
#define ICC_IAR0	15, 0, 12, 8, 0     /* Interrupt Controller Interrupt Acknowledge Register 0*/
#define ICC_IAR1	15, 0, 12, 12, 0    /* Interrupt Controller Interrupt Acknowledge Register 1*/
#define ICC_IGRPEN0	15, 0, 12, 12, 6    /* Interrupt Controller Interrupt Group 0 Enable register*/
#define ICC_IGRPEN1	15, 0, 12, 12, 7    /* Interrupt Controller Interrupt Group 1 Enable register*/
#define ICC_MCTLR	15, 6, 12, 12, 4    /* Interrupt Controller Monitor Control Register*/
#define ICC_MGRPEN1	15, 6, 12, 12, 7    /* Interrupt Controller Monitor Interrupt Group 1 Enable register*/
#define ICC_MSRE	15, 6, 12, 12, 5    /* Interrupt Controller Monitor System Register Enable register*/
#define ICC_PMR		15, 0, 4, 6, 0      /* Interrupt Controller Interrupt Priority Mask Register*/
#define ICC_RPR		15, 0, 12, 11, 3    /* Interrupt Controller Running Priority Register*/
#define ICC_SRE		15, 0, 12, 12, 5    /* Interrupt Controller System Register Enable register*/

/* GICv3 CPU Interface system 64 bit register defines. The format is: coproc, opc1, CRm */
#define ICC_ASGI1R_64	15, 1, 12   /* Interrupt Controller Alias Software Generated Interrupt Group 1 Register*/
#define ICC_SGI0R_64	15, 2, 12   /* Interrupt Controller Software Generated Interrupt Group 0 Register*/
#define ICC_SGI1R_64	15, 0, 12   /* Interrupt Controller Software Generated Interrupt Group 1 Register*/

#define FGicSetICC_SRE_EL1 FGicSetICC_SRE
#define FGicSetICC_SRE_EL2 FGicSetICC_SRE
#define FGicSetICC_SRE_EL3 FGicSetICC_SRE

/**
 * @name:
 * @msg: Interrupt Controller System Register Enable
 * @return {*}
 */
static inline void FGicSetICC_SRE(GICC_SRE_BITS bits)
{
    AARCH32_WRITE_SYSREG_32(ICC_SRE, bits);
    ISB();
}

#define FGicGetICC_SRE_EL3 FGicGetICC_SRE
#define FGicGetICC_SRE_EL2 FGicGetICC_SRE
#define FGicGetICC_SRE_EL1 FGicGetICC_SRE

/**
 * @name:
 * @msg: Interrupt Controller System Register Enable
 * @return {*}
 */
static inline u32 FGicGetICC_SRE(void)
{
    u32 value;
    value = AARCH32_READ_SYSREG_32(ICC_SRE);
    return value;
}


/**
 * @name:
 * @msg: void FGicEnableGroup0(void)
 * @return {*}
 */
static inline void FGicEnableGroup0(void)
{
    AARCH32_WRITE_SYSREG_32(ICC_IGRPEN0, 1);
    ISB();
}

/**
 * @name:
 * @msg: void FGicDisableGroup0(void)
 * @return {*}
 */
static inline void FGicDisableGroup0(void)
{
    AARCH32_WRITE_SYSREG_32(ICC_IGRPEN0, 0);
    ISB();    
}


#define FGicEnableGroup1_EL3 FGicEnableGroup1
#define FGicEnableGroup1_EL1 FGicEnableGroup1

/**
 * @name:
 * @msg:
 * @return {*}
 */
static inline void FGicEnableGroup1(void)
{
    AARCH32_WRITE_SYSREG_32(ICC_IGRPEN1, 1);
    ISB();
}

#define FGicDisableGroup1_EL3 FGicDisableGroup1
#define FGicDisableGroup1_EL1 FGicDisableGroup1


static inline void FGicDisableGroup1(void)
{
    AARCH32_WRITE_SYSREG_32(ICC_IGRPEN1, 0);
    ISB();
}

#define FGicSetICC_CTLR_EL3 FGicSetICC_CTLR
#define FGicSetICC_CTLR_EL1 FGicSetICC_CTLR

static inline void FGicSetICC_CTLR(GICC_CTLR_BITS bits)
{
    AARCH32_WRITE_SYSREG_32(ICC_CTLR, bits);
    ISB();
}

#define FGicGetICC_CTLR_EL3 FGicGetICC_CTLR
#define FGicGetICC_CTLR_EL1 FGicGetICC_CTLR

/**
 * @name:
 * @msg: Controls aspects of the behavior of the GIC CPU interface and provides information about the features implemented.
 * @return {*}
 */
static inline u32 FGicGetICC_CTLR(void)
{
    u32 reg;
    reg = AARCH32_READ_SYSREG_32(ICC_CTLR);
    return reg;
}

/**
 * @name:
 * @msg: The PE reads this register to obtain the INTID of the signaled Group 0 interrupt.
 * @return {*}
 */
static inline u32 FGicGetICC_IAR0(void)
{
    u32 reg;
    reg = AARCH32_READ_SYSREG_32(ICC_IAR0);
    return reg;
}

/**
 * @name:
 * @msg: The PE reads this register to obtain the INTID of the signaled Group 1 interrupt.
 * @return {*}
 */
static inline u32 FGicGetICC_IAR1(void)
{
    u32 reg;
    reg = AARCH32_READ_SYSREG_32(ICC_IAR1);
    return reg;
}

/**
 * @name:
 * @msg: A PE writes to this register to inform the CPU interface that it has completed the processing of the specified Group 0 interrupt
 * @return {*}
 * @param {u32} int_num
 */
static inline void FGicSetICC_EOIR0(u32 int_num)
{
    AARCH32_WRITE_SYSREG_32(ICC_EOIR0, int_num);
    ISB();  
}

/**
 * @name:
 * @msg: A PE writes to this register to inform the CPU interface that it has completed the processing of the specified Group 1 interrupt
 * @return {*}
 */
static inline void FGicSetICC_EOIR1(u32 int_num)
{
    AARCH32_WRITE_SYSREG_32(ICC_EOIR1, int_num);
    ISB();  
}

/**
 * @name:
 * @msg: When interrupt priority drop is separated from interrupt deactivation, a write to this register deactivates the specified interrupt.
 * @return {*}
 */
static inline void FGicSetICC_DIR(u32 int_num)
{
    AARCH32_WRITE_SYSREG_32(ICC_DIR, int_num);
    ISB();                 
}

/**
 * @name:
 * @msg: Provides an interrupt priority filter.
 * @param {u32} priority_mask is level for the CPU interface. If the priority of an interrupt is higher than the value
 * indicated by this field, the interface signals the interrupt to the PE.
 */
static inline void FGicSetICC_PMR(u32 priority_mask)
{
    AARCH32_WRITE_SYSREG_32(ICC_PMR, priority_mask);
    ISB();                 
}

/**
 * @name:
 * @msg: static inline u32 FGicGetICC_PMR(void)
 * @return {*}
 */
static inline u32 FGicGetICC_PMR(void)
{
    u32 reg;
    reg = AARCH32_READ_SYSREG_32(ICC_PMR);
    return reg;
}

/**
 * @name:
 * @msg:  Defines the point at which the priority value fields split into two parts, the group priority field and the subpriority field. The group priority field determines Group 1 interrupt preemption.
 * @return {*}
 */
static inline u32 FGicGetICC_BPR1(void)
{
    u32 reg;
    reg = AARCH32_READ_SYSREG_32(ICC_BPR1);
    return reg;
}

/**
 * @name:
 * @msg:  static inline void FGicSetICC_BPR1(u32 binary_point)
 * @return {*}
 */
static inline void FGicSetICC_BPR1(u32 binary_point)
{
    AARCH32_WRITE_SYSREG_32(ICC_BPR1, binary_point);
    ISB();
}

/**
 * @name:
 * @msg:  Defines the point at which the priority value fields split into two parts, the group priority field and the subpriority field. The group priority field determines Group 0 interrupt preemption.
 * @return {*}
 */
static inline u32 FGicGetICC_BPR0(void)
{
    u32 reg;
    reg = AARCH32_READ_SYSREG_32(ICC_BPR0);
    return reg;
}

/**
 * @name:
 * @msg:  static inline void FGicSetICC_BPR0(u32 binary_point)
 * @return {*}
 */
static inline void FGicSetICC_BPR0(u32 binary_point)
{
    AARCH32_WRITE_SYSREG_32(ICC_BPR0, binary_point);
    ISB();              
}

/**
 * @name:
 * @msg:  Indicates the highest priority pending Group 1 interrupt on the CPU interface.
 * @return {*}
 */
static inline u32 FGicGetICC_HPPIR1(void)
{
    u32 reg;
    reg = AARCH32_READ_SYSREG_32(ICC_HPPIR1);
    return reg;
}


/**
 * @name:
 * @msg:  static inline void FGicSetICC_HPPIR1(u32 binary_point)
 * @return {*}
 */
static inline void FGicSetICC_HPPIR1(u32 binary_point)
{
    AARCH32_WRITE_SYSREG_32(ICC_HPPIR1, binary_point);
    ISB();
}

/**
 * @name:
 * @msg:  Indicates the highest priority pending Group 0 interrupt on the CPU interface.
 * @return {*}
 */
static inline u32 FGicGetICC_HPPIR0(void)
{
    u32 reg;
    reg = AARCH32_READ_SYSREG_32(ICC_HPPIR0);
    return reg;
}


/**
 * @name:
 * @msg:  static inline void FGicSetICC_HPPIR0(u32 binary_point)
 * @return {*}
 */
static inline void FGicSetICC_HPPIR0(u32 binary_point)
{
    AARCH32_WRITE_SYSREG_32(ICC_HPPIR0, binary_point);
    ISB();            
}

/**
 * @name:
 * @msg:  static inline u32 FGicGetICC_RPR(void) --- Indicates the Running priority of the CPU interface.
 * @return {*}
 */
static inline u32 FGicGetICC_RPR(void)
{
    u32 reg;
    reg = AARCH32_READ_SYSREG_32(ICC_RPR);
    return reg;
}

/* SGI interface */
/**
 * @name:
 * @msg: Generates Secure Group 0 SGIs
 * @return {*}
 * @param {u32} int_num_bit
 * @param {u32} target_list
 * @param {GICC_SGIR_IRM_BITS} irm_bit
 * @param {u64} affinity_list
 */
static inline void FGicSetICC_SGI0R(u32 int_num_bit, u32 target_list, GICC_SGIR_IRM_BITS irm_bit, u64 affinity_list)
{
    u64 sgi_val;
    sgi_val = int_num_bit; /* The INTID of the SGI. */
    sgi_val |= affinity_list ; /* Aff3.Aff2.Aff1 */
    sgi_val |= irm_bit ; /* Interrupt Routing Mode. */
    sgi_val |= target_list; /* Target List. The set of PEs for which SGI interrupts will be generated. */

    DSB();
    AARCH32_WRITE_SYSREG_64(ICC_SGI0R_64, sgi_val);
    ISB();
}

/**
 * @name:
 * @msg: static inline void FGicSetICC_SGI1R(u32 int_num,u32 target_list,GICC_SGIR_IRM_BITS mode,u64 affinity_list)
 * @return {*}
 */
static inline void FGicSetICC_SGI1R(u32 intnum_bit, u32 target_list, GICC_SGIR_IRM_BITS irm_bit, u64 affinity_list)
{
    u64 sgi_val;
    sgi_val = intnum_bit; /* The INTID of the SGI. */
    sgi_val |= affinity_list ; /* Aff3.Aff2.Aff1 */
    sgi_val |= irm_bit ; /* Interrupt Routing Mode. */
    sgi_val |= target_list; /* Target List. The set of PEs for which SGI interrupts will be generated. */

    DSB();
    AARCH32_WRITE_SYSREG_64(ICC_SGI1R_64, sgi_val);
    ISB();
}


/**
 * @name:
 * @msg: static inline void FGicSetICC_ASGI1R(u32 int_num,u32 target_list,GICC_SGIR_IRM_BITS mode,u64 affinity_list)
 * @return {*}
 */
static inline void FGicSetICC_ASGI1R(u32 intnum_bit, u32 target_list, GICC_SGIR_IRM_BITS irm_bit, u64 affinity_list)
{
    u64 sgi_val;
    sgi_val = intnum_bit; /* The INTID of the SGI. */
    sgi_val |= affinity_list ; /* Aff3.Aff2.Aff1 */
    sgi_val |= irm_bit ; /* Interrupt Routing Mode. */
    sgi_val |= target_list; /* Target List. The set of PEs for which SGI interrupts will be generated. */

    DSB();
    AARCH32_WRITE_SYSREG_64(ICC_ASGI1R_64, sgi_val);
    ISB();
}

#endif

/***************************** distributor *********************************/

#define FGIC_GICD_CTLR_OFFSET 0x00000000U       /* Distributor Control Register ,RW */
#define FGIC_GICD_TYPER_OFFSET 0x00000004U      /* Interrupt Controller Type Register ,RO */
#define FGIC_GICD_IIDR_OFFSET 0x00000008U       /* Distributor Implementer Identification Register ,RO */
#define FGIC_GICD_TYPER2_OFFSET 0x0000000CU     /* Interrupt controller Type Register 2,RO */
#define FGIC_GICD_STATUSR_OFFSET 0x00000010U    /* Error Reporting Status Register, optional ,RW */
#define FGIC_GICD_SETSPI_NSR_OFFSET 0x00000040U /* Set SPI Register ,WO */
#define FGIC_GICD_CLRSPI_NSR_OFFSET 0x00000048U /*  Clear SPI Register ,WO */
#define FGIC_GICD_SETSPI_SR_OFFSET 0x00000050U  /*  Set SPI, Secure Register ,WO */
#define FGIC_GICD_CLRSPI_SR_OFFSET 0x00000058U  /*  Clear SPI, Secure Register ,WO */
#define FGIC_GICD_IGROUPR_OFFSET 0x00000080U    /* Interrupt Group Registers ,RW */

#define FGIC_GICD_ISENABLER_OFFSET 0x00000100U  /* Interrupt Set-Enable Registers ,RW */
#define FGIC_GICD_ICENABLER_OFFSET 0x00000180U  /* Interrupt Clear-Enable Registers ,RW */
#define FGIC_GICD_ISPENDR_OFFSET 0x00000200U    /* Interrupt Set-Pending Registers ,RW */
#define FGIC_GICD_ICPENDR_OFFSET 0x00000280U    /* Interrupt Clear-Pending Registers ,RW */
#define FGIC_GICD_ISACTIVER_OFFSET 0x00000300U  /* Interrupt Set-Active Registers ,RW */
#define FGIC_GICD_ICACTIVER_OFFSET 0x00000380U  /* Interrupt Clear-Active Registers ,RW */
#define FGIC_GICD_IPRIORITYR_OFFSET 0x00000400U /* Interrupt Priority Registers ,RW */
#define FGIC_GICD_ITARGETSR_OFFSET 0x00000800U  /* Interrupt Processor Targets Registers ,RO */

#define FGIC_GICD_ICFGR_OFFSET 0x00000C00U    /* Interrupt Configuration Registers ,RW */
#define FGIC_GICD_IGRPMODR_OFFSET 0x00000D00U /* Interrupt Group Modifier Registers */

#define FGIC_GICD_NSACR_OFFSET 0x00000E00U     /* Non-secure Access Control ,RW */
#define FGIC_GICD_SGIR_OFFSET 0x00000F00U      /* Software Generated Interrupt Register ,WO */
#define FGIC_GICD_CPENDSGIR_OFFSET 0x00000F10U /* SGI Clear-Pending Registers ,RW */
#define FGIC_GICD_SPENDSGIR_OFFSET 0x00000F20U /* SGI Set-Pending Registers ,RW */

#define FGIC_GICD_IGROUPR_E_OFFSET 0x00001000U /* Interrupt Group Registers for extended SPI ,RW */

#define FGIC_GICD_ISENABLER_E_OFFSET 0x00001200U /* Interrupt Set-Enable for extended SPI ,RW */

#define FGIC_GICD_ICENABLER_E_OFFSET 0x00001400U /* Interrupt Clear-Enable for extended SPI ,RW */

#define FGIC_GICD_ISPENDR_E_OFFSET 0x00001600U /* Interrupt Set-Pend for extended SPI range ,RW */
#define FGIC_GICD_ICPENDR_E_OFFSET 0x00001800U /* Interrupt Clear-Pend for extended SPI ,RW */

#define FGIC_GICD_ISACTIVER_E_OFFSET 0x00001A00U /* Interrupt Set-Active for extended SPI ,RW */

#define FGIC_GICD_ICACTIVER_E_OFFSET 0x00001C00U /* Interrupt Clear-Active for extended SPI ,RW */

#define FGIC_GICD_IPRIORITYR_E_OFFSET 0x00002000U /* Interrupt Priority for extended SPI range ,RW */
#define FGIC_GICD_ICFGR_E_OFFSET 0x00003000U      /* Extended SPI Configuration Register ,RW */
#define FGIC_GICD_IGRPMODR_E_OFFSET 0x00003400U   /* Interrupt Group Modifier for extended SPI ,RW */
#define FGIC_GICD_NSACR_E_OFFSET 0x00003600U      /* Non-secure Access Control Registers for extended SPI range ,RW */

#define FGIC_GICD_IROUTER_OFFSET 0x00006000U   /* Interrupt Routing Registers ,RW ,The offset of the GICD_IROUTER<n> register is 0x6000 + 8n. */
#define FGIC_GICD_IROUTER_E_OFFSET 0x00008000U /* Interrupt Routing Registers for extended SPI range ,RW */


/* FGIC_GICD_CTLR_OFFSET --- Distributor switch */
#define FGIC_GICD_CTLR_RWP_MASK BIT(31)
#define FGIC_GICD_CTLR_DS_MASK  BIT(6)

/* Need check system whether support Security states */

#define FGIC_GICD_CTLR_WRITE(gicd_base, reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_CTLR_OFFSET, reg)
#define FGIC_GICD_CTLR_READ(gicd_base) FGIC_READREG32(gicd_base , FGIC_GICD_CTLR_OFFSET)

/* FGIC_GICD_ISENABLER_OFFSET --- SPI Open */
#define FGIC_GICD_ISENABLER_VALUE_OFFSET(itnum) ((itnum % 32U))
#define FGIC_GICD_ISENABLER_VALUE_MASK(itnum)   (0x1U << FGIC_GICD_ISENABLER_VALUE_OFFSET(itnum))
#define FGIC_GICD_ISENABLER_READ_N_MASK(gicd_base,itnum)   FGIC_READREG32(gicd_base , FGIC_GICD_ISENABLER_OFFSET + ((itnum >> 5)<<2) )
#define FGIC_GICD_ISENABLER_WRITE_N_MASK(gicd_base,itnum,reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_ISENABLER_OFFSET + ((itnum >> 5)<<2), reg)

/* FGIC_GICD_TYPER_OFFSET --- Provides information about what features the GIC implementation supports. */
#define FGIC_GICD_TYPER_ITLINESNUMBER_MASK 0x1f


/* FGIC_GICD_ICENABLER_OFFSET --- SPI close */
#define FGIC_GICD_ICENABLER_DEFAULT_MASK  BIT_MASK(32)
#define FGIC_GICD_ICENABLER_VALUE_OFFSET(itnum) ((itnum % 32U))
#define FGIC_GICD_ICENABLER_VALUE_MASK(itnum)   (0x1U << FGIC_GICD_ICENABLER_VALUE_OFFSET(itnum))
#define FGIC_GICD_ICENABLER_READ_N_MASK(gicd_base,itnum)   FGIC_READREG32(gicd_base , FGIC_GICD_ICENABLER_OFFSET + ((itnum >> 5)<<2) )
#define FGIC_GICD_ICENABLER_WRITE_N_MASK(gicd_base,itnum,reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_ICENABLER_OFFSET + ((itnum >> 5)<<2), reg)


/* FGIC_GICD_IPRIORITYR_OFFSET --- SPI priority */
#define FGIC_GICD_IPRIORITYR_VALUE_MASK(itnum)   (0xFFU << ((itnum % 4U) << 3))
#define FGIC_GICD_IPRIORITYR_READ_N_MASK(gicd_base,itnum)   FGIC_READREG32(gicd_base , FGIC_GICD_IPRIORITYR_OFFSET + ((itnum >> 2)<<2) )
#define FGIC_GICD_IPRIORITYR_WRITE_N_MASK(gicd_base,itnum,reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_IPRIORITYR_OFFSET + ((itnum >> 2)<<2), reg)

/* FGIC_GICD_IROUTER_OFFSET --- SPI Routing */
#define FGIC_GICD_IROUTER_AFFINITY_MASK (((0xFFULL) <<32)|((0xFFULL) <<16)|((0xFFULL)<<8)|(0xFFULL)) /* affinity mask */
#define FGIC_GICD_IROUTER_RANGE_LIMIT (1023) /* GICD_IROUTER<n>, Interrupt Routing Registers, n = 0 - 1019 */
#define FGIC_GICD_IROUTER_BYTE_WIDTH 8
#define FGIC_GICD_IROUTER_WRITE(gicd_base, bank, reg) FGIC_WRITEREG64(gicd_base , FGIC_GICD_IROUTER_OFFSET + (bank * FGIC_GICD_IROUTER_BYTE_WIDTH), reg)
#define FGIC_GICD_IROUTER_READ(gicd_base, bank) FGIC_READREG64(gicd_base , FGIC_GICD_IROUTER_OFFSET + (bank * FGIC_GICD_IROUTER_BYTE_WIDTH))

/* FGIC_GICD_ITARGETSR_OFFSET --- legacy operation （ affinity routing is not enabled）  */
#define FGIC_GICD_ITARGETSR_BYTE_WIDTH 4
#define FGIC_GICD_ITARGETSR_WRITE(gicd_base, bank, reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_ITARGETSR_OFFSET + (bank * FGIC_GICD_ITARGETSR_BYTE_WIDTH), reg)
#define FGIC_GICD_ITARGETSR_READ(gicd_base, bank) FGIC_READREG32(gicd_base , FGIC_GICD_ITARGETSR_OFFSET + (bank * FGIC_GICD_ITARGETSR_BYTE_WIDTH))

/* FGIC_GICD_ICFGR_OFFSET --- edge-triggered or level-sensitive */
#define FGIC_GICD_ICFGR_VALUE_OFFSET(itnum) ((itnum % 16U) << 1)
#define FGIC_GICD_ICFGR_VALUE_MASK(itnum)   (0x3U << FGIC_GICD_ICFGR_VALUE_OFFSET(itnum))
#define FGIC_GICD_ICFGR_READ_N_MASK(gicd_base,itnum)   FGIC_READREG32(gicd_base , FGIC_GICD_ICFGR_OFFSET + ((itnum >> 4)<<2) )
#define FGIC_GICD_ICFGR_WRITE_N_MASK(gicd_base,itnum,reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_ICFGR_OFFSET + ((itnum >> 4)<<2), reg)


/* FGIC_GICD_ISPENDR_OFFSET --- about spi pending */
#define FGIC_GICD_ISPENDR_BYTE_WIDTH 4
#define FGIC_GICD_ISPENDR_WRITE(gicd_base, bank, reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_ISPENDR_OFFSET + (bank * FGIC_GICD_ISPENDR_BYTE_WIDTH), reg)
#define FGIC_GICD_ISPENDR_READ(gicd_base, bank) FGIC_READREG32(gicd_base , FGIC_GICD_ISPENDR_OFFSET + (bank * FGIC_GICD_ISPENDR_BYTE_WIDTH))


/* FGIC_GICD_ICPENDR_OFFSET --- about spi pending */
#define FGIC_GICD_ICPENDR_DEFAULT_MASK  BIT_MASK(32)
#define FGIC_GICD_ICPENDR_VALUE_OFFSET(itnum) ((itnum % 32U))
#define FGIC_GICD_ICPENDR_VALUE_MASK(itnum)   (0x1U << FGIC_GICD_ICPENDR_VALUE_OFFSET(itnum))
#define FGIC_GICD_ICPENDR_READ_N_MASK(gicd_base,itnum)   FGIC_READREG32(gicd_base , FGIC_GICD_ICPENDR_OFFSET + ((itnum >> 5)<<2) )
#define FGIC_GICD_ICPENDR_WRITE_N_MASK(gicd_base,itnum,reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_ICPENDR_OFFSET + ((itnum >> 5)<<2), reg)




/* FGIC_GICD_IGROUPR_OFFSET ---  */
#define FGIC_GICD_IGROUPR_VALUE_OFFSET(itnum) ((itnum % 32U))
#define FGIC_GICD_IGROUPR_VALUE_MASK(itnum)   (0x1U << FGIC_GICD_IGROUPR_VALUE_OFFSET(itnum))
#define FGIC_GICD_IGROUPR_READ_N_MASK(gicd_base,itnum)   FGIC_READREG32(gicd_base , FGIC_GICD_IGROUPR_OFFSET + ((itnum >> 5)<<2) )
#define FGIC_GICD_IGROUPR_WRITE_N_MASK(gicd_base,itnum,reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_IGROUPR_OFFSET + ((itnum >> 5)<<2), reg)


/* FGIC_GICD_IGRPMODR_OFFSET --- Controls whether the corresponding interrupt is in Secure Group 0、Non-secure Group 1、 Secure Group 1 */
#define FGIC_GICD_IGRPMODR_DEFAULT_MASK BIT_MASK(32)
#define FGIC_GICD_IGRPMODR_VALUE_OFFSET(itnum) ((itnum % 32U))
#define FGIC_GICD_IGRPMODR_VALUE_MASK(itnum)   (0x1U << FGIC_GICD_IGRPMODR_VALUE_OFFSET(itnum))
#define FGIC_GICD_IGRPMODR_READ_N_MASK(gicd_base,itnum)   FGIC_READREG32(gicd_base , FGIC_GICD_IGRPMODR_OFFSET + ((itnum >> 5)<<2) )
#define FGIC_GICD_IGRPMODR_WRITE_N_MASK(gicd_base,itnum,reg) FGIC_WRITEREG32(gicd_base , FGIC_GICD_IGRPMODR_OFFSET + ((itnum >> 5)<<2), reg)



#define GICD_ICFGR_MODE TRIGGER_LEVEL
#define GICD_IRPITER_MODE SPI_ROUTING_MODE

typedef enum
{
    GICD_CTLR_ENABLE_GRP0 = (1 << 0),
    GICD_CTLR_ENABLE_GRP1_NS = (1 << 1),
    GICD_CTLR_ENABLE_GRP1A = (1 << 1),
    GICD_CTLR_ENABLE_GRP1S = (1 << 2),
    GICD_CTLR_ENABLE_ALL = (1 << 2) | (1 << 1) | (1 << 0),
    GICD_CTLR_BIT_ARE_S = (1 << 4), /* Enable Secure state affinity routing ， for single Security state ，this bit is  */
    GICD_CTLR_BIT_ARE_NS = (1 << 5),    /* Enable Non-Secure state affinity routing */
    GICD_CTLR_BIT_DS = (1 << 6),    /* Disable Security support */
    GICD_CTLR_BIT_E1NWF = (1 << 7)  /* Enable "1-of-N" wakeup model */
} GICD_CTLR_VALUE;

typedef enum
{
    GICD_GROUP_G0S = 0,
    GICD_GROUP_G1NS = (1 << 0),
    GICD_GROUP_G1S = (1 << 2),
} GICD_GROUP_SECURE_MODE;

static inline void FGicSetGicd(uintptr dist_base, GICD_CTLR_VALUE ctrl_value)
{
    FGIC_GICD_CTLR_WRITE(dist_base, ctrl_value);
}

/**
 * @name: FGicEnableSPI
 * @msg:  configure the priority for a shared peripheral interrupt
 * @param {FGic} *gic_p is a pointer to the FGic instance.
 * @param {u32} spi_id spi interrupt identifier ,value range 32-1019
 */
static inline void FGicEnableSPI(uintptr dist_base, u32 spi_id)
{
    FGIC_GICD_ISENABLER_WRITE_N_MASK(dist_base, spi_id, (1U << (spi_id % 32)));
}

static inline void FGicDisableSPI(uintptr dist_base, u32 spi_id)
{
    FGIC_GICD_ICENABLER_WRITE_N_MASK(dist_base, spi_id, (1U << (spi_id % 32)));
}

static inline void FGicSetSpiPriority(uintptr dist_base, u32 spi_id, u32 priority)
{
    u32 mask;

    /* For SPIs , has one byte-wide entry per interrupt */
    mask = FGIC_GICD_IPRIORITYR_READ_N_MASK(dist_base, spi_id);
    mask &= ~FGIC_GICD_IPRIORITYR_VALUE_MASK(spi_id);
    mask |= ((priority & 0xffU) << (spi_id % 4) * 8U);
    FGIC_GICD_IPRIORITYR_WRITE_N_MASK(dist_base, spi_id, mask);
}

static inline u32 FGicGetSpiPriority(uintptr dist_base, u32 spi_id)
{
    u32 mask;

    /* For SPIs , has one byte-wide entry per interrupt */
    mask = FGIC_GICD_IPRIORITYR_READ_N_MASK(dist_base, spi_id);

    return (mask >> ((spi_id % 4U) * 8U)) & 0xFFU ;
}

static inline void FGicSetSpiRoute(uintptr dist_base, u32 spi_id, GICD_IRPITER_MODE route_mode, u64 affinity)
{
    u32 bank;

    /* For SPIs ,has one doubleword-wide entry per interrupt */
    bank = spi_id & FGIC_GICD_IROUTER_RANGE_LIMIT;
    __asm__ volatile("dsb 0xF" ::: "memory");
    FGIC_GICD_IROUTER_WRITE(dist_base, bank, affinity | route_mode);
    __asm__ volatile("isb 0xF" ::: "memory");
}

static inline u64 FGicGetSpiRoute(uintptr dist_base, u32 spi_id)
{
    u32 bank;
    /* For SPIs ,has one doubleword-wide entry per interrupt */
    bank = spi_id & FGIC_GICD_IROUTER_RANGE_LIMIT;
    return FGIC_GICD_IROUTER_READ(dist_base, bank);
}

static inline void FGicSetSpiLevel(uintptr dist_base, u32 spi_id, GICD_ICFGR_MODE mode)
{
    u32 mask ;
    mask = FGIC_GICD_ICFGR_READ_N_MASK(dist_base, spi_id);
    mask &= ~FGIC_GICD_ICFGR_VALUE_MASK(spi_id);
    mask |= (mode << FGIC_GICD_ICFGR_VALUE_OFFSET(spi_id));
    FGIC_GICD_ICFGR_WRITE_N_MASK(dist_base, spi_id, mask);
}

static inline u32 FGicGetSpiLevel(uintptr dist_base, u32 spi_id)
{
    u32 mask ;
    mask = FGIC_GICD_ICFGR_READ_N_MASK(dist_base, spi_id);
    return (mask >> ((spi_id % 16U) >> 1U)) ;
}

static inline void FGicSetSpiSecurity(uintptr dist_base, u32 spi_id, GICD_GROUP_SECURE_MODE mode)
{
    u32 mask ;
    /* Group status */
    mask = FGIC_GICD_IGROUPR_READ_N_MASK(dist_base, spi_id);
    mask &= ~FGIC_GICD_IGROUPR_VALUE_MASK(spi_id);

    mask |= ((mode & 0x1) << (spi_id % 32));
    FGIC_GICD_IGROUPR_WRITE_N_MASK(dist_base, spi_id, mask);

    /* Group modifier */
    mask = FGIC_GICD_IGRPMODR_READ_N_MASK(dist_base, spi_id);
    mask &= ~FGIC_GICD_IGRPMODR_VALUE_MASK(spi_id);

    mask |= (((mode & 0x2) >> 1)  << (spi_id % 32));
    FGIC_GICD_IGRPMODR_WRITE_N_MASK(dist_base, spi_id, mask);
}

static inline u32 FGicGetSpiSecurity(uintptr dist_base, u32 spi_id)
{
    u32 mask ;
    u32 group_status, group_modifier;
    /* Group status */
    mask = FGIC_GICD_IGROUPR_READ_N_MASK(dist_base, spi_id);
    mask &= FGIC_GICD_IGROUPR_VALUE_MASK(spi_id);
    group_status = (mask >> (spi_id % 32));

    /* Group modifier */
    mask = FGIC_GICD_IGRPMODR_READ_N_MASK(dist_base, spi_id);
    mask &= FGIC_GICD_IGRPMODR_VALUE_MASK(spi_id);
    group_modifier = (mask >> (spi_id % 32));

    return ((group_modifier << 1) | group_status);
}


/***************************** redistributor *********************************/

/* Each Redistributor defines two 64KB frames in the physical address map */
#define FGIC_GICR_CTLR_OFFSET 0x00000000U    /* See the register description Redistributor Control Register ,RW */
#define FGIC_GICR_IIDR_OFFSET 0x00000004U    /* Implementer Identification Register ,RO */
#define FGIC_GICR_TYPER_OFFSET 0x00000008U   /* Redistributor Type Register ,RO */
#define FGIC_GICR_STATUSR_OFFSET 0x00000010U /* Error Reporting Status Register, optional ,RW */
#define FGIC_GICR_WAKER_OFFSET 0x00000014U   /* See the register description Redistributor Wake Register ,RW */
#define FGIC_GICR_MPAMIDR_OFFSET 0x00000018U /* Report maximum PARTID and PMG Register ,RO */
#define FGIC_GICR_PARTIDR_OFFSET 0x0000001CU /* Set PARTID and PMG Register ,RW */

#define FGIC_GICR_SETLPIR_OFFSET 0x00000040U /* Set LPI Pending Register ,WO */
#define FGIC_GICR_CLRLPIR_OFFSET 0x00000048U /* Clear LPI Pending Register ,WO */

#define FGIC_GICR_PROPBASER_OFFSET 0x00000070U /* Redistributor Properties Base Address Register ,RW */
#define FGIC_GICR_PENDBASER_OFFSET 0x00000078U /* Redistributor LPI Pending Table Base Address Register ,RW */

/* Redistributor - SGI_BASE */

#define FGIC_GICR_SGI_BASE_OFFSET 0x10000U /* 64KB frames */

#define FGIC_GICR_IGROUPR0_OFFSET 0x00000080U  /* Interrupt Group Register 0 ,RW */
#define FGIC_GICR_IGROUPR_E_OFFSET 0x00000084U /* Interrupt Group Registers for extended PPI range ,RW */

#define FGIC_GICR_ISENABLER0_OFFSET 0x00000100U  /* Interrupt Set-Enable Register 0 ,RW */
#define FGIC_GICR_ISENABLER_E_OFFSET 0x00000104U /* Interrupt Set-Enable for extended PPI range ,RW */

#define FGIC_GICR_ICENABLER0_OFFSET 0x00000180U  /* Interrupt Clear-Enable Register 0 ,RW */
#define FGIC_GICR_ICENABLER_E_OFFSET 0x00000184U /* Interrupt Clear-Enable for extended PPI range ,RW */


#define FGIC_GICR_ISPENDR0_OFFSET 0x00000200U  /* Interrupt Set-Pend Register 0 ,RW */
#define FGIC_GICR_ISPENDR_E_OFFSET 0x00000204U /* Interrupt Set-Pend for extended PPI range ,RW */

#define FGIC_GICR_ICPENDR0_OFFSET 0x00000280U /* Interrupt Clear-Pend Register 0 ,RW */

#define FGIC_GICR_ICPENDR_E_OFFSET 0x00000284U /* Interrupt Clear-Pend for extended PPI range ,RW */

#define FGIC_GICR_ISACTIVER0_OFFSET 0x00000300U  /* Interrupt Set-Active Register 0 ,RW */
#define FGIC_GICR_ISACTIVER_E_OFFSET 0x00000304U /* Interrupt Set-Active for extended PPI range ,RW */

#define FGIC_GICR_ICACTIVER0_OFFSET 0x00000380U  /* Interrupt Clear-Active Register 0 ,RW */
#define FGIC_GICR_ICACTIVER_E_OFFSET 0x00000384U /* Interrput Clear-Active for extended PPI range ,RW */

#define FGIC_GICR_IPRIORITYR_OFFSET 0x00000400U   /* Interrupt Priority Registers ,RW */
#define FGIC_GICR_IPRIORITYR_E_OFFSET 0x00000420U /* Interrupt Priority for extended PPI range ,RW */

#define FGIC_GICR_ICFGR0_OFFSET 0x00000C00U /*  SGI Configuration Register ,RW*/
#define FGIC_GICR_ICFGR1_OFFSET 0x00000C04U /*  PPI Configuration Register ,RW*/

#define FGIC_GICR_ICFGR_E_OFFSET 0x00000C08U   /* Extended PPI Configuration Register ,RW */
#define FGIC_GICR_IGRPMODR0_OFFSET 0x00000D00U /* Interrupt Group Modifier Register 0 ,RW */

#define FGIC_GICR_IGRPMODR_E_OFFSET 0x00000D04U /* Interrupt Group Modifier for extended PPI range ,RW */

#define FGIC_GICR_NSACR_OFFSET 0x00000E00U /* Non-Secure Access Control Register ,RW */


/* FGIC_GICR_TYPER_OFFSET --- Provides information about the configuration of this Redistributor. */
#define FGIC_GICR_TYPER_BYTE_WIDTH 4
#define FGIC_GICR_TYPER_L_READ(redis_base) FGIC_READREG32(redis_base , FGIC_GICR_TYPER_OFFSET)
#define FGIC_GICR_TYPER_H_READ(redis_base) FGIC_READREG32(redis_base , FGIC_GICR_TYPER_OFFSET + FGIC_GICR_TYPER_BYTE_WIDTH)
#define FGIC_GICR_TYPER_READ(redis_base)   FGIC_READREG64(redis_base , FGIC_GICR_TYPER_OFFSET)

/* FGIC_GICR_WAKER_OFFSET ---  Permits software to control the behavior of the WakeRequest power management signal corresponding to the Redistributor */
#define FGIC_GICR_WAKER_PROCESSOR_SLEEP_MASK BIT(1)
#define FGIC_GICR_WAKER_CHILDREN_ASLEEP_MASK BIT(2)
#define FGIC_GICR_WAKER_CLEAR_BIT(redis_base, bit) FGIC_CLEARBIT(redis_base,FGIC_GICR_WAKER_OFFSET,bit)
#define FGIC_GICR_WAKER_WRITE(redis_base, reg) FGIC_WRITEREG32(redis_base , FGIC_GICR_WAKER_OFFSET, reg)
#define FGIC_GICR_WAKER_READ(redis_base) FGIC_READREG32(redis_base , FGIC_GICR_WAKER_OFFSET)

/* FGIC_GICR_IPRIORITYR_OFFSET --- Enables forwarding of the corresponding SGI or PPI to the CPU interfaces*/
#define FGIC_GICR_IPRIORITYR_VALUE_MASK(itnum)   (0xFFU << ((itnum % 4U) << 3))
#define FGIC_GICR_IPRIORITYR_READ(sgi_base,itnum)   FGIC_READREG32(sgi_base , FGIC_GICR_IPRIORITYR_OFFSET + ((itnum >> 2)<<2) )
#define FGIC_GICR_IPRIORITYR_WRITE(sgi_base,itnum,reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_IPRIORITYR_OFFSET + ((itnum >> 2)<<2), reg)

/* FGIC_GICR_ISPENDR0_OFFSET --- Adds the pending state to the corresponding SGI or PPI. */
#define FGIC_GICR_ISPENDR0_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_ISPENDR0_OFFSET, reg)
#define FGIC_GICR_ISPENDR0_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_ISPENDR0_OFFSET)

/* FGIC_GICR_ICPENDR0_OFFSET --- Removes the pending state from the corresponding SGI or PPI. */
#define FGIC_GICR_ICPENDR0_DEFAULT_MASK BIT_MASK(32)
#define FGIC_GICR_ICPENDR0_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_ICPENDR0_OFFSET, reg)
#define FGIC_GICR_ICPENDR0_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_ICPENDR0_OFFSET)

/* FGIC_GICR_ISACTIVER0_OFFSET --- Activates the corresponding SGI or PPI. These registers are used when saving and restoring GIC state. */

#define FGIC_GICR_ISACTIVER0_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_ISACTIVER0_OFFSET, reg)
#define FGIC_GICR_ISACTIVER0_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_ISACTIVER0_OFFSET)

/* FGIC_GICR_ICACTIVER0_OFFSET --- Deactivates the corresponding SGI or PPI. These registers are used when saving and restoring GIC state.*/
#define FGIC_GICR_ICACTIVER0_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_ICACTIVER0_OFFSET, reg)
#define FGIC_GICR_ICACTIVER0_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_ICACTIVER0_OFFSET)

/* FGIC_GICR_IGROUPR0_OFFSET --- Controls whether the corresponding SGI or PPI is in Group 0 or Group 1. */
#define FGIC_GICR_IGROUPR0_DEFAULT_MASK BIT_MASK(32)
#define FGIC_GICR_IGROUPR0_VALUE_OFFSET(itnum) ((itnum % 32U))
#define FGIC_GICR_IGROUPR0_VALUE_MASK(itnum)   (0x1U << FGIC_GICR_IGROUPR0_VALUE_OFFSET(itnum))
#define FGIC_GICR_IGROUPR0_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_IGROUPR0_OFFSET, reg)
#define FGIC_GICR_IGROUPR0_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_IGROUPR0_OFFSET)

/* FGIC_GICR_IGRPMODR0_OFFSET --- controls whether the corresponding interrupt is in: • Secure Group 0.• Non-secure Group 1.• When System register access is enabled, Secure Group 1. */
#define FGIC_GICR_IGRPMODR0_DEFAULT_MASK BIT_MASK(32)
#define FGIC_GICR_IGRPMODR0_VALUE_OFFSET(itnum) ((itnum % 32U))
#define FGIC_GICR_IGRPMODR0_VALUE_MASK(itnum)   (0x1U << FGIC_GICR_IGRPMODR0_VALUE_OFFSET(itnum))
#define FGIC_GICR_IGRPMODR0_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_IGRPMODR0_OFFSET, reg)
#define FGIC_GICR_IGRPMODR0_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_IGRPMODR0_OFFSET)

/* FGIC_GICR_ISENABLER0_OFFSET --- Enables forwarding of the corresponding interrupt to the CPU interfaces. */
#define FGIC_GICR_ISENABLER0_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_ISENABLER0_OFFSET, reg)
#define FGIC_GICR_ISENABLER0_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_ISENABLER0_OFFSET)

/* FGIC_GICR_ICENABLER0_OFFSET --- Disables forwarding of the corresponding interrupt to the CPU interfaces. */
#define FGIC_GICR_ICENABLER0_DEFAULT_MASK BIT_MASK(32)
#define FGIC_GICR_ICENABLER0_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_ICENABLER0_OFFSET, reg)
#define FGIC_GICR_ICENABLER0_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_ICENABLER0_OFFSET)

/* FGIC_GICR_ICFGR0_OFFSET */
#define FGIC_GICR_ICFGR0_DEFAULT_MASK   0
#define FGIC_GICR_ICFGR0_VALUE_OFFSET(itnum) ((itnum % 16U) << 1)
#define FGIC_GICR_ICFGR0_VALUE_MASK(itnum)   (0x3U << FGIC_GICR_ICFGR0_VALUE_OFFSET(itnum))
#define FGIC_GICR_ICFGR0_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_ICFGR0_OFFSET, reg)
#define FGIC_GICR_ICFGR0_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_ICFGR0_OFFSET)

/* FGIC_GICR_ICFGR1_OFFSET */
#define FGIC_GICR_ICFGR1_VALUE_OFFSET(itnum) ((itnum % 16U) << 1)
#define FGIC_GICR_ICFGR1_VALUE_MASK(itnum)   (0x3U << FGIC_GICR_ICFGR1_VALUE_OFFSET(itnum))
#define FGIC_GICR_ICFGR1_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_ICFGR1_OFFSET, reg)
#define FGIC_GICR_ICFGR1_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_ICFGR1_OFFSET)

/* FGIC_GICR_CTLR_OFFSET */

#define FGIC_GICR_CTLR_RWP_MASK BIT(2)

/* FGIC_GICR_NSACR_OFFSET */

#define FGIC_GICR_NSACR_WRITE(sgi_base, reg) FGIC_WRITEREG32(sgi_base , FGIC_GICR_NSACR_OFFSET, reg)
#define FGIC_GICR_NSACR_READ(sgi_base) FGIC_READREG32(sgi_base , FGIC_GICR_NSACR_OFFSET)

typedef enum
{
    GICR_GROUP_G0S = 0,
    GICR_GROUP_G1NS = (1 << 0),
    GICR_GROUP_G1S = (1 << 2),
} GICR_GROUP_SECURE_MODE;

typedef enum
{
    GICR_WAKER_PROCESSOR_SLEEP = (1 << 1),
    GICR_WAKER_CHILDREN_ASLEEP = (1 << 2)
} GICR_WAKER_MODE;

static inline u32 FGicGetGicrAffinity(uintptr redis_base)
{
    return FGIC_GICR_TYPER_H_READ(redis_base);
}


static inline void FGicWakeGicr(uintptr redis_base)
{
    u32 mask ;
    mask = FGIC_GICR_WAKER_READ(redis_base);
    mask &= ~GICR_WAKER_PROCESSOR_SLEEP ;
    FGIC_GICR_WAKER_WRITE(redis_base, mask);

    do
    {
        mask = FGIC_GICR_WAKER_READ(redis_base);

    }
    while ((mask & GICR_WAKER_CHILDREN_ASLEEP) != 0); /* This PE is not in, and is not entering, a low power state.  */
}


static inline void FGicEnablePrivateInt(uintptr redis_base, s32 int_id)
{
    FGIC_GICR_ISENABLER0_WRITE(redis_base + FGIC_GICR_SGI_BASE_OFFSET, (1U << (int_id % 32)));
}

static inline void FGicDisablePrivateInt(uintptr redis_base, s32 int_id)
{
    FGIC_GICR_ICENABLER0_WRITE(redis_base + FGIC_GICR_SGI_BASE_OFFSET, (1U << (int_id % 32)));
}

static inline void FGicSetPrivatePriority(uintptr redis_base, s32 spi_id, u32 priority)
{
    u32 mask;

    /* For SPIs , has one byte-wide entry per interrupt */
    mask = FGIC_GICR_IPRIORITYR_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET, spi_id);
    mask &= ~FGIC_GICR_IPRIORITYR_VALUE_MASK(spi_id);
    mask |= ((priority & 0xffU) << (spi_id % 4) * 8U);
    FGIC_GICR_IPRIORITYR_WRITE(redis_base + FGIC_GICR_SGI_BASE_OFFSET, spi_id, mask);
}

static inline u32 FGicGetPrivatePriority(uintptr redis_base, s32 spi_id)
{
    u32 mask;
    /* For SPIs , has one byte-wide entry per interrupt */
    mask = FGIC_GICR_IPRIORITYR_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET, spi_id);

    return (mask >> ((spi_id % 4U) * 8U)) & 0xFFU;
}

static inline void FGicSetSgiLevel(uintptr redis_base, s32 spi_id, GICD_ICFGR_MODE mode)
{
    u32 mask ;
    mask = FGIC_GICR_ICFGR0_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET);
    mask &= ~FGIC_GICR_ICFGR0_VALUE_OFFSET(spi_id);
    mask |= (mode << FGIC_GICR_ICFGR0_VALUE_OFFSET(spi_id));
    FGIC_GICR_ICFGR0_WRITE(redis_base, mask);
}

static inline u32 FGicGetSgiLevel(uintptr redis_base, s32 spi_id)
{
    u32 mask ;
    mask = FGIC_GICR_ICFGR0_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET);
    return (mask >> ((spi_id % 16U) >> 1U)) ;
}


static inline void FGicSetPpiLevel(uintptr redis_base, s32 spi_id, GICD_ICFGR_MODE mode)
{
    u32 mask ;
    mask = FGIC_GICR_ICFGR1_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET);
    mask &= ~FGIC_GICR_ICFGR1_VALUE_OFFSET(spi_id);
    mask |= (mode << FGIC_GICR_ICFGR1_VALUE_OFFSET(spi_id));
    FGIC_GICR_ICFGR1_WRITE(redis_base, mask);
}


static inline u32 FGicGetPpiLevel(uintptr redis_base, s32 spi_id)
{
    u32 mask ;
    mask = FGIC_GICR_ICFGR1_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET);
    return (mask >> ((spi_id % 16U) >> 1U)) ;
}

static inline void FGicSetPrivateSecurity(uintptr redis_base, s32 spi_id, GICD_GROUP_SECURE_MODE mode)
{
    u32 mask ;
    /* Group status */
    mask = FGIC_GICR_IGROUPR0_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET);
    mask &= ~FGIC_GICR_IGROUPR0_VALUE_MASK(spi_id);

    mask |= ((mode & 0x1) << (spi_id % 32));
    FGIC_GICR_IGROUPR0_WRITE(redis_base + FGIC_GICR_SGI_BASE_OFFSET, mask);

    /* Group modifier */
    mask = FGIC_GICR_IGRPMODR0_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET);
    mask &= ~FGIC_GICR_IGRPMODR0_VALUE_MASK(spi_id);

    mask |= (((mode & 0x2) >> 1)  << (spi_id % 32));
    FGIC_GICR_IGRPMODR0_WRITE(redis_base + FGIC_GICR_SGI_BASE_OFFSET, mask);
}


static inline u32 FGicGetPrivateSecurity(uintptr redis_base, s32 spi_id)
{
    u32 mask ;
    u32 group_status, group_modifier;
    /* Group status */
    mask = FGIC_GICR_IGROUPR0_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET);
    mask &= FGIC_GICR_IGROUPR0_VALUE_MASK(spi_id);
    group_status = (mask >> (spi_id % 32));

    /* Group modifier */
    mask = FGIC_GICR_IGRPMODR0_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET);
    mask &= FGIC_GICR_IGRPMODR0_VALUE_MASK(spi_id);
    group_modifier = (mask >> (spi_id % 32));

    return ((group_modifier << 1) | group_status);
}


static inline u32 FGicNonSecureAccessRead(uintptr redis_base)
{
    return FGIC_GICR_NSACR_READ(redis_base + FGIC_GICR_SGI_BASE_OFFSET);
}


#ifdef __cplusplus
}
#endif

#endif
