/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: media_test_example.c
 * Date: 2023-08-05 16:05:49
 * LastEditTime: 2023-08-10 16:05:49
 * Description:  This file is for lighting up the screen  and providing a example
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/08/10   Modify the format and establish the version
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fparameters.h"
#include "fdebug.h"
#include "fio.h"
#include "fsleep.h"
#include "fassert.h"
#include "fmmu.h"
#include "fdcdp.h"
#include "fdc.h"
#include "fdp_hw.h"
#include "fdc_hw.h"
#include "fdc_common_hw.h"
#include "media_test_example.h"

/************************** Variable Definitions *****************************/
static FDcDp dcdp_config;
static GraphicsTest blt_buffer;

static u8 *static_frame_buffer_address = (u8 *)0xa0000000 ;

static void FMediaHpdBreakCallback(void *args, u32 index);
static void FMediaHpdConnectCallback(void *args, u32 index);
static void FMediaAuxTimeoutCallback(void *args, u32 index);
static void FMediaAuxErrorCallback(void *args, u32 index);
static void FMediaIrqSet(FDcDp *instance_p);
/************************** Function Prototypes ******************************/
/**
 * @name: FMediaHpdDetect
 * @msg:  detect the hpd status
 * @return Null
 */
void FMediaHpdDetect(void)
{
    FDcDp *instance_p = &dcdp_config;
    u32 ret;
    u32 index;
    u32 mode_id;

    for (index = 0; index < FDCDP_INSTANCE_NUM; index++)
    {
        if (instance_p->connect_flg[index] == 1)
        {
            ret = FDcDpHotPlugConnect(instance_p, index);
            if (ret != FMEDIA_DP_SUCCESS)
            {
                printf("Hotplug failed\r\n");
            }

            instance_p->connect_flg[index] = 0;
        }
    }
}

/**
 * @name: BltVideoToFill
 * @msg:  write the rgb to the dc
 * @param {FDcCtrl} *instance_p is the struct of dc
 * @param {uintptr} offset is the addr
 * @param {u32} length is the length of the pixel
 * @param {void*} config is rgb value
 * @return Null
 */
static void PhyFramebufferWrite(FDcCtrl *instance_p, uintptr offset, u32 length,  void *config)
{
    u32 Index;
    for (Index = 0; Index < length; Index++)
    {
       FtOut32(dcdp_config.user_config->fb_virtual + offset + Index * 4, *((u32 *)(config + Index * 4))) ;
    }

    return;

}




/**
 * @name: BltVideoToFill
 * @msg:  fill the rgb into the dc
 * @param {FDcCtrl} *instance_p is the struct of dc
 * @param {GraphicsTest} config is the RGB value
 * @param {u32} width is the width of screen
 * @return Null
 */
static void BltVideoToFill(FDcCtrl *instance_p, GraphicsTest *config, u32 width, u32 height)
{
    FASSERT(instance_p  != NULL);
    FASSERT(config  != NULL);

    u32  ResWidth;
    u32  ResHeight;
    u32  Stride;
    u32  I;
    u32  J;
    u32  Blt;

    Stride = FDcWidthToStride(width, 32, 1);
    memcpy(&Blt, config, sizeof(GraphicsTest));
    for (I = 0; I < (height); I++)
    {
        for (J = 0; J < (width * 2); J++)
        {
            PhyFramebufferWrite(instance_p, I * Stride + J * 4, 1, &Blt);
        }
    }
}

/**
 * @name: FMediaHpdConnectCallback
 * @msg:  the hpd connect event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FMediaHpdConnectCallback(void *args, u32 index)
{
    FASSERT(args != NULL);

    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, FDP_TX_INTERRUPT); /*clear interrupt*/

    FDcDpHotPlug(instance_p, index, FDCDP_DISCONNCET_TO_CONNECT);

    instance_p->connect_flg[index] = 1;
}

