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
 * FilePath: fi2c_master_slave_example.c
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:41:20
 * Description:  This file is for  providing functions to file cmd_fi2c_master_slave.c
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/02/17    first commit
 */

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "fparameters.h"
#include "fkernel.h"
#include "ftypes.h"
#include "fdebug.h"

#include "fsleep.h"
#include "fi2c.h"
#include "fi2c_hw.h"

#include "fmio_hw.h"
#include "fmio.h"

#include "finterrupt.h"
#include "fcpu_info.h"
#include "ferror_code.h"
#include "i2c_master_slave_example.h"
#include "fio_mux.h"
/***************** Macros (Inline Functions) Definitions *********************/
#define FI2CMS_DEBUG_TAG "I2C-MASTER-SLAVE"
#define FI2CMS_ERROR(format, ...)   FT_DEBUG_PRINT_E(FI2CMS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2CMS_WARN(format, ...)    FT_DEBUG_PRINT_W(FI2CMS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2CMS_INFO(format, ...)    FT_DEBUG_PRINT_I(FI2CMS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2CMS_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FI2CMS_DEBUG_TAG, format, ##__VA_ARGS__)

#ifndef CONFIG_TARGET_E2000
    #error "This example support only E2000 D/Q/S !!!"
#endif

#if defined(CONFIG_FIREFLY_DEMO_BOARD)
#define MASTER_MIO FMIO1_ID
#define SLAVE_MIO FMIO2_ID
#else
#define MASTER_MIO FMIO0_ID
#define SLAVE_MIO FMIO1_ID
#endif

static FI2c master_device;

typedef struct data
{
    FI2c device;
    boolean first_write;
    u32 buff_idx;
    u8 buff[IO_BUF_LEN];
} FI2cSlaveData;

FI2cSlaveData slave;

static FMioConfig master_mio_config;
static FMioCtrl master_mio_ctrl;
static FMioConfig slave_mio_config;
static FMioCtrl slave_mio_ctrl;

static u8 write_buf[IO_BUF_LEN] __attribute__((aligned(4)));
static u8 read_buf[IO_BUF_LEN] __attribute__((aligned(4)));

void FI2cSlaveCb(void *instance_p, void *para, u32 evt)
{
    FI2cSlaveData *slave_p = &slave;
    u8 *val = (u8 *)para;
    /*
    *Do not increment buffer_idx here,because we set maximum lenth is IO_BUF_LEN
    */
    if (slave_p->buff_idx >= IO_BUF_LEN)
    {
        slave_p->buff_idx = slave_p->buff_idx % IO_BUF_LEN;
    }
    switch (evt)
    {
        case FI2C_EVT_SLAVE_WRITE_RECEIVED:
            if (slave_p->first_write)
            {
                slave_p->buff_idx = *val;
                slave_p->first_write = FALSE;
            }
            else
            {
                slave_p->buff[slave_p->buff_idx++] = *val;
            }

            break;
        case FI2C_EVT_SLAVE_READ_PROCESSED:
            /* The previous byte made it to the bus, get next one */
            slave_p->buff_idx++;
            /* fallthrough */
            break;
        case FI2C_EVT_SLAVE_READ_REQUESTED:
            *val = slave_p->buff[slave_p->buff_idx++];
            break;
        case FI2C_EVT_SLAVE_STOP:
        case FI2C_EVT_SLAVE_WRITE_REQUESTED:
            slave_p->first_write = TRUE;
            break;
        default:
            break;
    }

    return;
}

void FI2cSlaveWriteReceived(void *instance_p, void *para)
{
    FI2cSlaveCb(instance_p, para, FI2C_EVT_SLAVE_WRITE_RECEIVED);
}

void FI2cSlaveReadProcessed(void *instance_p, void *para)
{
    FI2cSlaveCb(instance_p, para, FI2C_EVT_SLAVE_READ_PROCESSED);
}

void FI2cSlaveReadRequest(void *instance_p, void *para)
{
    FI2cSlaveCb(instance_p, para, FI2C_EVT_SLAVE_READ_REQUESTED);
}

void FI2cSlaveStop(void *instance_p, void *para)
{
    FI2cSlaveCb(instance_p, para, FI2C_EVT_SLAVE_STOP);
}

void FI2cSlaveWriteRequest(void *instance_p, void *para)
{
    FI2cSlaveCb(instance_p, para, FI2C_EVT_SLAVE_WRITE_REQUESTED);
}


