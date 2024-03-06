
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
 * FilePath: sata_pcie_common.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for sata pcie common functions 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  * 1.0   zhangyan   2023/3/31   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#include "finterrupt.h"
#include "fpcie_ecam.h"
#include "fpcie_ecam_common.h"
#include "fsata.h"
#include "fsata_hw.h"
#include "fcpu_info.h"
#include "ferror_code.h"

#include "sata_pcie_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
static u8 mem[50000] __attribute__((aligned(ADDR_ALIGNMENT))) = {0};
/*max support 16 ahci controllers*/
static FSataCtrl sata_device[SATA_HOST_MAX_NUM];
static FPcieEcam pcie_device; 
static u32 port;
static s32 sata_host_count = 0;
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void FSataPcieIrqHandler(void *param)
{
    FSataIrqHandler(0, param);
}

static void PCieIntxInit(FPcieEcam *instance_p)
{
    u32 cpu_id;
    GetCpuId(&cpu_id);
    FSATA_DEBUG("cpu_id is %d", cpu_id);
    InterruptSetTargetCpus(FPCIE_ECAM_INTA_IRQ_NUM, cpu_id);

    InterruptSetPriority(FPCIE_ECAM_INTA_IRQ_NUM, 0);
    /* register interrupt handler*/
    InterruptInstall(FPCIE_ECAM_INTA_IRQ_NUM, (IrqHandler)FPcieEcamIntxIrqHandler, instance_p, "pcieInta");
    InterruptUmask(FPCIE_ECAM_INTA_IRQ_NUM);

}

static FError FPcieInit(FPcieEcam *pcie_device)
{
    FPcieEcamCfgInitialize(pcie_device, FPcieEcamLookupConfig(FPCIE_ECAM_INSTANCE0),NULL);
    FSATA_DEBUG("\n");
    FSATA_DEBUG("	PCI:\n");
    FSATA_DEBUG("	B:D:F			VID:PID			parent_BDF			class_code\n");
    FPcieEcamEnumerateBus(pcie_device,0) ;
    PCieIntxInit(pcie_device);    /*注册pcie intx中断处理函数*/
}

static FError SataPcieIrqInstall(FSataCtrl *sata_device, FPcieEcam *pcie_device, u8 bus, u8 device, u8 function)
{
    FError ret = FSATA_SUCCESS;
    /* 初始化sata 控制器对应的中断处理函数*/
    FPcieIntxFun intx_fun;
    intx_fun.IntxCallBack = FSataPcieIrqHandler;
    intx_fun.args = sata_device;
    intx_fun.bus = bus;
	intx_fun.device = device;
	intx_fun.function = function;

    /*将bdf的interrupt pin和interrupt line进行初始化，并将intx_fun写入到pcie_obj成员中，初始化pcie_obj*/
    ret = FPcieEcamIntxRegister(pcie_device, bus, device, function, &intx_fun);
    if (FSATA_SUCCESS != ret)
    {
        FSATA_ERROR("FPcieIntxRegiterIrqHandler failed.\n");
        return FSATA_ERR_OPERATION;
    }

    return ret;
}

static void FSataDhrsIrq(void *args)
{
    FSataCtrl *instance_p = (FSataCtrl *)args;
    printf("Sata pcie DEVICE-TO-HOST register fis interrupt.\n");
    instance_p->dhrs_flag = 1;
}

static void FSataPssIrq(void *args)
{
    /* pio setup fis irq handler */
    printf("Sata pcie pio setup fis interrupt.\n");
}

static void FSataSdbIrq(void *args)
{
    FSataCtrl *instance_p = (FSataCtrl *)args;
    printf("Sata pcie set device bits interrupt.\n");
    instance_p->sdb_flag = 1;
}

static void FSataConnectIrq(void *args)
{
    /*connnect irq handler */
    printf("Sata pcie port connect change status interrupt.\n");
}

void FSataPcieIrqInit(u32 ahci_host)
{
    u32 instance_id = ahci_host;
    FSataSetHandler(&sata_device[instance_id], FSATA_PORT_IRQ_CONNECT, FSataConnectIrq, NULL);
    FSataSetHandler(&sata_device[instance_id], FSATA_PORT_IRQ_SDB_FIS, FSataSdbIrq, &sata_device[instance_id]);
    FSataSetHandler(&sata_device[instance_id], FSATA_PORT_IRQ_D2H_REG_FIS, FSataDhrsIrq, &sata_device[instance_id]);
    FSataSetHandler(&sata_device[instance_id], FSATA_PORT_IRQ_PIOS_FIS, FSataPssIrq, NULL);
    FSataIrqEnable(&sata_device[instance_id], FSATA_PORT_IRQ_FREEZE);
}

void FSataPcieIrqDeInit(u32 ahci_host)
{
    u32 instance_id = ahci_host;
    FSataIrqDisable(&sata_device[instance_id], FSATA_PORT_IRQ_FREEZE);
}

