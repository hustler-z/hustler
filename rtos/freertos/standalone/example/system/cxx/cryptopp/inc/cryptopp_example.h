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
 * FilePath: cryptopp_example.h
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for crypto++ example definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

#ifndef CRYPTOPP_EXAMPLE_H
#define CRYPTOPP_EXAMPLE_H

/* use C linkage to support use C++ method in C */
#ifdef __cplusplus
extern "C" {
#endif

int FCryptoPPCrc32Example(void);
int FCryptoPPMd5Example(void);
int FCryptoPPMd4Example(void);
int FCryptoPPAdler32Example(void);

#ifdef __cplusplus
}
#endif

#endif