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
 * FilePath: i2c_master_slave_example.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:41:25
 * Description:  This file is for i2c master_slave communication defintion
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/02/17    first commit
 */

#ifndef  I2C_MASTER_SLAVE_EXAMPLE_H
#define  I2C_MASTER_SLAVE_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ftypes.h"
#include "fi2c.h"
#include "sdkconfig.h"
#if defined(CONFIG_TARGET_E2000)
#include "fmio_hw.h"
#include "fmio.h"
#include "fiopad.h"
#endif

#define VIRTUAL_EEPROM_MEM_BYTE_LEN 1 /*1->8bit ~ 4->32bit*/

#define IO_BUF_LEN 256

FError FI2cMasterSlaveExample(void);

#ifdef __cplusplus
}
#endif

#endif