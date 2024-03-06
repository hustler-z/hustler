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
 * FilePath: mphalport.h
 * Date: 2023-12-07 14:53:41
 * LastEditTime: 2023-12-07 17:36:39
 * Description:  This file is for defining the function
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */

#ifndef  MP_HALPORT_H
#define  MP_HALPORT_H

#include "ftypes.h"
#ifdef __cplusplus
extern "C"
{
#endif


static inline mp_uint_t mp_hal_ticks_ms(void)
{
    return 0;
}
static inline void mp_hal_set_interrupt_char(char c)
{
}


#ifdef __cplusplus
}
#endif

#endif