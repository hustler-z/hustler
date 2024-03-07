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
 * FilePath: pcie_config_read_example.c
 * Date: 2023-05-09 13:26:58
 * LastEditTime: 2023-05-09 17:23:07
 * Description:  This file is for PCIe config space read functions.
 *
 * Modify History:
 *  Ver       Who        Date         Changes
 * -----    ------     --------    --------------------------------------
 *  1.0   liqiaozhong  2023/5/9    add basic three examples, finish PCIe probe, config 
 *                                 space reading and writing function
 *  1.1   huangheng    2023/8/6    it is implemented with fpcie_ecam interface
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "ftypes.h"
#include "fpcie_ecam.h"
#include "fpcie_ecam_common.h"
#include "pcie_enumerate_example.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "pcie_enumerate_example.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
#define FPCIE_CAP_TYPE0 0   /* PCI Express Capabilities */   
#define FPCIE_CAP_TYPE1 1   /* PCI Express Extended Capabilities */

/************************** Variable Definitions *****************************/
static FPcieEcam pcie_ecam_obj;
/* list of PCIe register info */


static struct FPcieRegInfo regs_start[] = 
{
    {"Device ID [31:16] | Vendor ID [15:0]", FPCIE_CCR_ID_REG},
    {"Status [31:16] | Command [15:0]", FPCIE_CCR_CMD_STATUS_REGS},
    {"Class Code [31:8] | Revision ID [7:0]", FPCIE_CCR_REV_CLASSID_REGS},
    {"BIST [31:24] | Header_Type [23:16] | Latency Timer [15:8] | Cache Line size [7:0]", FPCIE_CCR_CLS_LT_HT_REGS}
} ;


static struct FPcieRegInfo regs_endpoint[] = 
{
    {"Base Address 0 [31:0]", FPCIE_CCR_BAR_ADDR0_REGS},
    {"Base Address 1 [31:0]", FPCIE_CCR_BAR_ADDR1_REGS},
    {"Base Address 2 [31:0]", FPCIE_CCR_BAR_ADDR2_REGS},
    {"Base Address 3 [31:0]", FPCIE_CCR_BAR_ADDR3_REGS},
    {"Base Address 4 [31:0]", FPCIE_CCR_BAR_ADDR4_REGS},
    {"Base Address 5 [31:0]", FPCIE_CCR_BAR_ADDR5_REGS},
    {"CardBus CIS Pointer [31:0]", FPCIE_CCR_CARDBUS_CIS_REGS},
    {"Subsystem Vendor ID [31:16] | Subsystem ID [15:0]", FPCIE_CCR_SUB_ID_REGS},
    {"Expansion ROM Base Address [31:0]", FPCIE_CCR_TYPE0_EXPANSION_ROM_REGS},
    {"Capabilities Pointer [7:0]", FPCIE_CCR_CAPBILITIES_PONINTERS_REGS},
    {"Max_Lat [31:24] | Min_Gnt [23:16] | Interrupt Pin [15:8] | Interrupt Line [7:0]", FPCIE_CCR_INTX_LAT_GNT_REGS},
};

static struct FPcieRegInfo regs_bridge[] =
{
    {"Base Address 0 [31:0]", FPCIE_CCR_BAR_ADDR0_REGS},
    {"Base Address 1 [31:0]", FPCIE_CCR_BAR_ADDR1_REGS},
    {"Secondary Latency [31:24]|Subordinate Bus [23:16] |Secondary Bus [15:8] |Primary Bus[7:0]", FPCIE_CCR_PB_SEC_SUB_SECL_REGS},
    {"Secondary Status [31:16] | I/O Limit [15:8] | I/O Base [7:0]", FPCIE_CCR_IOB_IOL_SECS_REGS},
    {"Memory Limit [31:16] | Memory Base [15:0]", FPCIE_CCR_MEMB_MEML_REGS},
    {"Prefetchable Memory Limit [31:16] | Prefetchable Memory Base [15:0]", FPCIE_CCR_PMEMB_PMEML_REGS},
    {"Prefetchable Base - Upper 32-bits [31:0]", FPCIE_CCR_PREBU32_REGS},
    {"Prefetchable Limit - Upper 32-bits [31:0]", FPCIE_CCR_PRELIMITU32_REGS},
    {"I/O Limit Upper 16-bits [31:16] | I/O Base Upper 16-bits [15:0]", FPCIE_CCR_IOU16_IOL16_REGS},
    {"Expansion ROM Base Address [31:0]", FPCIE_CCR_TYPE1_EXPANSION_ROM_REGS},
    {"Bridge Control [31:16] | Interrupt Pin [15:8] | Interrupt Line [7:0]", FPCIE_CCR_BRIDGE_CONTROL_REGS},
};


