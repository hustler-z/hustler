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
 * FilePath: sgi_example.c
 * Date: 2023-04-26 08:37:22
 * LastEditTime: 2022-04-27 11:00:53
 * Description:  This file is for the sgi test example functions.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------     --------------------------------------
 * 1.0   liqiaozhong   2023/4/27    first commit and add spi, sgi, spi functions
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "finterrupt.h"
#include "fexception.h"
#include "fcache.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "gic_common.h"
#include "sgi_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef enum
{
    FGIC_SGI_SELF_TEST = 0,
    FGIC_SGI_OTHER_TEST,
    FGIC_SGI_PREEMPT_TEST,
} FGIC_SGI_TEST_MODE;
/************************** Variable Definitions *****************************/
static u8 *share_recv_buffer = 0;
static u32 *share_flg_pointor = 0;
static volatile u32 sgi_preempt_state ; /* 0 is initialization state ,1 is FGIC_SGI_1_VECT  finish test ,2 is FGIC_SGI_2_VECT  finish test */
static volatile u32 sgi_self_test_flg ; /* 0 is initialization state ,1 is the state of receiving the correct feedback from FGIC SGI 0 PRIORITY  */
static FGIC_SGI_TEST_MODE test_mode;
/***************** Macros (Inline Functions) Definitions *********************/
#define FGIC_SGI_0_VECT 0
#define FGIC_SGI_1_VECT 1
#define FGIC_SGI_2_VECT 2


#define FGIC_SGI_0_PRIORITY IRQ_PRIORITY_VALUE_12
#define FGIC_SGI_1_PRIORITY IRQ_PRIORITY_VALUE_8
#define FGIC_SGI_2_PRIORITY IRQ_PRIORITY_VALUE_0
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
void FSgiTestIrqHandler(s32 vector, void *param)
{
    
    FGIC_INFO("FSgiTestIrqHandler:");
    if (test_mode == FGIC_SGI_PREEMPT_TEST)
    {
        if (vector == FGIC_SGI_2_VECT)
        {
            FGIC_INFO("GET sgi 2 irq.");
            sgi_preempt_state = 2;
            FGIC_INFO("Exit sgi2.");
        }
        else if (vector == FGIC_SGI_1_VECT)
        {
            static fsize_t value_sgi1[3] = {0};
            FGIC_INFO("GET sgi 1 irq.");

            /* Enable the nested interrupts to allow preemption */     
            FInterruptNestedEnable(value_sgi1);

            InterruptCoreInterSend(FGIC_SGI_2_VECT, 1 << MASTER_CORE_ID);
            while (sgi_preempt_state != 2)
                ;
            sgi_preempt_state = 1;
            /* Disable the nested interrupt before exiting IRQ mode */
            FInterruptNestedDisable(value_sgi1);
            
            FGIC_INFO("Exit sgi1 is out.");
        }
        else if (vector == FGIC_SGI_0_VECT)
        {
            static fsize_t value_sgi0[3] = {0};
            FGIC_INFO("GET sgi 0 irq.");

            /* Enable the nested interrupts to allow preemption */     
            FInterruptNestedEnable(value_sgi0);

            InterruptCoreInterSend(FGIC_SGI_1_VECT, 1 << MASTER_CORE_ID);
            while (sgi_preempt_state != 1)
                ;

            /* Disable the nested interrupt before exiting IRQ mode */
            FInterruptNestedDisable(value_sgi0);

            FGIC_INFO("Exit sgi0.");
        }
    }
    else if (test_mode == FGIC_SGI_SELF_TEST)
    {
        sgi_self_test_flg = 1;
    }
    else
    {
        FGIC_ERROR("Error test_mode %x", test_mode);
    }
}

/* early data init */
static void FGicSgiEarlyInit(void)
{
    share_flg_pointor = (u32 *)SHARE_BUFFER_BASE;
    share_recv_buffer = (u8 *)SHARE_BUFFER_BASE + SHARE_BUFFER_DATA_OFFSET;
}

