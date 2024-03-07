/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: fddma_bdl.c
 * Created Date: 2023-08-08 15:46:34
 * Last Modified: 2023-12-21 11:12:57
 * Description:  This file is for DDMA BDL transfer interface definition which is only used in I2S examples.
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0  wangzongqiang     2023/07/23    init
 *  1.1  liqiaozhong       2023/12/12    an overhaul
 */

#ifndef  FDDMA_BDL_H
#define  FDDMA_BDL_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fddma.h"

#ifdef __cplusplus
extern "C"
{
#endif
/**************************** Type Definitions *******************************/
typedef enum
{
    FDDMA_BURST_LENGTH_1 = 0,
    FDDMA_BURST_LENGTH_2 = 1,
    FDDMA_BURST_LENGTH_4 = 3,
} FDdmaBurstLength;

typedef enum
{
    FDDMA_BURST_SIZE_4_BYTE = 0,
    FDDMA_BURST_SIZE_1_BYTE = 1,
    FDDMA_BURST_SIZE_2_BYTE = 2,
} FDdmaBurstSize;

typedef struct _FDdmaBdlDesc
{
    u32 src_addr_l;  /* 0x0, 数据源地址低32位 */
    u32 src_addr_h;  /* 0x4, 数据源地址高32位 */
    u32 total_bytes; /* 0x08, 传输数据总量*/
    u32 ioc;         /* 0x0c, 该条目传输完成中断产生控制位  */
} __attribute__((__packed__)) FDdmaBdlDesc; /* BDL描述符 */

typedef struct
{
    fsize_t           current_desc_num; /* 表示当前操作的是所在描述符列表中的第几个描述符 */
    uintptr           src_addr;         /* DDMA传输源地址-物理地址 */
    fsize_t           trans_length;     /* 单个描述符所负责的传输数据量 */
    boolean           ioc;              /* TRUE：该描述符传输完成会单独输出一个中断和置位状态位（DMA_Cx_STATE的[2]位）；FLASE：不单独输出 */
} FDdmaBdlDescConfig;  /* BDL描述符配置 */
/***************** Macros (Inline Functions) Definitions *********************/
#define FDDMA_BDL_ADDR_ALIGMENT            128U /* BDL模式的地址需要按128位对齐 */
#define FDDMA_BDL_VALID_NUM                4U   /*BDL模式下所有条目数据总量必须为4的倍数*/
/************************** Function Prototypes ******************************/
/* 仅在支持BDL模式的DDMA控制器有效，目前仅在I2S用例中使用 */
/* 配置一个DDMA-BDL描述符 */
FError FDdmaBDLSetDesc(FDdmaBdlDesc *const first_desc_addr_p, FDdmaBdlDescConfig const *bdl_desc_config_p);

/*检查并设置用于DDMA BDL模式的通道*/
FError FDdmaChanBdlConfigure(FDdma *const instance, FDdmaChanIndex channel_id, const FDdmaChanConfig *channel_config_p);

#ifdef __cplusplus
}
#endif

#endif