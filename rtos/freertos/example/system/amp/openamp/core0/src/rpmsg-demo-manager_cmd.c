/*
 * @Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * @FilePath: rpmsg-demo-manager_cmd.c
 * @Date: 2023-04-23 16:02:34
 * @LastEditTime: 2023-04-23 16:02:35
 * @Description:  This file is for manager rpmsg demos
 * 
 * @Modify History: 
 *  Ver   Who           Date        Changes
 * ----- ------         --------    --------------------------------------
 * 1.0  liushengming    2023/04/23  first release
 * 1.1  liushengming    2023/05/31  add loadelf
 */

#include <stdio.h>
#include <openamp/version.h>
#include <openamp/open_amp.h>
#include <metal/version.h>
#include <metal/alloc.h>
#include <string.h>
#include "strto.h"
#include "shell.h"
#include "platform_info.h"
#include "rpmsg_service.h"
#include <metal/sleep.h>
#include "rsc_table.h"
#include "fdebug.h"
#include "fkernel.h"
#include "felf.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"
#include "task.h"
#include "finterrupt.h"
#include "fpsci.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define     DEMO_MANG_MASTER_DEBUG_TAG "    DEMO_MANG_MASTER"
#define     DEMO_MANG_MASTER_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( DEMO_MANG_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_MANG_MASTER_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( DEMO_MANG_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_MANG_MASTER_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( DEMO_MANG_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)

/**************************** Type Definitions *******************************/

#define MANAGE_READ 1
#define MANAGE_WAIT 0

#define DEMO_MANAGER_ADDR   (RPMSG_RESERVED_ADDRESSES - 1)

#define RPMSG_PING_DEMO     0x1
#define RPMSG_SAM_PING_DEMO 0X2
#define RPMSG_MAT_MULT_DEMO 0X3
#define RPMSG_NO_COPY_DEMO  0X4
#define RPMSG_DEMO_MAX      0x5

#define REMOTEPROC_MASK     BIT(CONFIG_TARGET_CPU_MASK)
#define ELF_LOAD_ADDR       0xf1000000U
/************************** Variable Definitions *****************************/
/* Globals */

static struct rpmsg_endpoint lept;
static struct rpmsg_device *rpdev = NULL;

static int volatile cmd_ok = MANAGE_WAIT;
static u32 demo_flag = RPMSG_PING_DEMO;
static void *platform = NULL;
static u32 elf_boot_flag = 0;

extern struct image_store_ops mem_image_store_ops;
struct mem_file {
	const void *base;
};

static struct mem_file image = {
	.base = (void *)ELF_LOAD_ADDR,
};

/************************** Function Prototypes ******************************/
extern int rpmsg_echo(struct rpmsg_device *rdev, void *priv);
extern int rpmsg_sample_echo(struct rpmsg_device *rdev, void *priv) ;
extern int matrix_multiplyd(struct rpmsg_device *rdev, void *priv) ;
extern int rpmsg_nocopy_echo(struct rpmsg_device *rdev, void *priv) ;
static int FOpenampClose(void *platform);

static void FOpenampCmdUsage()
{
    printf("Usage:\r\n");
    printf("openamp auto \r\n");
    printf("-- Auto running.\r\n");
    printf("-- [1] This application echoes back data that was sent to it by the master core.\r\n");
    printf("-- [2] This application simulate sample rpmsg driver. For this it echo 100 time message sent by the rpmsg sample client available in distribution.\r\n");
    printf("-- [3] This application receives two matrices from the master, multiplies them and returns the result to the master core.\r\n");
    printf("-- [4] This application echoes back data that was sent to it by the master core.\r\n");
}

/*-----------------------------------------------------------------------------*
 *  Image Function
 *-----------------------------------------------------------------------------*/
static int FLoadelfRemoteproc(struct image_store_ops *store_ops,void *platform)
{
    int ret = 0;
    struct remoteproc *rproc = platform;
    struct remoteproc_priv *prproc = rproc->priv;

    prproc->cpu_mask = REMOTEPROC_MASK;
    if (rproc == NULL)
        return -1;
    /* Configure remoteproc to get ready to load executable */
	remoteproc_config(rproc, NULL);

    /* Load remoteproc executable */
	DEMO_MANG_MASTER_DEBUG_I("Start to load executable with remoteproc_load(0x%x) \r\n",(unsigned long)image.base);

    rproc->bootaddr = ElfLoadElfImagePhdr((unsigned long)image.base);
    if (!rproc->bootaddr)
    {
        rproc->state = RPROC_READY;
    }
    /* Start the processor */
    ret = remoteproc_start(rproc);
	if (ret) 
    {
        remoteproc_shutdown(rproc);
		DEMO_MANG_MASTER_DEBUG_E("failed to start processor\r\n");
		return ret;
	}
	DEMO_MANG_MASTER_DEBUG_I("successfully started the processor\r\n");

    return ret;
}

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                             uint32_t src, void *priv)
{
    u32 i = *(u32 *)data;

    (void)ept;
    (void)src;
    (void)priv;
    DEMO_MANG_MASTER_DEBUG_E("src:0x%x",src);
    /* 通过'src'可以筛选自己想要的数据来源 */

    if (i != demo_flag)
    {
        DEMO_MANG_MASTER_DEBUG_I("Data corruption at index %d.\r\n", i);
        DEMO_MANG_MASTER_DEBUG_I("Want data is %d.\r\n", demo_flag);
        return RPMSG_ERROR_BASE;
    }
    cmd_ok = MANAGE_READ;
    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    rpmsg_destroy_ept(&lept);
    DEMO_MANG_MASTER_DEBUG_E("Echo test: service is destroyed.\r\n");
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
                                       const char *name, uint32_t dest)
{
    DEMO_MANG_MASTER_DEBUG_I("New endpoint notification is received.\r\n");
    if (strcmp(name, DEMO_SERVICE_NAME))
    {
        DEMO_MANG_MASTER_DEBUG_E("Unexpected name service %s.\r\n", name);
    }
    else
        (void)rpmsg_create_ept(&lept, rdev, DEMO_SERVICE_NAME,
                               DEMO_MANAGER_ADDR, dest,
                               rpmsg_endpoint_cb,
                               rpmsg_service_unbind);
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int FManageEptCreat(struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;

    /* Create RPMsg endpoint */
    ret = rpmsg_create_ept(&lept, rdev, DEMO_SERVICE_NAME,
                            DEMO_MANAGER_ADDR, RPMSG_ADDR_ANY,
                            rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        DEMO_MANG_MASTER_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    while (!is_rpmsg_ept_ready(&lept))
    {
        DEMO_MANG_MASTER_DEBUG_I("start to wait platform_poll \r\n");
        platform_poll(priv);
    }
    DEMO_MANG_MASTER_DEBUG_I("Manage rpmsg_ept_ready!!!");
    return ret;
}

int FRunningApp(struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;

    ret = rpmsg_send(&lept, &demo_flag, sizeof(u32));
    if (ret < 0)
    {
        DEMO_MANG_MASTER_DEBUG_E("Failed to send data,ret:%d...\r\n",ret);
        return ret;
    }
    cmd_ok = MANAGE_WAIT;
    do
    {
        platform_poll(priv);
    }
    while ( cmd_ok == MANAGE_WAIT );

    if (demo_flag != 0 && cmd_ok == MANAGE_READ)
    {
        if (demo_flag == RPMSG_PING_DEMO)
        {
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("***********Demo rpmsg_echo running***********......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            ret = rpmsg_echo(rdev, priv);
            if (ret != 0)
            {
                DEMO_MANG_MASTER_DEBUG_E("rpmsg_echo running error,ecode:%d.",ret);
                return ret;
            }
        }

        if (demo_flag == RPMSG_SAM_PING_DEMO)
        {
            printf("\r\n\r\n");
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n");
            DEMO_MANG_MASTER_DEBUG_I("*******Demo rpmsg_sample_echo running********......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            ret = rpmsg_sample_echo(rdev, priv);
            if (ret != 0)
            {
                DEMO_MANG_MASTER_DEBUG_E("rpmsg_sample_echo running error,ecode:%d.",ret);
                return ret;
            }
        }

        if (demo_flag == RPMSG_MAT_MULT_DEMO)
        {
            printf("\r\n\r\n");
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*******Demo matrix_multiplyd running********......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            ret = matrix_multiplyd(rdev, priv);
            if (ret != 0)
            {
                DEMO_MANG_MASTER_DEBUG_E("matrix_multiplyd running error,ecode:%d.",ret);
                return ret;
            }
        }
        
        if (demo_flag == RPMSG_NO_COPY_DEMO)
        {
            printf("\r\n\r\n");
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*******Demo rpmsg_nocopy_echo running********......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            ret = rpmsg_nocopy_echo(rdev, priv);
            if (ret != 0)
            {
                DEMO_MANG_MASTER_DEBUG_E("rpmsg_nocopy_echo running error,ecode:%d.",ret);
                return ret;
            }
        }
        cmd_ok = MANAGE_WAIT;
        DEMO_MANG_MASTER_DEBUG_I(" Demo %d running over...",demo_flag);
        demo_flag++;
        return ret;
    }
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
static int FRpmsgDemoManager(void *platform)
{
    int ret = 0;
    rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_MASTER, NULL, rpmsg_name_service_bind_cb);
    if (!rpdev)
    {
        DEMO_MANG_MASTER_DEBUG_E("Failed to create rpmsg virtio device.\r\n");
        ret = platform_cleanup(platform);
        return ret;
    }
    else
    {
        ret = FManageEptCreat(rpdev, platform);
    }
    return ret;
}



static int FOpenampClose(void *platform)
{
    int ret = 0;
    struct remoteproc *rproc = platform;
    if (rproc == NULL)
        return -1;
    if (rpdev == NULL)
        return -1;
    
    rpmsg_destroy_ept(&lept);
    
    platform_release_rpmsg_vdev(rpdev, platform);

    ret = remoteproc_shutdown(rproc);
    if (ret != 0)
    {
        DEMO_MANG_MASTER_DEBUG_E("Can't shutdown remoteproc,error code:0x%x.",ret);
    }

    ret = platform_cleanup(platform);
    if (ret != 0)
    {
        DEMO_MANG_MASTER_DEBUG_E("Can't remove platform,error code:0x%x.",ret);
    }
    
    return ret;
}

int FOpenampExample(void)
{
    int ret = 0;
    
    demo_flag = RPMSG_PING_DEMO;

    if (elf_boot_flag == 0)
    {
        /* Initialize platform */
        ret = platform_init(1, 0, &platform);
        if (ret)
        {
            DEMO_MANG_MASTER_DEBUG_E("Failed to initialize platform.\r\n");
            platform_cleanup(platform);
            return -1;
        }

        ret = FLoadelfRemoteproc(&mem_image_store_ops,platform);
        if (ret)
        {
            DEMO_MANG_MASTER_DEBUG_E("Failed to FLoadelfRemoteproc.\r\n");
            platform_cleanup(platform);
            return -1;
        }
    
        ret = FRpmsgDemoManager(platform);
        if (ret)
        {
            return FOpenampClose(platform);
        }
        elf_boot_flag = 1;
    }
    if (elf_boot_flag == 1)
    {
        while (demo_flag < RPMSG_DEMO_MAX)
        {
            ret = FRunningApp(rpdev, platform);
            if (ret)
            {
                return FOpenampClose(platform);
            }
        }
    }
    return ret;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
void RpmsgEchoTask( void * args )
{
	int ret;

    DEMO_MANG_MASTER_DEBUG_I("complier %s ,%s \r\n", __DATE__, __TIME__);
	DEMO_MANG_MASTER_DEBUG_I("openamp lib version: %s (", openamp_version());
	DEMO_MANG_MASTER_DEBUG_I("Major: %d, ", openamp_version_major());
	DEMO_MANG_MASTER_DEBUG_I("Minor: %d, ", openamp_version_minor());
	DEMO_MANG_MASTER_DEBUG_I("Patch: %d)\r\n", openamp_version_patch());

	DEMO_MANG_MASTER_DEBUG_I("libmetal lib version: %s (", metal_ver());
	DEMO_MANG_MASTER_DEBUG_I("Major: %d, ", metal_ver_major());
	DEMO_MANG_MASTER_DEBUG_I("Minor: %d, ", metal_ver_minor());
	DEMO_MANG_MASTER_DEBUG_I("Patch: %d)\r\n", metal_ver_patch());

	/* Initialize platform */
	DEMO_MANG_MASTER_DEBUG_I("start application");
	ret = FOpenampExample();
	if (ret) 
	{
		DEMO_MANG_MASTER_DEBUG_E("Failed to running example.\r\n");
	}
	DEMO_MANG_MASTER_DEBUG_I("Stopping application...\r\n");
    vTaskDelete(NULL);
}


BaseType_t FOpenampCmdEntry(int argc, char *argv[])
{
    BaseType_t ret = 0;

    if (!strcmp(argv[1], "auto"))
    {
        taskENTER_CRITICAL(); /* no schedule when create task */

        ret = xTaskCreate((TaskFunction_t )RpmsgEchoTask, /* 任务入口函数 */
                            (const char* )"RpmsgEchoTask",/* 任务名字 */
                            (uint16_t )(4096*2), /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )4, /* 任务的优先级 */
                            NULL); /* 任务控制块指针 */
        taskEXIT_CRITICAL(); /* allow schedule since task created */
        
        if(ret != pdPASS)
        {
            DEMO_MANG_MASTER_DEBUG_E("Failed to create a RpmsgEchoTask task ");
            return -1;
        }
    }
    else
    {
        FOpenampCmdUsage();
        return -1;
    }

    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), openamp_echo, FOpenampCmdEntry, test freertos openamp);