static struct FPcieCapabilityInfo capability_list[] =
{
    { FPCI_CAP_LIST_ID, "Capability ID"},
    { FPCI_CAP_ID_PM, "Power Management"},
    { FPCI_CAP_ID_AGP, "Accelerated Graphics Port"},
    { FPCI_CAP_ID_VPD, "Vital Product Data"},
    { FPCI_CAP_ID_SLOTID, "Slot Identification"},
    { FPCI_CAP_ID_MSI, "Message Signalled Interrupts"},
    { FPCI_CAP_ID_CHSWP, "CompactPCI HotSwap"},
    { FPCI_CAP_ID_PCIX, "PCI-X "},
    { FPCI_CAP_ID_HT, "HyperTransport"},
    { FPCI_CAP_ID_VNDR, "Vendor-Specific"},
    { FPCI_CAP_ID_DBG, "Debug port"},
    { FPCI_CAP_ID_CCRC, "CompactPCI Central Resource Control"},
    { FPCI_CAP_ID_SHPC, "PCI Standard Hot-Plug Controller"},
    { FPCI_CAP_ID_SSVID, "Bridge subsystem vendor/device ID"},
    { FPCI_CAP_ID_AGP3, "AGP Target PCI-PCI bridge"},
    { FPCI_CAP_ID_SECDEV, "Secure Device"},
    { FPCI_CAP_ID_EXP, "PCI Express"},
    { FPCI_CAP_ID_MSIX, "MSI-X"},
    { FPCI_CAP_ID_SATA, "SATA Data/Index Conf."},
    { FPCI_CAP_ID_AF, "PCI Advanced Features"},
    { FPCI_CAP_ID_EA, "PCI Enhanced Allocation"},
    {},
};

static struct FPcieCapabilityInfo extend_capability_list[] =
{
    { FPCI_EXT_CAP_ID_ERR, "Advanced Error Reporting"},
    { FPCI_EXT_CAP_ID_VC, "Virtual Channel Capability"},
    { FPCI_EXT_CAP_ID_DSN, "Device Serial Number"},
    { FPCI_EXT_CAP_ID_PWR, "Power Budgeting"},
    { FPCI_EXT_CAP_ID_RCLD, "Root Complex Link Declaration"},
    { FPCI_EXT_CAP_ID_RCILC, "Root Complex Internal Link Control"},
    { FPCI_EXT_CAP_ID_RCEC, "Root Complex Event Collector"},
    { FPCI_EXT_CAP_ID_MFVC, "Multi-Function VC Capability"},
    { FPCI_EXT_CAP_ID_VC9, "same as _VC "},
    { FPCI_EXT_CAP_ID_RCRB, "Root Complex RB?"},
    { FPCI_EXT_CAP_ID_VNDR, "Vendor-Specific"},
    { FPCI_EXT_CAP_ID_CAC, "Config Access - obsolete"},
    { FPCI_EXT_CAP_ID_ACS, "Access Control Services"},
    { FPCI_EXT_CAP_ID_ARI, "Alternate Routing ID"},
    { FPCI_EXT_CAP_ID_ATS, "Address Translation Services"},
    { FPCI_EXT_CAP_ID_SRIOV, "Single Root I/O Virtualization"},
    { FPCI_EXT_CAP_ID_MRIOV, "Multi Root I/O Virtualization"},
    { FPCI_EXT_CAP_ID_MCAST, "Multicast"},
    { FPCI_EXT_CAP_ID_PRI, "Page Request Interface"},
    { FPCI_EXT_CAP_ID_AMD_XXX, "Reserved for AMD"},
    { FPCI_EXT_CAP_ID_REBAR, "Resizable BAR"},
    { FPCI_EXT_CAP_ID_DPA, "Dynamic Power Allocation"},
    { FPCI_EXT_CAP_ID_TPH, "TPH Requester"},
    { FPCI_EXT_CAP_ID_LTR, "Latency Tolerance Reporting"},
    { FPCI_EXT_CAP_ID_SECPCI, "Secondary PCIe Capability"},
    { FPCI_EXT_CAP_ID_PMUX, "Protocol Multiplexing"},
    { FPCI_EXT_CAP_ID_PASID, "Process Address Space ID"},
    { FPCI_EXT_CAP_ID_DPC, "Downstream Port Containment"},
    { FPCI_EXT_CAP_ID_L1SS, "L1 PM Substates"},
    { FPCI_EXT_CAP_ID_PTM, "Precision Time Measurement"},
    {},
};

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/


static void FPcieShowRegs(FPcieEcam *instance_p,u8 bus, u8 device, u8 function, struct FPcieRegInfo *regs,u32 regs_num)
{
    u8  val_8 = 0;
    u16 val_16 = 0;
    u32 val_32 = 0;
    u32 config_data;
    u32 cnt ; 

    for (cnt = 0; cnt < regs_num; cnt++)
    {
        if(FPcieEcamReadConfigSpace(instance_p,bus,device,function,regs[cnt].offset,&config_data)) 
        {
            printf("%s:%d,Failed to read config space", __FUNCTION__, __LINE__) ;
            return ;
        }
        printf("\t0x%0*lx :",8,config_data) ;
        printf("\t%d :",regs[cnt].offset) ;
        printf("\t%s \r\n",regs[cnt].name) ;
        
    }
}

