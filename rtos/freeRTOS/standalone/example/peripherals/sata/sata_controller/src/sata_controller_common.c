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
 * FilePath: sata_controller_common.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for sata controller example common functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/31   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "strto.h"
#include "finterrupt.h"
#include "fsata.h"
#include "fsata_hw.h"
#include "fcpu_info.h"
#include "ftypes.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#include "sata_controller_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
static u8 mem[50000] __attribute__((aligned(ADDR_ALIGNMENT))) = {0};
/*sata controller */
static FSataCtrl sata_device[FSATA_NUM];
/*sata controller default port*/
static u32 port = FSATA_CONTROLLER_DEFAULT_PORT;
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/

static void FSataDhrsIrq(void *args)
{
    FSataCtrl *instance_p = (FSataCtrl *)args;
    printf("Sata controller DEVICE-TO-HOST register fis interrupt.\n");
    instance_p->dhrs_flag = 1;
}

static void FSataPssIrq(void *args)
{
    /* pio setup fis irq handler */
    printf("Sata controller pio setup fis interrupt.\n");
}

static void FSataSdbIrq(void *args)
{
    FSataCtrl *instance_p = (FSataCtrl *)args;
    printf("Sata controller set device bits interrupt.\n");
    instance_p->sdb_flag = 1;
}

static void FSataConnectIrq(void *args)
{
    /* connnect irq handler */
    printf("Sata controller port connect change status interrupt.\n");
}

/*set the sata interrupt*/
void FSataControllerIrqInit(u32 ahci_host)
{
    u32 instance_id = ahci_host;
    u32 cpu_id;
    GetCpuId(&cpu_id);
    FSATA_DEBUG("cpu_id is %d, irq_num=%d", cpu_id, sata_device[instance_id].config.irq_num);
    InterruptSetTargetCpus(sata_device[instance_id].config.irq_num, cpu_id);

    FSataSetHandler(&sata_device[instance_id], FSATA_PORT_IRQ_CONNECT, FSataConnectIrq, NULL);
    FSataSetHandler(&sata_device[instance_id], FSATA_PORT_IRQ_SDB_FIS, FSataSdbIrq, &sata_device[instance_id]);
    FSataSetHandler(&sata_device[instance_id], FSATA_PORT_IRQ_D2H_REG_FIS, FSataDhrsIrq, &sata_device[instance_id]);
    FSataSetHandler(&sata_device[instance_id], FSATA_PORT_IRQ_PIOS_FIS, FSataPssIrq, NULL);
    FSataIrqEnable(&sata_device[instance_id], FSATA_PORT_IRQ_FREEZE);

    /* register interrupt handler*/
    InterruptSetPriority(sata_device[instance_id].config.irq_num, 0);
    InterruptInstall(sata_device[instance_id].config.irq_num, FSataIrqHandler, &sata_device[instance_id], sata_device[instance_id].config.instance_name);
    InterruptUmask(sata_device[instance_id].config.irq_num);

}

void FSataControllerIrqDeInit(u32 ahci_host)
{
    u32 instance_id = ahci_host;
    InterruptMask(sata_device[instance_id].config.irq_num);
    FSataIrqDisable(&sata_device[instance_id], FSATA_PORT_IRQ_FREEZE);
}

FError FSataControllerInit(u32 ahci_host)
{
    u32 instance_id = ahci_host;
    const FSataConfig *config_p = NULL;
    FError ret = FSATA_SUCCESS;
    
    config_p = FSataLookupConfig(instance_id, FSATA_TYPE_CONTROLLER);
    if (config_p != NULL)
    {
        ret = FSataCfgInitialize(&sata_device[instance_id], config_p);
        if (FSATA_SUCCESS != ret)
        {
            FSATA_ERROR("init sata failed, ret: 0x%x", ret);
        }

        FSATA_DEBUG("plat ahci host[%d] base_addr = 0x%x", instance_id, sata_device[instance_id].config.base_addr);
        FSATA_DEBUG("plat ahci host[%d] irq_num = %d", instance_id, sata_device[instance_id].config.irq_num);
    }

    /* init ahci host and port */
    ret = FSataAhciInit(&sata_device[instance_id]);
    if (FSATA_SUCCESS != ret)
    {
        FSataCfgDeInitialize(&sata_device[instance_id]);
        FSATA_ERROR("FSataAhciInit sata failed, ret: 0x%x", ret);
        return FSATA_ERR_OPERATION;
    }
    u32 port_map = sata_device[instance_id].port_map;
    /* command list address must be 1K-byte aligned */
    ret = FSataAhciPortStart(&sata_device[instance_id], port, (uintptr)mem);
    if (FSATA_SUCCESS != ret)
    {
        FSATA_ERROR("FSataAhciPortStart host %d port %d failed, ret: 0x%x", instance_id, port, ret);
        return FSATA_ERR_OPERATION;
    }
    ret = FSataAhciReadInfo(&sata_device[instance_id], port);
    if (FSATA_SUCCESS != ret)
    {
        FSataCfgDeInitialize(&sata_device[instance_id]);
        FSATA_ERROR("FSataAhciReadInfo %d-%d failed, ret: 0x%x", instance_id, port, ret);
        return FSATA_ERR_OPERATION;
    }

    return ret;
}

FError FSataControllerReadWrite(u8 ahci_host, u32 blk, u32 blk_num, u8 *buffer, u32 trans_type)
{
    FError ret;
    u32 instance_id = ahci_host;

    if (ahci_host >= FSATA_NUM)
    {
        FSATA_ERROR("ahci_host number is bigger than FSATA_NUM.\n");
        return FSATA_ERR_INVALID_PARAMETER;
    }

    switch (trans_type)
    {
    case SATA_PIO_READ_MODE:
        ret = FSataReadWrite(&sata_device[instance_id], port, blk, blk_num, buffer, FALSE, FALSE);
        break;
    case SATA_PIO_WRITE_MODE:
        ret = FSataReadWrite(&sata_device[instance_id], port, blk, blk_num, buffer, FALSE, TRUE);
        break;
    case SATA_FPDMA_READ_MODE:
        ret = FSataReadWrite(&sata_device[instance_id], port, blk, blk_num, buffer, TRUE, FALSE);
        break;
    case SATA_FPDMA_WRITE_MODE:
        ret = FSataReadWrite(&sata_device[instance_id], port, blk, blk_num, buffer, TRUE, TRUE);
        break;
    }

    if (FT_SUCCESS != ret)
    {
        FSATA_ERROR("Sata conrtoller failed to read and write data.\r\n");
        return FSATA_ERR_OPERATION;
    }

    return ret;
}

int FSataControllerDeinit(u32 ahci_host)
{
    u32 instance_id = ahci_host;
    /*interrupt deinit*/
    InterruptMask(sata_device[instance_id].config.irq_num);
    /*sata deinit*/
    FSataCfgDeInitialize(&sata_device[instance_id]);

    return 0;
}