FError FSataPcieInit(u32 ahci_host)
{
    FError ret = FSATA_SUCCESS;
    u32 instance_id = ahci_host;
    s32 host;
    u32 bdf;
    u32 class;
    u16 pci_command;
    const u32 class_code = PCI_CLASS_STORAGE_SATA_AHCI;
    uintptr bar_addr = 0;
    u16 vid, did;
    const FSataConfig *config_p = NULL;
    u8 bus,device,function;
    u32 config_data ;

    ret = FPcieInit(&pcie_device);
    if (FSATA_SUCCESS != ret)
    {
        FSATA_ERROR("FPcieInit failed.\n");
        return FSATA_ERR_OPERATION;
    }

    memset(&sata_device[instance_id], 0, sizeof(sata_device[instance_id]));
    config_p = FSataLookupConfig(ahci_host, FSATA_TYPE_PCIE);
    ret = FSataCfgInitialize(&sata_device[instance_id], config_p);
    if (FSATA_SUCCESS != ret)
    {
        FSATA_ERROR("Init sata failed. \n");
        return FSATA_ERR_OPERATION;
    }
    /* find xhci host from pcie instance */
    for (host = 0; host < pcie_device.scans_bdf_count; host++)
    {

        bus		= pcie_device.scans_bdf[host].bus;
		device	= pcie_device.scans_bdf[host].device;
		function= pcie_device.scans_bdf[host].function;

        FPcieEcamReadConfigSpace(&pcie_device,bus,device,function,FPCIE_CCR_REV_CLASSID_REGS,&config_data) ;
		class =  config_data >> 8;

        if (class == class_code)
        {
            FPcieEcamReadConfigSpace(&pcie_device,bus,device,function,FPCIE_CCR_ID_REG,&config_data) ;
			vid = FPCIE_CCR_VENDOR_ID_MASK(config_data);
			did = FPCIE_CCR_DEVICE_ID_MASK(config_data);

            FSATA_DEBUG("AHCI-PCI HOST found !!!, b.d.f = %x.%x.%x\n", bus, device, function);

            FPcieEcamReadConfigSpace(&pcie_device,bus,device,function,FPCIE_CCR_BAR_ADDR5_REGS,(u32 *)&bar_addr) ;
            FSATA_DEBUG("FSataPcieIntrInstall BarAddress %p", bar_addr);


            if (0x0 == bar_addr)
            {
                FSATA_ERROR("Bar address: 0x%lx", bar_addr);
                return -1;
            }

            SataPcieIrqInstall(&sata_device[sata_host_count], &pcie_device, bus,device,function);
            sata_device[sata_host_count].config.base_addr = bar_addr;
            sata_host_count++;
            if (sata_host_count >= SATA_HOST_MAX_NUM)
            {
                break;
            }
        }
    }

    /*中断设置，注册中断响应函数,包括pcie与各个sata控制器*/

    /* 对ahci控制器，及以下的port的初始化 */
    ret = FSataAhciInit(&sata_device[instance_id]);
    if (FSATA_SUCCESS != ret)
    {
        FSataCfgDeInitialize(&sata_device[instance_id]);
        FSATA_ERROR("FSataAhciInit sata failed, ret: 0x%x", ret);
        return FSATA_ERR_OPERATION;
    }

    for (port = 0; port < sata_device[instance_id].n_ports; port++)
    {
        if (!(sata_device[ahci_host].link_port_map & BIT(port)))
        {
            printf("ahci_host %d port %d is not link\n", ahci_host, port);
            continue;
        }
        else
        {
            printf("ahci_host %d port %d is link\n", ahci_host, port);
            break;
        }
    }
        
    /* command list address must be 1K-byte aligned */
    ret = FSataAhciPortStart(&sata_device[instance_id], port, (uintptr)mem);
    if (FSATA_SUCCESS != ret)
    {
        FSATA_ERROR("FSataAhciPortStart port %d failed, ret: 0x%x", port, ret);
        return FSATA_ERR_OPERATION;
    }

    ret = FSataAhciReadInfo(&sata_device[instance_id], port);
    if (FSATA_SUCCESS != ret)
    {
        FSataCfgDeInitialize(&sata_device[instance_id]);
        FSATA_ERROR("FSataAhciReadInfo failed, ret: 0x%x", ret);
        return FSATA_ERR_OPERATION;     
    }

    return ret;
}

FError FSataPcieReadWrite(u8 ahci_host, u32 blk, int blk_num, u8 *buffer, u32 trans_type)
{
    FError ret;
    u32 instance_id = ahci_host;

    if (ahci_host >= sata_host_count)
    {
        FSATA_ERROR("ahci_host number is bigger than sata_host_count.");
        return -5;
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
        FSATA_ERROR("Sata pcie failed to read and write data.\r\n");
        return FSATA_ERR_OPERATION;
    }

    return ret;
}

int FSataPcieDeinit(u32 ahci_host)
{
    u32 instance_id = ahci_host;
    /*sata deinit*/
    FSataCfgDeInitialize(&sata_device[instance_id]);
    /*reset the sata_host_count*/
    sata_host_count = 0;
    return 0;
}