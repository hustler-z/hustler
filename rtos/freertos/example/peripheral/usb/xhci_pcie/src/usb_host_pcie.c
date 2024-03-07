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
 * FilePath: usb_host.c
 * Date: 2022-07-22 13:57:42
 * LastEditTime: 2022-07-22 13:57:43
 * Description:  This file is for the usb host functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/9/20  init commit
 * 2.0   zhugengyu  2024/1/17  support pcie xhci host
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "fassert.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fdebug.h"
#include "fcache.h"
#include "fmemory_pool.h"

#include "fpcie_ecam.h"
#include "fpcie_ecam_common.h"

#include "usbh_core.h"
/************************** Constant Definitions *****************************/
#define FUSB_MEMP_TOTAL_SIZE     SZ_1M
#define FPCIE_INSTANCE_ID        FPCIE_ECAM_INSTANCE0

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FMemp memp;
static u8 memp_buf[FUSB_MEMP_TOTAL_SIZE];
static struct usbh_bus usb;
static uintptr usb_base = 0U;
static FPcieEcam pcie_device;

/***************** Macros (Inline Functions) Definitions *********************/
#define FUSB_DEBUG_TAG "USB-HC"
#define FUSB_ERROR(format, ...) FT_DEBUG_PRINT_E(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_WARN(format, ...)  FT_DEBUG_PRINT_W(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_INFO(format, ...)  FT_DEBUG_PRINT_I(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_DEBUG(format, ...) FT_DEBUG_PRINT_D(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
extern void USBH_IRQHandler(void *);

/*****************************************************************************/
static void UsbHcPcieInterrruptHandler(void *param)
{
    USBH_IRQHandler(param);
}

static void PCieIntxInit(FPcieEcam *instance_p)
{
    u32 cpu_id;
    u32 irq_num = FPCIE_ECAM_INTA_IRQ_NUM;
    u32 irq_priority = 13U;

    (void)GetCpuId(&cpu_id);
    FUSB_DEBUG("interrupt num: %d", irq_num);
    (void)InterruptSetTargetCpus(irq_num, cpu_id);

    InterruptSetPriority(irq_num, irq_priority);

    /* register intr callback */
    InterruptInstall(irq_num,
                     FPcieEcamIntxIrqHandler,
                     &pcie_device,
                     NULL);

    /* enable irq */
    InterruptUmask(irq_num);
}

static FError PcieInit(FPcieEcam *pcie_device)
{
    FError ret = FT_SUCCESS;

    ret = FPcieEcamCfgInitialize(pcie_device, FPcieEcamLookupConfig(FPCIE_INSTANCE_ID), NULL);
    if (FT_SUCCESS != ret)
    {
        return ret;
    }
    
    FUSB_DEBUG("\n");
    FUSB_DEBUG("	PCI:\n");
    FUSB_DEBUG("	B:D:F			VID:PID			parent_BDF			class_code\n");
    ret = FPcieEcamEnumerateBus(pcie_device, 0);
    if (FT_SUCCESS != ret)
    {
        return ret;
    }

    PCieIntxInit(pcie_device);    /* register pcie_device intx handler */
}

static FError USBPcieIrqInstall(FPcieEcam *pcie_device, u8 bus, u8 device, u8 function)
{
    FError ret = FT_SUCCESS;
    FPcieIntxFun intx_fun;
    intx_fun.IntxCallBack = UsbHcPcieInterrruptHandler;
    intx_fun.args = &usb;
    intx_fun.bus = bus;
	intx_fun.device = device;
	intx_fun.function = function;

    ret = FPcieEcamIntxRegister(pcie_device, bus, device, function, &intx_fun);
    if (FT_SUCCESS != ret)
    {
        FUSB_ERROR("FPcieIntxRegiterIrqHandler failed.\n");
        return ret;
    }

    return ret;
}

void UsbHcSetupMemp(void)
{
    if (FT_COMPONENT_IS_READY != memp.is_ready)
    {
        USB_ASSERT(FT_SUCCESS == FMempInit(&memp, &memp_buf[0], &memp_buf[0] + FUSB_MEMP_TOTAL_SIZE));
    }
}

