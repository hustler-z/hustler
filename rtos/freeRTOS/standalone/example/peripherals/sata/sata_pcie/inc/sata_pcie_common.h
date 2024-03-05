/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: sata_pcie_common.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for sata pcie example common definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/8   first release
 */
#ifndef  SATA_PCIE_COMMON_H
#define  SATA_PCIE_COMMON_H

#include "fdebug.h"
#include "fsata.h"
#include "fpcie_ecam.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FSATA_DEBUG_TAG "SATA_PCIE_TEST"
#define FSATA_ERROR(format, ...)     FT_DEBUG_PRINT_E(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_WARN(format, ...)      FT_DEBUG_PRINT_W(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_INFO(format, ...)      FT_DEBUG_PRINT_I(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_DEBUG(format, ...)     FT_DEBUG_PRINT_D(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)

enum
{
    SATA_PIO_READ_MODE = 0,
    SATA_PIO_WRITE_MODE = 1,
    SATA_FPDMA_READ_MODE = 2,
    SATA_FPDMA_WRITE_MODE = 3,

    SATA_TRANS_TYPE_NUM
};

#define PCI_CLASS_STORAGE_SATA_AHCI 0x010601
/*sata host max num*/
#define SATA_HOST_MAX_NUM   PLAT_AHCI_HOST_MAX_COUNT
/*set the sata block size*/
#define FSATA_BLOCK_SIZE    512
/*Address aligned with 1024*/
#define ADDR_ALIGNMENT 1024
/*sata test default ahci host*/
#define FSATA_PCIE_TEST_AHCI_HOST 0
/*sata pcie test used start block*/
#define FSATA_PCIE_TEST_BLOCK 100
/*sata pcie test used block num*/
#define FSATA_PCIE_TEST_BLOCK_NUM 1
/*pcie and sata init function*/
FError FSataPcieInit(u32 ahci_host);
/*sata pcie read and write funtion*/
FError FSataPcieReadWrite(u8 ahci_host, u32 blk, int blk_num, u8 *buffer, u32 trans_type);
/*sata pcie deinit funtion*/
int FSataPcieDeinit(u32 ahci_host);
/*set the sata interrupt*/
void FSataPcieIrqInit(u32 ahci_host);
/*deinit the sata interrupt*/
void FSataPcieIrqDeInit(u32 ahci_host);
#ifdef __cplusplus
}
#endif

#endif