static void FPcieHeaderShow(FPcieEcam *instance_p, u8 bus, u8 device, u8 function)
{
    u32 class_code ;
    u32 config_reg ;
    u8 header_type;
    
    if(FPcieEcamReadConfigSpace(instance_p,bus,device,function,FPCIE_CCR_REV_CLASSID_REGS,&config_reg) != FT_SUCCESS)
    {
        printf("%s:%d,Failed to read config space", __FUNCTION__, __LINE__) ;
        return ;
    } 
 
    class_code = FPCIE_CCR_CLASS_CODE_MASK(config_reg) ;

    /* read header type */
    if(FPcieEcamReadConfigSpace(instance_p,bus,device,function,FPCIE_CCR_CLS_LT_HT_REGS,&config_reg))
    {
        printf("%s:%d,Failed to read config space", __FUNCTION__, __LINE__) ;
        return ;
    }
    header_type = FPCIE_CCR_HEAD_TYPE_MASK(config_reg) ;


    printf("\n\n\tFind an endpoint device %x:%x:%x\n\tHeader info:\r\n",bus,device,function );
    printf("\n\n\tClass code =                  0x%.2x (%s)", (int)class_code, FPcieEcamClassStr(class_code));
    FPcieShowRegs(instance_p, bus, device, function, regs_start,sizeof(regs_start)/sizeof(struct FPcieRegInfo));


    switch (header_type & 0x03)
    {
        case FPCIE_CCR_HEAD_TYPE0:  /* "normal" PCI device */
            FPcieShowRegs(instance_p, bus, device, function, regs_endpoint,sizeof(regs_endpoint)/sizeof(struct FPcieRegInfo));
            break;

        case FPCIE_CCR_HEAD_TYPE1:  /* PCI-to-PCI bridge */
            FPcieShowRegs(instance_p, bus, device, function, regs_bridge,sizeof(regs_bridge)/sizeof(struct FPcieRegInfo));
            break;

        case FPCIE_CCR_HEAD_TYPE2: /* PCI-to-CardBus bridge */
            printf("PCI-to-CardBus bridge not supported\r\n") ;
            break;
    }

}


static void FPcieScanCapability(FPcieEcam *instance_p,u8 bus, u8 device, u8 function,u32 cap_type,struct FPcieCapabilityInfo *cap_list,u32 cap_list_num)
{
    u8 cid;
    for (u32 i = 0; i < cap_list_num; i++)
    {
        cid = cap_list[i].cid ;
        if(FPCIE_CAP_TYPE0 == cap_type)
        {
            if(FPcieEcamHasCapability(instance_p,bus, device, function,cap_list[i].cid))
            {
                printf("	%s %*s", cap_list[i].name, (int)(36 - strlen(cap_list[i].name)), "");
                printf("	cid = 0x%x\n", cid);
            }
        }
        else 
        {
            if(FPcieEcamHasExtendCapability(instance_p,bus, device, function,cap_list[i].cid))
            {
                printf("	%s %*s", cap_list[i].name, (int)(36 - strlen(cap_list[i].name)), "");
                printf("	cid = 0x%x\n", cid);
            }
        }
    }
}

/* function of PCIe config space read example */
int FPcieConfigReadExample(void)
{
    u8 bus,dev,function;
    /* PCIe probe */
    FPcieEcamCfgInitialize(&pcie_ecam_obj, FPcieEcamLookupConfig(FPCIE_ECAM_INSTANCE0),NULL); /* load base addr information of MMIO and IO space */
    printf("\n\tPCI:\n");

    FPcieEcamEnumerateBus(&pcie_ecam_obj, 0); /* scan bus0 and print BDF information of found devices */

    for (u32 i = 0; i < pcie_ecam_obj.scans_bdf_count; i++)
    {
        bus =  pcie_ecam_obj.scans_bdf[i].bus;
        dev =  pcie_ecam_obj.scans_bdf[i].device;
        function = pcie_ecam_obj.scans_bdf[i].function;
        FPcieHeaderShow(&pcie_ecam_obj, bus, dev, function) ;
        printf("\r\n\tPCie capability: \r\n") ;
        FPcieScanCapability(&pcie_ecam_obj, bus, dev, function,FPCIE_CAP_TYPE0,capability_list,sizeof(capability_list)/sizeof(struct FPcieCapabilityInfo)) ;
        printf("\r\n\tPCie extend capability \r\n") ;
        FPcieScanCapability(&pcie_ecam_obj, bus, dev, function,FPCIE_CAP_TYPE1,extend_capability_list,sizeof(extend_capability_list)/sizeof(struct FPcieCapabilityInfo)) ;
    }

    /* print message on example run result */
    printf("\n%s@%d: PCIe config space read example finish.\r\n", __func__, __LINE__);

    return 0;
}