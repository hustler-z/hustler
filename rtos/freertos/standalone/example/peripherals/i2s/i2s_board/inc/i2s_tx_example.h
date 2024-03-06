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
 * FilePath: i2s_tx_example.h
 * Date: 2023-05-23 09:15:16
 * LastEditTime: 2023-05-23 17:15:16
 * Description: This file is for the I2S board TX example function definition. 
 *
 * Modify History:
 *  Ver       Who             Date         Changes
 * -----   ----------       --------     ---------------------------------
 *  1.0   wangzongqiang    2023/07/23    init
 *  1.1   liqiaozhong      2023/12/19    solve bdl miss intr issue
 */

#ifndef I2S_TX_EXAMPLE_H
#define I2S_TX_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* includes */
#include "ftypes.h"
#include "ferror_code.h"

FError FI2sDdmaTxExample(void);
FError FI2sDdmaTxStopWork(void);

/* This structure holds the Real-Time Clock configuration values */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif