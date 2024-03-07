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
 * FilePath: slave_core_sgi.c
 * Date: 2023-04-26 08:37:22
 * LastEditTime: 2022-04-27 11:00:53
 * Description:  This file is for slave core sgi functions.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------     --------------------------------------
 * 1.0   liqiaozhong   2023/4/27    first commit and add spi, sgi, spi functions
 */

#include <stdio.h>

#include "ftypes.h"
#include "finterrupt.h"
#include "fcache.h"

#include "gic_common.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static u32 *share_flg_pointor;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void FSgiTestIrqHandler(s32 vector, void *param)
{
    printf("Slave is received %d \r\n", vector);
    *share_flg_pointor = SHARE_BUFFER_FLG_FROM_SLAVE;
    FCacheDCacheFlushLine((intptr)share_flg_pointor);
}

void SlaveCoreSgiEarlyInit(void)
{
    share_flg_pointor = (u32 *)SHARE_BUFFER_BASE;
    InterruptSetPriority(FGIC_OTHER_SGI_VECT, 0);
    InterruptInstall(FGIC_OTHER_SGI_VECT, FSgiTestIrqHandler, NULL, "sgi_0");
    InterruptUmask(FGIC_OTHER_SGI_VECT);
}


