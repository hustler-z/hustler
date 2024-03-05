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
 * FilePath: lvgl_disp_test.h
 * Created Date: 2023-06-07 09:19:18
 * Last Modified: Mon Oct 23 2023
 * Description:  This file is for providing the lvgl test config
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0       Wangzq     2023/06/09         Modify the format and establish the version
 */
#ifndef LVGL_DSIP_TEST_H
#define LVGL_DSIP_TEST_H

#include "ftypes.h"
#include "fparameters.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    u32 channel;/* data */
    u32 width;
    u32 height;
    u32 multi_mode;
    u32 color_depth;
    u32 refresh_rate;
} InputParm;

/*hpd event detect*/
void lv_hpd_detect(void);

/*init the media*/
FError FMediaDispInit(void);
/*init the irq*/
void FMediaInterruptInit(void);

/*disable the irq*/
void FMediaInterruptDeinit(void);

/*deinit the dc*/
FError FMediaChannelDeinit(u32 id);

/*dump the register*/
void FMediaDebug(u32 id);

/*init the media control*/
FError FMediaCtrlProbe(void);

void FMediaLvInit(void);

FError FMediaLvConfig(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif 