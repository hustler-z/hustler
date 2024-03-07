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
 * FilePath: main.c
 * Date: 2022-02-24 16:56:46
 * LastEditTime: 2022-03-21 17:00:56
 * Description:  This file is for AMP example that running rpmsg_echo_task and open scheduler
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0 huanghe    2022/03/25  first commit
 *  1.1 huanghe    2023/03/09  Adapt OpenAMP routines based on e2000D/Q
 */


#include "ftypes.h"
#include "fpsci.h"
#include "shell.h"
#include "fsleep.h"
#include "fprintk.h"
#include "fdebug.h"
#include "shell_port.h"

#define OPENAMP_MAIN_DEBUG_TAG "OPENAMP_MAIN"
#define OPENAMP_MAIN_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)

extern int FOpenampCmdEntry(int argc, char *argv[]) ;

int main(void)
{
    BaseType_t ret;

    ret = LSUserShellTask() ;
    if(ret != pdPASS)
        goto FAIL_EXIT;

    vTaskStartScheduler(); /* 启动任务，开启调度 */   
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);
    return 0;
}

