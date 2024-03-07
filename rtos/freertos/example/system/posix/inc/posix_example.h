/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: posix_example.h
 * Date: 2023-10-12 10:42:40
 * LastEditTime: 2023-10-12 10:42:40
 * Description:  This file is for freertos posix function define
 *
 * Modify History:
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0 wangxiaodong 2023/10/12  first commit
 */


#ifndef POSIX_EXAMPLE_H
#define POSIX_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* POSIX Demo task */
void CreatePOSIXDemoTasks(void);
void CreateThreadDemoTasks(void);

#ifdef __cplusplus
}
#endif

#endif // !