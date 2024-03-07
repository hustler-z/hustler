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
 * FilePath: media_test_example.h
 * Date: 2023-08-10 14:53:41
 * LastEditTime: 2023-08-10 17:36:39
 * Description:  This file is for defining the function
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/08/10   Modify the format and establish the version
 */

#ifndef MEDIA_TEST_EXAMPLE_H
#define MEDIA_TEST_EXAMPLE_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif
/**************************** Type Definitions *******************************/
typedef struct
{
    u8 Blue;
    u8 Green;
    u8 Red;
    u8 reserve;
} GraphicsTest;

/************************** Function Prototypes ******************************/
/*detect the hpd status*/
void FMediaHpdDetect(void);

/*init the media control*/
FError FMediaCtrlProbe(void);

/*init the irq*/
void FMediaInterruptInit(void);

/*disable the irq*/
void FMediaInterruptDeinit(void);

/*deinit the dc*/
FError FMediaChannelDeinit(u32 id);

/*the demo for testing the media*/
void FMediaDisplayDemo(void);

/*dump the register*/
void FMediaDebug(u32 id);

/*test the demo for light the screen*/
FError FMediaExample(void);

#ifdef __cplusplus
}
#endif

#endif