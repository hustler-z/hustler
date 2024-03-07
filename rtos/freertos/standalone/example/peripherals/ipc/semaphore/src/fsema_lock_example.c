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
 * FilePath: fsema_lock_example.c
 * Date: 2023-05-25 16:03:21
 * LastEditTime: 2023-05-31 15:56:57
 * Description:  This file is for semaphore lock example function implementation. 
 * 
 * Modify History:
 *  Ver    Who         Date            Changes
 * -----  ------      --------    --------------------------------------
 *  1.0  liuzhihong   2023/5/26      first release
 */


/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "ftypes.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "fsema_lock_example.h"
/************************** Constant Definitions *****************************/
enum
{
    FSEMA_OPS_OK = 0,
    FSEMA_OPS_INVAILD_PARAMS, /* 无效硬件信号量控制器id */
    FSEMA_OPS_INIT_FAILED,
    FSEMA_OPS_MALLOC_FAILED,
    FSEMA_OPS_CREATE_LOCKER_FAILED,
    FSEMA_OPS_DUPLICATE_LOCKER, /* 已经存在同名锁实例 */
    FSEMA_OPS_LOCKER_NO_FOUND, /* 未找到锁实例 */
    FSEMA_OPS_TAKE_LOCKER_FAILED,/* 获取锁实例失败 */
    FSEMA_OPS_GIVE_LOCKER_FAILED,/* 释放锁实例失败 */
    FSEMA_OPS_DELETE_LOCKER_FAILED,
    FSEMA_OPS_RESET_ALL_LOCKER_FAILED,/* 复位所有锁实例失败 */
};

/**************************** Type Definitions *******************************/


/************************** Variable Definitions *****************************/
/* variables used in example */
static FSemaConfig sema_cfg;
static FSema sema;
static int init_flag;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/


/**
 * @name: FSemaOpsInit
 * @msg: 初始化Semaphore控制器
 * @return {int} 返回错误码信息
 * @param {u32} instance_id
 */
static int FSemaOpsInit(u32 instance_id)
{
    FError err = FSEMA_SUCCESS;


    if (instance_id >= FSEMA_INSTANCE_NUM)
    {
        return FSEMA_OPS_INVAILD_PARAMS;
    }

    sema_cfg = *FSemaLoopkupConfig(instance_id);
    err = FSemaCfgInitialize(&sema, &sema_cfg);
    if (FSEMA_SUCCESS != err)
    {
        printf("Init failed !!! 0x%x \r\n", err);
        return FSEMA_OPS_INIT_FAILED;
    }

    init_flag = 1;
    return FSEMA_OPS_OK;
}

/**
 * @name: FSemaOpsFindLocker
 * @msg: 寻找名字为locker_name的锁
 * @return {static FSemaLocker *} 找到返回锁实例，没有找到返回NULL
 * @param {char} *locker_name
 */
static FSemaLocker *FSemaOpsFindLocker(const char *locker_name)
{
    u32 loop;
    FSemaLocker *locker = NULL;

    for (loop = 0; loop < FSEMA_NUM_OF_LOCKER; loop++)
    {
        if ((NULL != sema.locker[loop]) && (NULL != sema.locker[loop]->name))
        {
            if (!strcmp(sema.locker[loop]->name, locker_name))
            {
                locker = sema.locker[loop];
                break;
            }
        }
    }

    return locker;
}


/**
 * @name: FSemaOpsCreateLocker
 * @msg: 创建名为locker_name的锁
 * @return {int} 返回错误码信息
 * @param {char} *locker_name
 */
static int FSemaOpsCreateLocker(const char *locker_name)
{

    FError err = FSEMA_SUCCESS;
    FSemaLocker *locker = malloc(sizeof(FSemaLocker));
    if (NULL == locker)
    {
        return FSEMA_OPS_MALLOC_FAILED;
    }

    if (NULL != FSemaOpsFindLocker(locker_name))
    {
        free(locker);
        return FSEMA_OPS_DUPLICATE_LOCKER;
    }

    memset(locker, 0, sizeof(*locker));
    err = FSemaCreateLocker(&sema, locker);
    if (FSEMA_SUCCESS != err)
    {
        printf("Create locker failed !!! 0x%x \r\n", err);
        free(locker);
        return FSEMA_OPS_CREATE_LOCKER_FAILED;
    }

    /* assign name of locker */
    memset(locker->name, 0, FSEMA_LOCKER_NAME_LEN);
    fsize_t name_len = min(strlen(locker_name), (fsize_t)(FSEMA_LOCKER_NAME_LEN - 1));
    memcpy(locker->name, locker_name, name_len);
    locker->name[name_len] = '\0';
    printf("Create locker-%s success !!! \r\n", locker->name);

    return FSEMA_OPS_OK;
}

static void FSemaOpsRelax(FSema *const instance)
{
    fsleep_microsec(10);
}

/**
 * @name: FSemaOpsTakeLocker
 * @msg: owner获取名为locker_name的锁
 * @return {int} 返回错误码信息
 * @param {char} *locker_name
 * @param {u32} owner
 */