/**
 * @name: FMediaHpdBreakCallback
 * @msg:  the hpd disconnect event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FMediaHpdBreakCallback(void *args, u32 index)
{
    FASSERT(args != NULL);

    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, FDP_TX_INTERRUPT); /*clear interrupt*/

    FDcDpHotPlug(instance_p, index, FDCDP_CONNECT_TO_DISCONNCET);

    instance_p->connect_flg[index] = 0;
}

/**
 * @name: FMediaAuxTimeoutCallback
 * @msg:  the aux timeout  event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FMediaAuxTimeoutCallback(void *args, u32 index)
{
    FASSERT(args != NULL);

    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, FDP_TX_INTERRUPT); /*clear interrupt*/

    printf("Dp:%d aux connect timeout\r\n", index);

}

/**
 * @name: FMediaAuxErrorCallback
 * @msg:  the aux error  event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FMediaAuxErrorCallback(void *args, u32 index)
{
    FASSERT(args != NULL);

    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, FDP_TX_INTERRUPT); /*clear interrupt*/

    printf("Dp:%d aux connect error\r\n", index);

}

/**
 * @name: FMediaIrqSet
 * @msg:  set the irq event and instance
 * @param {FDcDp} *instance_p is the instance of dcdp
 * @return Null
 */
static void FMediaIrqSet(FDcDp *instance_p)
{
    FASSERT(instance_p != NULL);

    u32 cpu_id;
    u32 index;
    FMediaIntrConfig intr_config;
    memset(&intr_config, 0, sizeof(intr_config));

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(instance_p->dp_instance_p[0].config.irq_num, cpu_id);/*the dc0 and dc1 have the same num of irq_num*/

    FDcDpRegisterHandler(instance_p, FDCDP_HPD_IRQ_CONNECTED, FMediaHpdConnectCallback, (void *)instance_p);
    FDcDpRegisterHandler(instance_p, FDCDP_HPD_IRQ_DISCONNECTED, FMediaHpdBreakCallback, (void *)instance_p);
    FDcDpRegisterHandler(instance_p, FDCDP_AUX_REPLY_TIMEOUT, FMediaAuxTimeoutCallback, (void *)instance_p);
    FDcDpRegisterHandler(instance_p, FDCDP_AUX_REPLY_ERROR, FMediaAuxErrorCallback, (void *)instance_p);

    InterruptSetPriority(instance_p->dp_instance_p[0].config.irq_num, 0);/*the dp0 and dp1 have the same irq_num*/
    InterruptInstall(instance_p->dp_instance_p[0].config.irq_num, FDcDpInterruptHandler, instance_p, "media");
    InterruptUmask(instance_p->dp_instance_p[0].config.irq_num);
}

/**
 * @name: FDcDpIrqAllEnable
 * @msg:  enable the irq
 * @param  {FDcDp} *instance_p is the instance of dcdp
 * @return Null
 */
static void FDcDpIrqAllEnable(FDcDp *instance_p)
{
    int index = 0;
    FDcDpIntrEventType event_type = FDCDP_HPD_IRQ_CONNECTED;
    for (index = 0; index < FDCDP_INSTANCE_NUM; index++)
    {
        for (event_type = 0; event_type < FDCDP_INSTANCE_NUM; event_type++)
        {
            FDcDpIrqEnable(instance_p, index, event_type);
        }
    }
}

/**
 * @name: FMediaInterruptInit
 * @msg:  test the irq
 * @param  {FDcDp} *instance_p is the instance of dcdp
 * @return Null
 */
void FMediaInterruptInit(void)
{
    /* set media irq handler */
    FMediaIrqSet(&dcdp_config);
    FDcDpIrqAllEnable(&dcdp_config);
}

/**
 * @name: FMediaInterruptDeinit
 * @msg:  disable the irq
 * @return Null
 */
void FMediaInterruptDeinit(void)
{
    /* interrupt deinit,dp0 and dp1 have the same irq_num */
    InterruptMask(dcdp_config.dp_instance_p[0].config.irq_num);
}

/**
 * @name: FMediaCtrlProbe
 * @msg:  init the media control
 * @return ret,FMEDIA_DP_SUCCESS means success
 */