FError FI2cMioSlaveInit(u32 address, u32 speed_rate)
{
    FI2cConfig input_cfg;
    const FI2cConfig *config_p = NULL;
    FI2cSlaveData *slave_p = &slave;
    FI2c *instance_p = &slave_p->device;
    FError status = FI2C_SUCCESS;

    /* MIO init */
    slave_mio_ctrl.config = *FMioLookupConfig(SLAVE_MIO);
    status = FMioFuncInit(&slave_mio_ctrl, FMIO_FUNC_SET_I2C);
    if (status != FT_SUCCESS)
    {
        FI2CMS_ERROR("MIO initialize error.");
        return ERR_GENERAL;
    }
    /* init mio fuction */
    FIOPadSetMioMux(SLAVE_MIO);
    memset(slave_p, 0, sizeof(*slave_p));
    slave_p->first_write = TRUE;

    /* Lookup default configs by instance id */
    config_p = FI2cLookupConfig(0);/* get a default reference for MIO config */
    if (NULL == config_p)
    {
        FI2CMS_ERROR("Config of mio instance %d non found.\r\n", SLAVE_MIO);
        return FI2C_ERR_INVAL_PARM;
    }

    /* Modify configuration */
    input_cfg = *config_p;
    input_cfg.instance_id = SLAVE_MIO;
    input_cfg.base_addr = FMioFuncGetAddress(&slave_mio_ctrl, FMIO_FUNC_SET_I2C);
    input_cfg.irq_num = FMioFuncGetIrqNum(&slave_mio_ctrl, FMIO_FUNC_SET_I2C);
    input_cfg.ref_clk_hz = FMIO_CLK_FREQ_HZ;
    input_cfg.work_mode = FI2C_SLAVE;
    input_cfg.slave_addr = address; /* just assign one address as id */
    input_cfg.speed_rate = speed_rate;

    /* Initialization */
    status = FI2cCfgInitialize(instance_p, &input_cfg);
    if (FI2C_SUCCESS != status)
    {
        FI2CMS_ERROR("Init mio slave failed, ret: 0x%x\r\n", status);
        return status;
    }

    FI2cSlaveRegisterIntrHandler(instance_p, FI2C_EVT_SLAVE_WRITE_RECEIVED, FI2cSlaveWriteReceived);
    FI2cSlaveRegisterIntrHandler(instance_p, FI2C_EVT_SLAVE_READ_PROCESSED, FI2cSlaveReadProcessed);
    FI2cSlaveRegisterIntrHandler(instance_p, FI2C_EVT_SLAVE_READ_REQUESTED, FI2cSlaveReadRequest);
    FI2cSlaveRegisterIntrHandler(instance_p, FI2C_EVT_SLAVE_STOP, FI2cSlaveStop);
    FI2cSlaveRegisterIntrHandler(instance_p, FI2C_EVT_SLAVE_WRITE_REQUESTED, FI2cSlaveWriteRequest);

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(input_cfg.irq_num, cpu_id);

    /* umask mio irq */
    InterruptSetPriority(input_cfg.irq_num, input_cfg.irq_prority);
    /* register intr callback */
    InterruptInstall(input_cfg.irq_num, FI2cSlaveIntrHandler, instance_p, "fi2cslave");
    /* slave mode intr set */
    status = FI2cSlaveSetupIntr(instance_p);
    /* enable irq */
    InterruptUmask(input_cfg.irq_num);

    if (FI2C_SUCCESS != status)
    {
        FI2CMS_ERROR("Setup mio slave interrupt failed, ret: 0x%x\r\n", status);
        return status;
    }

    return status;
}

FError FI2cMioMasterInit(u32 address, u32 speed_rate)
{
    FI2cConfig input_cfg;
    const FI2cConfig *config_p = NULL;
    FI2c *instance_p = &master_device;
    FError status = FI2C_SUCCESS;

    /* MIO init */
    master_mio_ctrl.config = *FMioLookupConfig(MASTER_MIO);
    status = FMioFuncInit(&master_mio_ctrl, FMIO_FUNC_SET_I2C);
    if (status != FT_SUCCESS)
    {
        FI2CMS_ERROR("MIO initialize error.");
        return ERR_GENERAL;
    }

    FIOPadSetMioMux(MASTER_MIO);

    memset(instance_p, 0, sizeof(*instance_p));
    /* Lookup default configs by instance id */
    config_p = FI2cLookupConfig(0);/* get a default reference for MIO config */
    if (NULL == config_p)
    {
        FI2CMS_ERROR("Config of mio instance %d non found.", MASTER_MIO);
        return FI2C_ERR_INVAL_PARM;
    }

    /* Modify configuration */
    input_cfg = *config_p;
    input_cfg.instance_id = MASTER_MIO;
    input_cfg.base_addr = FMioFuncGetAddress(&master_mio_ctrl, FMIO_FUNC_SET_I2C);
    input_cfg.irq_num = FMioFuncGetIrqNum(&master_mio_ctrl, FMIO_FUNC_SET_I2C);
    input_cfg.ref_clk_hz = FMIO_CLK_FREQ_HZ;
    input_cfg.slave_addr = address;
    input_cfg.speed_rate = speed_rate;

    /* Initialization */
    status = FI2cCfgInitialize(instance_p, &input_cfg);

    /*  callback function for FI2C_MASTER_INTR_EVT interrupt  */
    instance_p->master_evt_handlers[FI2C_EVT_MASTER_TRANS_ABORTED] = NULL;
    instance_p->master_evt_handlers[FI2C_EVT_MASTER_READ_DONE] = NULL;
    instance_p->master_evt_handlers[FI2C_EVT_MASTER_WRITE_DONE] = NULL;

    if (FI2C_SUCCESS != status)
    {
        FI2CMS_ERROR("Init mio master failed, ret: 0x%x", status);
        return status;
    }

    printf("Set target slave_addr: 0x%x with mio-%d\r\n", input_cfg.slave_addr, MASTER_MIO);
    return status;
}