/* implement cherryusb weak functions */
void usb_hc_low_level_init(uint32_t id)
{
    FError ret = FT_SUCCESS;
    u32 instance_id = id;
    s32 host;
    u32 bdf;
    u32 class;
    u16 pci_command;
    u8 bus,device,function;
    u16 vid, did;
    uintptr bar0_addr = 0;
    uintptr bar1_addr = 0;
    const u32 class_code = FPCI_CLASS_SERIAL_USB_XHCI; /* sub class and base class definition */
    u32 config_data;

    UsbHcSetupMemp();

    ret = PcieInit(&pcie_device);
    if (FT_SUCCESS != ret)
    {
        FUSB_ERROR("FPcieInit failed.\n");
        FASSERT(0);
        return;
    }

    /* find xhci host from pcie_device instance */
    for (host = 0; host < pcie_device.scans_bdf_count; host++)
    {
        bus		= pcie_device.scans_bdf[host].bus;
		device	= pcie_device.scans_bdf[host].device;
		function= pcie_device.scans_bdf[host].function;

        FPcieEcamReadConfigSpace(&pcie_device,bus,device,function,FPCIE_CCR_REV_CLASSID_REGS,&config_data);
		class =  config_data >> 8;

        if (class == class_code)
        {
            (void)FPcieEcamReadConfigSpace(&pcie_device,bus,device,function,FPCIE_CCR_ID_REG,&config_data);
			vid = FPCIE_CCR_VENDOR_ID_MASK(config_data);
			did = FPCIE_CCR_DEVICE_ID_MASK(config_data);            

            FUSB_DEBUG("xHCI-PCI HOST found !!!, b.d.f = %x.%x.%x\n", bus, device, function);
            FPcieEcamReadConfigSpace(&pcie_device,bus,device,function,FPCIE_CCR_BAR_ADDR0_REGS,(u32 *)&bar0_addr);
            bar0_addr &= ~0xfff;

#if defined(FAARCH64_USE)
            FPcieEcamReadConfigSpace(&pcie_device,bus,device,function,FPCIE_CCR_BAR_ADDR1_REGS,(u32 *)&bar1_addr);
#endif

            FUSB_DEBUG("FSataPcieIntrInstall BarAddress %p:%p", bar1_addr, bar0_addr);

            if ((0x0 == bar0_addr) && (0x0 == bar1_addr))
            {
                FUSB_ERROR("Invalid Bar address");
                FASSERT(0);
                return;
            }        

            USBPcieIrqInstall(&pcie_device, bus, device, function);
            usb_base = (bar1_addr << 32U) | bar0_addr;
            FUSB_INFO("xHCI base address: 0x%lx", usb_base);
        }                
    }

}

unsigned long usb_hc_get_register_base(uint32_t id)
{
    return usb_base;
}

void *usb_hc_malloc(size_t size)
{
    return usb_hc_malloc_align(sizeof(void *), size);
}

void *usb_hc_malloc_align(size_t align, size_t size)
{
    void *result = FMempMallocAlign(&memp, size, align);

    if (result)
    {
        memset(result, 0U, size);
    }

    return result;
}

void usb_hc_free(void *ptr)
{
    if (NULL != ptr)
    {
        FMempFree(&memp, ptr);
    }
}

void usb_assert(const char *filename, int linenum)
{
    FAssert(filename, linenum, 0xff);
}

void usb_hc_dcache_invalidate(void *addr, unsigned long len)
{
    FCacheDCacheInvalidateRange((uintptr)addr, len);
}
/*****************************************/

static void UsbInitTask(void *args)
{
    u32 id = (u32)(uintptr)args;
    memset(&usb, 0 , sizeof(usb));

    if (0 == usbh_initialize(id, &usb))
    {
        printf("Init cherryusb host successfully.put 'usb lsusb -t' to see devices.\r\n");
    }
    else
    {
        FUSB_ERROR("Init cherryusb host failed.");
    }

    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSInitUsb(u32 id)
{
    BaseType_t ret = pdPASS;

    taskENTER_CRITICAL(); /* no schedule when create task */

    ret = xTaskCreate((TaskFunction_t)UsbInitTask,
                      (const char *)"UsbInitTask",
                      (uint16_t)2048,
                      (void *)(uintptr)id,
                      (UBaseType_t)configMAX_PRIORITIES - 1,
                      NULL);
    FASSERT_MSG(pdPASS == ret, "create task failed");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    return ret;
}

BaseType_t FFreeRTOSListUsbDev(int argc, char *argv[])
{
    return lsusb(argc, argv);
}