FError FMediaCtrlProbe(void)
{
    FError ret = FMEDIA_DCDP_SUCCESS;
    u32 index;
    FDcDpCfgInitialize(&dcdp_config);

    for (index = 0; index < FDCDP_INSTANCE_NUM; index ++)
    {
        dcdp_config.dc_instance_p[index].config = *FDcLookupConfig(index);
        dcdp_config.dp_instance_p[index].config = *FDpLookupConfig(index);
    }
    return ret;
}

/**
 * @name: FMediaDisplayDemo
 * @msg:  the demo for testing the media
 * @return Null
 */
void FMediaDisplayDemo(void)
{
    FDcDp *instance_p = &dcdp_config;
    for (u32 index = 0; index < FDCDP_INSTANCE_NUM; index ++)
    {
        blt_buffer.Red = 0xff;
        blt_buffer.Green = 0xff;
        blt_buffer.Blue = 0x0;
        blt_buffer.reserve = 0;
        BltVideoToFill(&dcdp_config.dc_instance_p[index], &blt_buffer, 640, 480);
    }
    return ;
}

/**
 * @name: FMediaChannelDeinit
 * @msg:  deinit the dc
 * @param {u32} id is the channel_num of dc
 * @return Null
 */
FError FMediaChannelDeinit(u32 id)
{
    FError ret;
    ret = FDcDpDeInitialize(&dcdp_config, id);
    return ret;
}

/**
 * @name: FMediaDebug
 * @msg:  debug the dc
 * @param {u32} id is the channel_num of dc
 * @return Null
 */
void FMediaDebug(u32 id)
{
    u32 index;
    if (id == FDCDP_INSTANCE_NUM)
    {
        for (index = 0; index < FDCDP_INSTANCE_NUM; index ++)
        {
            FDcDump(dcdp_config.dc_instance_p[index].config.dcch_baseaddr);
            FDpDump(dcdp_config.dp_instance_p[index].config.dp_channe_base_addr);
        }
    }
    else
    {
        FDcDump(dcdp_config.dc_instance_p[id].config.dcch_baseaddr);
        FDpDump(dcdp_config.dp_instance_p[id].config.dp_channe_base_addr);
    }
}

FError FMediaExample(void)
{
    FError ret = FMEDIA_DCDP_SUCCESS;
    u32 index;
    u32 start_index;
    u32 end_index;

    /*设置用户参数*/
    u32 channel = 2;/* 0 or 1 or 2*/
    if (channel == FDCDP_INSTANCE_NUM)
    {
        start_index = 0;
        end_index = FDCDP_INSTANCE_NUM;
    }
    else
    {
        start_index = channel;
        end_index = channel + 1;
    }
    for (index = start_index; index < end_index; index ++)
    {
        dcdp_config.user_config[index].color_depth = 32;
        dcdp_config.user_config[index].width = 640;
        dcdp_config.user_config[index].height = 480;
        dcdp_config.user_config[index].refresh_rate = 60;
        dcdp_config.user_config[index].multi_mode = 0;
        dcdp_config.user_config[index].fb_phy = (uintptr)static_frame_buffer_address;
        dcdp_config.user_config[index].fb_virtual = (uintptr)static_frame_buffer_address ;/*当前例程虚拟地址和物理地址一致，实际需要根据需要进行映射*/
    }
    ret = FMediaCtrlProbe();
    if (ret != FMEDIA_DCDP_SUCCESS)
    {
        printf("FMediaCtrlProbe probe error!\n");
        goto err;
    }
    ret = FDcDpInitialize(&dcdp_config, channel);
    if (ret != FMEDIA_DCDP_SUCCESS)
    {
        printf("FDcDpInitialize  error!\n");
        goto err;
    }

    FMediaDisplayDemo();
    FMediaInterruptInit();

err:
    /* print message on example run result */
    if (FMEDIA_DCDP_SUCCESS == ret)
    {
        printf("%s@%d: Serial poll example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Serial poll example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }
    return ret;
}