FError FI2cMasterwrite(const u8 *buf_p, u32 buf_len, u32 inchip_offset)
{
    FError status = FI2C_SUCCESS;
    FI2c *instance_p = &master_device;

    if (buf_len < IO_BUF_LEN && inchip_offset < IO_BUF_LEN)
    {
        /* code */
        if ((IO_BUF_LEN - inchip_offset) < buf_len)
        {
            FI2CMS_ERROR("Write to eeprom failed, out of eeprom size.");
            return FI2C_ERR_INVAL_PARM;
        }
    }
    else
    {
        FI2CMS_ERROR("Write to eeprom failed, out of eeprom size.");
        return FI2C_ERR_INVAL_PARM;
    }

    status = FI2cMasterWritePoll(instance_p, inchip_offset, VIRTUAL_EEPROM_MEM_BYTE_LEN, buf_p, buf_len);

    if (FI2C_SUCCESS != status)
    {
        FI2CMS_ERROR("Write to eeprom failed, ret: 0x%x", status);
        return status;
    }

    return status;
}

FError FI2cMasterRead(u8 *buf_p, u32 buf_len, u32 inchip_offset)
{
    FI2c *instance_p = &master_device;
    FError status = FI2C_SUCCESS;
    FASSERT(buf_len);

    memset(buf_p, 0, buf_len);
    status = FI2cMasterReadPoll(instance_p,
                                inchip_offset,
                                VIRTUAL_EEPROM_MEM_BYTE_LEN,
                                buf_p,
                                buf_len);
    return status;
}

FError FI2cSlaveDeinit(void)
{
    FError ret;
    /* deinit mio func */
    ret = FMioFuncDeinit(&slave_mio_ctrl);
    if (ret != FT_SUCCESS)
    {
        FI2CMS_ERROR("MIO deinit error!");
        return ret;
    }
    
    FI2c *instance_p = &slave.device;
    InterruptMask(instance_p->config.irq_num);
    FI2cDeInitialize(instance_p);
    return FT_SUCCESS;
}

FError FI2cMasterDeinit(void)
{
    FError ret;
    /* deinit mio func */
    ret = FMioFuncDeinit(&master_mio_ctrl);
    if (ret != FT_SUCCESS)
    {
        FI2CMS_ERROR("MIO deinit error!");
        return ret;
    }
    FI2c *instance_p = &master_device;
    FI2cDeInitialize(instance_p);
    return FT_SUCCESS;
}

void FI2cSlaveDump(void)
{
    FI2cSlaveData *slave_p = &slave;
    printf("buf size: %d, buf idx: %d\r\n", sizeof(slave_p->buff), slave_p->buff_idx);
    FtDumpHexByte(slave_p->buff, IO_BUF_LEN);
}

FError FI2cMasterSlaveExample(void)
{
    FError ret = 0;
    u32 address = 0x01;
    u32 speed_rate = FI2C_SPEED_STANDARD_RATE;/*kb/s*/

    FIOMuxInit();

    ret = FI2cMioSlaveInit(address, speed_rate);
    if (ret != FT_SUCCESS)
    {
        FI2CMS_ERROR("FI2cMioSlaveInit mio_id :%d is error!\n",SLAVE_MIO);
        goto err;
    }

    ret = FI2cMioMasterInit(address, speed_rate);
    if (FT_SUCCESS != ret)
    {
        FI2CMS_ERROR("FI2cMioMasterInit mio_id :%d is error!\n",MASTER_MIO);
        goto err;
    }

    u32 offset = 0x05;
    const char input[] = {"0123456789"};
    u32 input_len = strlen(input);
    strncpy(write_buf, input, input_len);
    printf("write 0x%x len %d\r\n", offset, input_len);
    ret = FI2cMasterwrite(write_buf, input_len, offset);
    if (FI2C_SUCCESS != ret)
    {
        FI2CMS_ERROR("FI2cMasterwrite error!\n");
        goto err;
    }

    FI2cSlaveDump();

    ret = FI2cMasterRead(read_buf, input_len, offset);
    if (FI2C_SUCCESS == ret)
    {
        printf("\r\nRead 0x%x len %d:%s.\r\n", offset, input_len,read_buf);
        FtDumpHexByte(read_buf, input_len);
    }
    ret = FI2cMasterDeinit();
    if (FI2C_SUCCESS != ret)
    {
        FI2CMS_ERROR("FI2cMasterDeinit error!\n");
        goto err;
    }
    /* print message on example run result */
err:
    if (0 == ret)
    {
        printf("%s@%d: I2C master-slave example [success]\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: I2C master-slave example [failure]\r\n", __func__, __LINE__);
    }

    return 0;
}
