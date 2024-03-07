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
 *  1.2 liusm      2023/11/20  Update example
 */


#include "ftypes.h"
#include "fpsci.h"
#include "shell.h"
#include "fsleep.h"
#include "fprintk.h"
#include "fdebug.h"
#include "shell_port.h"

extern int rpmsg_echo_task(void);

int main(void)
{
    BaseType_t ret;
    f_printk("freertos %s ,%s \r\n",__DATE__, __TIME__) ;
    rpmsg_echo_task();
    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);  
    return 0;
}

