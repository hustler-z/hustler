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
 * FilePath: lvgl_disp_test.c
 * Created Date: 2023-06-07 09:19:18
 * Last Modified: 2023-10-31 16:45:42
 * Description:  This file is for test the lvgl demo
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0       Wangzq     2023/06/09         Modify the format and establish the version
 */
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "ftypes.h"
#include "fparameters.h"
#include "ferror_code.h"
#include "fassert.h"
#include "finterrupt.h"
#include "fcpu_info.h"

#include "fdcdp.h"
#include "fdc.h"
#include "fdp_hw.h"
#include "fdc_hw.h"
#include "fdc_common_hw.h"
#include "lv_port_disp.h"

#include "lvgl_disp_test.h"


/************************** static definition ******************************/
static FDcDp dcdp_config;
static u8 *static_frame_buffer_address = (u8 *)0xa0000000 ;

static void FMediaHpdBreakCallback(void *args, u32 index);
static void FMediaHpdConnectCallback(void *args, u32 index);
static void FMediaAuxTimeoutCallback(void *args, u32 index);
static void FMediaAuxErrorCallback(void *args, u32 index);
static void FMediaIrqSet(FDcDp *instance_p);

/************************** Function Prototypes ******************************/
/**
 * @name: lv_hpd_detect
 * @msg:  to detect the hpd event
 * @return Null
 */
void lv_hpd_detect(void)
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
 * @name: FMediaAuxRecievedCallback
 * @msg:  the aux replay intr
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FMediaAuxRecievedCallback(void *args, u32 index)
{
    FASSERT(args != NULL);
    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, FDP_TX_INTERRUPT); /*clear interrupt*/
    printf("Dp:%d aux reply recieved\r\n", index);
}

/**
 * @name: FMediaCtrlProbe
 * @msg:  init the media control
 * @return ret,FMEDIA_DP_SUCCESS means success
 */
FError FMediaCtrlProbe(void)
{
    FError ret;
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
 * @name: FMediaInterrupInit
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
 * @name: FMediaChannelDeinit
 * @msg:  deinit the dc
 * @param {u32} id is the channel_num of dc
 * @return Null
 */
FError FMediaChannelDeinit(u32 id)
{
    FError ret;
    ret =  FDcDpDeInitialize(&dcdp_config, id);
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

/**
 * @name: FMediaDispInit
 * @msg:  init the media
 * @param {u32} channel_num is the channel of dc
 * @param {u32} width is the width of sync
 * @param {u32} height is the height of sync
 * @param {u32} multi_mode is the multi_mode of sync
 * @return ret,FMEDIA_DP_SUCCESS means success
 */
FError FMediaDispInit(void)
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
        dcdp_config.user_config[index].fb_virtual = (uintptr)static_frame_buffer_address;
    }

    ret = FMediaCtrlProbe();
    if (ret != FMEDIA_DCDP_SUCCESS)
    {
        printf("FMediaCtrlProbe probe error!\n");
    }
    ret = FDcDpInitialize(&dcdp_config, channel);
    if (ret != FMEDIA_DCDP_SUCCESS)
    {
        printf("FDcDpInitialize  error!\n");
    }
    FMediaInterruptInit();
    return ret;
}

FError FMediaLvConfig(void)
{
    FError ret = FMEDIA_DP_SUCCESS;
    u32 index;
    FMediaLvgldispInit();
    for (index = 0; index < FDCDP_INSTANCE_NUM; index ++)
    {
        FMediaDispFramebuffer(&dcdp_config);
    }


    return ret;
}

void FMediaLvInit(void)
{
    lv_init();
    return;
}




