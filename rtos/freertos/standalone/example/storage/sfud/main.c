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
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-10-08 18:47:54
 * Description:  This file is for memory pool basic test example main function.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add memory pool basic test example
 *  1.2    liqiaozhong  2023/10/8    divide example into spi and qspi parts
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell_port.h"
#endif

#include "sfud_spi_probe_example.h"
#include "sfud_qspi_probe_example.h"
#include "sfud_spi_wr_example.h"
#include "sfud_qspi_wr_example.h"
#include "sfud_spi_bench_example.h"
/***************** Macros (Inline Functions) Definitions *********************/
/******************************* Functions ***********************************/
int main()
{
#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
    LSUserShellLoop();    
#else
    /* if shell command is not enabled, run example one by one */
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_FIREFLY_DEMO_BOARD)
    FSfudSpiProbeExample();
    FSfudSpiWRExample();
    FSfudSpiBenchExample();
#endif
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
    FSfudQspiProbeExample();
    FSfudQspiWRExample();
#endif
#endif

    return 0;
}