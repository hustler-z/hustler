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
 * FilePath: atomic_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for test atomic functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong   2023/2/17   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "ferror_code.h"
#include "fparameters.h"
#include "fsleep.h"
#include "ftypes.h"
#include "fkernel.h"
#include "fatomic.h"
#include "fassert.h"
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function *****************************************/

void FAtomicExample(void)
{
    int ret;
    int i = 0;
    u32 count = 0;

    i = 0;
    while (i++ < 10)
    {
        ret = FATOMIC_ADD(count, 1);
    }
    FASSERT_MSG(count == 10, "FATOMIC_ADD [failure].\r\n");

    i = 0;
    while (i++ < 10) 
    {
        ret = FATOMIC_INC(count);
    }
    FASSERT_MSG(count == 20, "FATOMIC_INC [failure].\r\n");

    i = 0;
    while (i++ < 10) 
    {
        ret = FATOMIC_SUB(count, 1);
    }
    FASSERT_MSG(count == 10, "FATOMIC_SUB [failure].\r\n");

    i = 0;
    while (i++ < 10) 
    {
        ret = FATOMIC_DEC(count);
    }
    FASSERT_MSG(count == 0, "FATOMIC_DEC [failure].\r\n");

    i = 0;
    count = 0;
    while (i++ < 16)
    {
        ret = FATOMIC_OR(count, BIT(i-1));
    }
    FASSERT_MSG(count == 0xFFFF, "FATOMIC_OR [failure].\r\n");

    i = 0;
    count = 0xFFFF;
    while (i++ < 16)
    {
        ret = FATOMIC_AND(count, ~BIT(i-1));
    }
    FASSERT_MSG(count == 0, "FATOMIC_AND [failure].\r\n");

    FATOMIC_CAS_BOOL(count, 0, 1);
    FASSERT_MSG(count == 1, "FATOMIC_CAS_BOOL [failure].\r\n");

    FATOMIC_CAS_VAL(count, 0xFF, 0);
    FASSERT_MSG(count == 1, "FATOMIC_CAS_VAL [failure].\r\n");

    FATOMIC_LOCK(count, 1);
    FASSERT_MSG(count == 1, "FATOMIC_LOCK [failure].\r\n");

    FATOMIC_UNLOCK(count);
    FASSERT_MSG(count == 0, "FATOMIC_UNLOCK [failure].\r\n");

    printf("Atomic example test [success].\r\n");

}