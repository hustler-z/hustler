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
 * FilePath: sys_init.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:56:14
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */


#ifndef SYS_INIT_H
#define SYS_INIT_H

#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

int FLibmetalSysInit() ;
void FLibmetalSysCleanup() ;
int FRegisterMetalDevice(void);

#ifdef __cplusplus
}
#endif

#endif // !