static int FSemaOpsTakeLocker(const char *locker_name, u32 owner)
{

    FError err = FSEMA_SUCCESS;
    FSemaLocker *locker = FSemaOpsFindLocker(locker_name);

    if (NULL == locker)
    {
        return FSEMA_OPS_LOCKER_NO_FOUND;
    }

    err = FSemaTryLock(locker, owner, 10, FSemaOpsRelax);
    if (FSEMA_SUCCESS != err)
    {
        printf("Take locker failed !!! 0x%x\r\n", err);
        return FSEMA_OPS_TAKE_LOCKER_FAILED;
    }
    else
    {
        printf("Take locker success !!!\r\n");
    }

    return FSEMA_OPS_OK;
}

/**
 * @name: FSemaOpsGiveLocker
 * @msg: owner释放名为locker_name的锁
 * @return {int} 返回错误码信息
 * @param {char} *locker_name
 * @param {u32} owner
 */
static int FSemaOpsGiveLocker(const char *locker_name, u32 owner)
{

    FError err = FSEMA_SUCCESS;
    FSemaLocker *locker = FSemaOpsFindLocker(locker_name);

    if (NULL == locker)
    {
        return FSEMA_OPS_LOCKER_NO_FOUND;
    }

    err = FSemaUnlock(locker, owner);
    if (FSEMA_SUCCESS != err)
    {
        printf("Give locker failed !!! 0x%x \r\n", err);
        return FSEMA_OPS_GIVE_LOCKER_FAILED;
    }

    return FSEMA_OPS_OK;
}



/**
 * @name: FSemaOpsUnlockAll
 * @msg: 解除所有的锁，释放锁实例
 * @return {int} 返回错误码信息
 */
static int FSemaOpsUnlockAll(void)
{
    u32 loop;
    FError err = FSEMA_SUCCESS;

    err = FSemaUnlockAll(&sema);
    if (FSEMA_SUCCESS != err)
    {
        printf("Reset all locker failed !!! 0x%x \r\n", err);
        return FSEMA_OPS_RESET_ALL_LOCKER_FAILED;
    }

    for (loop = 0; loop < FSEMA_NUM_OF_LOCKER; loop++)
    {
        if (NULL != sema.locker[loop])
        {
            free(sema.locker[loop]);
            sema.locker[loop] = NULL;
        }
    }

    return FSEMA_OPS_OK;
}

/**
 * @name: FSemaOpsDeinit
 * @msg: 去初始化Semaphore控制器
 * @return {int} 返回错误码信息
 */
static int FSemaOpsDeinit(void)
{
    int ret = FSEMA_OPS_OK;

    ret = FSemaOpsUnlockAll(); /* only error when not inited sema controller */
    if (FSEMA_OPS_OK == ret)
    {
        FSemaDeInitialize(&sema);
        init_flag = 0;
    }

    return ret;
}



/* function of fsema lock example, 
   Note: make sure all resource allocated in FSemaLockExample has been de-allocated before exit */
int FSemaLockExample()
{
    u32 instance_id = FSEMA0_ID;

    int ret = FSEMA_OPS_OK;
    const char *locker_apple = "apple";
    const char *locker_pine = "pine";
    u32 owener_x = 0x1234;
    u32 owener_y = 0xabcd;

    /* 初始化控制器 */
    ret = FSemaOpsInit(instance_id);
    if (FSEMA_OPS_OK != ret)
    {
        goto exit;
    }
    /* 创建锁实例 */
    ret = FSemaOpsCreateLocker(locker_apple);
    if (FSEMA_OPS_OK != ret)
    {
        goto exit;
    }

    ret = FSemaOpsCreateLocker(locker_pine);
    if (FSEMA_OPS_OK != ret)
    {
        goto exit;
    }

     /* owener_x 获取锁 apple */
    printf("Owner 0x%x take locker %s, should success.\r\n", owener_x, locker_apple);
    ret = FSemaOpsTakeLocker(locker_apple, owener_x);
    if (FSEMA_OPS_OK != ret)
    {
        goto exit;
    }

    /* owener_y 获取锁 pine */
    printf("Owner 0x%x take locker %s, should success.\r\n", owener_y, locker_pine);
    ret = FSemaOpsTakeLocker(locker_pine, owener_y);
    if (FSEMA_OPS_OK != ret)
    {
        goto exit;
    }

    /* owener_y 尝试获取锁 apple, must fail */
    printf("Owner 0x%x take locker %s, should fail.\r\n", owener_y, locker_apple);
    ret = FSemaOpsTakeLocker(locker_apple, owener_y);

    /* owener_x 释放锁 apple */
    printf("Owner 0x%x give locker %s\r\n", owener_x, locker_apple);
    ret = FSemaOpsGiveLocker(locker_apple, owener_x);
    if (FSEMA_OPS_OK != ret)
    {
        goto exit;
    }

    /* owener_y 尝试获取锁 apple, should success */
    printf("Owner 0x%x take locker %s, should success.\r\n", owener_y, locker_apple);
    ret = FSemaOpsTakeLocker(locker_apple, owener_y);
    if (FSEMA_OPS_OK != ret)
    {
        goto exit;
    }

    /* 去初始化控制器 */
    ret = FSemaOpsDeinit();

    /* print message on example run result */
exit:
    if(init_flag)
    {
        FSemaOpsDeinit();
    }     

    if (0 == ret)
    {
        printf("%s@%d: fsema lock example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: fsema lock example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }

    return 0;
}






