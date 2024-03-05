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
 * FilePath: main.c
 * Date: 2023-03-27 14:54:41
 * Last Modified: 2024-02-19 10:27:14
 * Description:  This file is for gdma main function.
 *
 * Modify History:
 *  Ver      Who         Date         Changes
 * -----   ------      --------     --------------------------------------
 *  1.0  liqiaozhong   2023/3/28    first commit
 */

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell_port.h"
#endif

#include "gdma_direct_transfer_example.h"
#include "gdma_bdl_transfer_example.h"
#include "gdma_performance_example.h"
#include "gdma_abort_example.h"
#include "gdma_multi_channel_example.h"
/******************************* Main Function *******************************/
int main()
{
#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
    LSUserShellLoop();    
#else
    /* if shell command is not enabled, run example one by one */
    FGdmaDirectTransferExample();
    FGdmaBDLTransferExample(); 
    FGdmaPerformanceExample();
    FGdmaAbortExample();
    FGdmaMultiChannelExample();
#endif
    return 0;
}