/* register 3 SGI intrs and set their priority */
static void FGicSgiInit(void)
{
    InterruptSetPriorityGroupBits(IRQ_GROUP_PRIORITY_5); /* use ggg.sssss group field */
    InterruptSetPriorityMask(IRQ_PRIORITY_MASK_15); /* Allow all interrupts to be answered  */

    InterruptMask(FGIC_SGI_0_VECT);
    InterruptMask(FGIC_SGI_1_VECT);
    InterruptMask(FGIC_SGI_2_VECT);

    InterruptSetPriority(FGIC_SGI_0_VECT, FGIC_SGI_0_PRIORITY);
    InterruptSetPriority(FGIC_SGI_1_VECT, FGIC_SGI_1_PRIORITY);
    InterruptSetPriority(FGIC_SGI_2_VECT, FGIC_SGI_2_PRIORITY);

    InterruptInstall(FGIC_SGI_0_VECT, FSgiTestIrqHandler, NULL, "sgi_0");
    InterruptInstall(FGIC_SGI_1_VECT, FSgiTestIrqHandler, NULL, "sgi_1");
    InterruptInstall(FGIC_SGI_2_VECT, FSgiTestIrqHandler, NULL, "sgi_2");

    InterruptUmask(FGIC_SGI_0_VECT);
    InterruptUmask(FGIC_SGI_1_VECT);
    InterruptUmask(FGIC_SGI_2_VECT);
}

/* master core send to master self */
static int FGicSgiSelfStartUp(void)
{
    u32 cnt = 0;
    test_mode = FGIC_SGI_SELF_TEST;
    sgi_self_test_flg = 0;
    InterruptCoreInterSend(FGIC_SGI_0_VECT, 1 << MASTER_CORE_ID);

    while (sgi_self_test_flg == 0)
    {
        if ((cnt++) >= 0xffffff)
        {
            FGIC_ERROR("FGicSgiSelfStartUp timeout.");
            return -1;
        }
    }

    FGIC_INFO("Sgi self test is ok.");
    return 0;
}

/* master core send to master self and preempt self intr */
static int FGicSgiPreemptStartUp(void)
{
    u32 cnt = 0;
    test_mode = FGIC_SGI_PREEMPT_TEST;
    sgi_preempt_state = 0;
    InterruptCoreInterSend(FGIC_SGI_0_VECT, 1 << MASTER_CORE_ID);

    while (sgi_preempt_state != 1)
    {
        if ((cnt++) >= 0xfffffff)
        {
            FGIC_ERROR("FGicSgiPreemptStartUp timeout");
            return -1;
        }
    }

    FGIC_INFO("Sgi preempt test is ok");
    return 0;
}

/* master core send to slave core */
static int FGicSgiOtherStartUp(void)
{
    u32 cnt = 0;
    test_mode = FGIC_SGI_OTHER_TEST;
    *share_flg_pointor = 0;
    FCacheDCacheFlushLine((intptr)share_flg_pointor);
    InterruptCoreInterSend(FGIC_SGI_0_VECT, 1 << SLAVE_CORE_ID);

    while (*share_flg_pointor != SHARE_BUFFER_FLG_FROM_SLAVE) /* rececive */
    {
        if ((cnt++) >= 0xffffff)
        {
            FGIC_ERROR("FGicSgiOtherStartUp timeout %x", *share_flg_pointor);
            return -1;
        }

        FCacheDCacheInvalidateLine((intptr)share_flg_pointor);
    }
    FGIC_INFO("Sgi other test is ok ");
    return 0;
}

/* function of SGI intr example */
int FSgiExample(void)
{
    int ret = FGIC_EXAMPLE_OK;
    int err = FGIC_EXAMPLE_OK;

    FGicSgiEarlyInit();
    FGicSgiInit();
    ret = FGicSgiSelfStartUp();
    if (ret != FGIC_EXAMPLE_OK)
    {
        err = FGIC_EXAMPLE_TIMEOUT;
        FGIC_ERROR("Sgi intr self test timeout.");
        goto exit;
    }
    ret = FGicSgiPreemptStartUp();
    if (ret != FGIC_EXAMPLE_OK)
    {
        err = FGIC_EXAMPLE_TIMEOUT;
        FGIC_ERROR("Sgi intr preempt test timeout.");
        goto exit;
    }
    ret = FGicSgiOtherStartUp();
    if (ret != FGIC_EXAMPLE_OK)
    {
        err = FGIC_EXAMPLE_TIMEOUT;
        FGIC_ERROR("Sgi intr other test timeout.");
        goto exit;
    }
exit:
    /* print message on example run result */
    if (err == FGIC_EXAMPLE_OK )
    {
        printf("%s@%d: sgi example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: sgi example failed !!!, err = %d \r\n", __func__, __LINE__, err);
    }

    return 0;
}