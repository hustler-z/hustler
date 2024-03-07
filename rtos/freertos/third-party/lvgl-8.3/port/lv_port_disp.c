/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: lv_port_disp.c
 * Date: 2022-09-05 17:38:05
 * LastEditTime: 2022-07-07  12:11:05
 * Description:  This file is for providing the interface of lvgl test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/03/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 * 
 */

#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "fassert.h"
#include "finterrupt.h"
#include "fparameters.h"
#include "timers.h"
#include "task.h"
#include "fcpu_info.h"
#include "event_groups.h"

#include "fdcdp.h"
#include "lv_port_disp.h"
#include "lv_conf.h"
#include "../lvgl.h"

static lv_color_int_t *rtt_fbp[FDCDP_INSTANCE_NUM] ;
static u32 multi_mode;


static void FFreeRTOSMediaDispFlush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
/************************ Function Prototypes ******************************/

/**
 * @name: FMediaDispFramebuffer
 * @msg:  set the lvgl framebuffer addr and ensure the connected dp have the correct addr
 * @return null
 */
void FMediaDispFramebuffer(FDcDp *instance)
{
    FASSERT(instance != NULL);
  
    if ((rtt_fbp[0] == NULL))
    {
        rtt_fbp[0] = (lv_color_int_t *)instance->user_config[0].fb_virtual;
        multi_mode = instance->user_config[0].multi_mode;
    }
    else
    {
        rtt_fbp[1] = (lv_color_int_t *)instance->user_config[1].fb_virtual;
    }
    return;

}

/**
 * @name: FFreeRTOSPortInit
 * @msg:  init the lv config and set the instance
 * @return null
 */
void FFreeRTOSPortInit(void)
{
    static lv_disp_draw_buf_t draw_buf_dsc_1;
    static lv_color_t buf_1[LV_HOR_RES_MAX * 10];                             /*A buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, LV_HOR_RES_MAX * 10); /*Initialize the display buffer*/

    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = FFreeRTOSMediaDispFlush;
    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_1;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);

    return;
}

volatile bool disp_flush_enabled = true;

/**
 * @name: FFreeRTOSDispdEnableUpdate
 * @msg:  Enable updating the screen (the flushing process) when FMediaDispFlush() is called by LVGL
 * @return null
 */
void FFreeRTOSDispdEnableUpdate(void)
{
    disp_flush_enabled = true;
}

/**
 * @name: FFreeRTOSDispdDisableUpdate
 * @msg:  Disable updating the screen (the flushing process) when FMediaDispFlush() is called by LVGL
 * @return null
 */
void FFreeRTOSDispdDisableUpdate(void)
{
    disp_flush_enabled = false;
}

/**
 * @name: FMediaDispFlush
 * @msg:  flush the framebuffer
 * @param {lv_disp_drv_t *} disp_drv is the Display Driver structure to be registered by HAL
 * @param {const lv_area_t *} area is the specific area on the display you want to flush
 * @param {lv_color_t *} color_p is the image pixel of
 * @return null
 */
static void FFreeRTOSMediaDispFlush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
     long int location = 0;
    int32_t x;
    int32_t y;
    if (disp_flush_enabled)
    {
        if (multi_mode == 0)
        {

            for (y = area->y1; y <= area->y2; y++)
            {
                for (x = area->x1; x <= area->x2; x++)
                {
                    location = (x) + (y) * LV_HOR_RES_MAX;
                    rtt_fbp[0][location] = color_p->full;
                    color_p++;
                }
            }
        }
        else if (multi_mode == 1)
        {
            for (y = area->y1; y <= area->y2; y++)
            {
                for (x = area->x1; x <= (area->x2); x++)
                {
                    if (x < area->x2 / 2)
                    {

                        location = (x) + (y) * (LV_HOR_RES_MAX);
                        rtt_fbp[0][location  * 2] = (color_p->full);
                        rtt_fbp[0][location  * 2 + 1] = (color_p->full);
                    }
                    else
                    {
                        location = (x) + (y) * (LV_HOR_RES_MAX);
                        rtt_fbp[1][(location - area->x2 / 2) * 2 ] = (color_p->full);
                        rtt_fbp[1] [(location - area->x2 / 2) * 2 + 1 ] = (color_p->full);
                    }
                    color_p++;
                }
            }
        }
        else
        {
            for (y = area->y1; y <= area->y2; y++)
            {
                for (x = area->x1; x <= (area->x2); x++)
                {
                    if (y < LV_VER_RES_MAX / 2)
                    {
                        location = (x) + (y) * (LV_HOR_RES_MAX) * 2;
                        rtt_fbp[0][location] = (color_p->full);
                        rtt_fbp[0][(location) + LV_HOR_RES_MAX] = (color_p->full);
                    }
                    else
                    {
                        location = (x) + (y - LV_VER_RES_MAX / 2) * (LV_HOR_RES_MAX) * 2  ;
                        rtt_fbp[1][location] = (color_p->full);
                        rtt_fbp[1][(location)  + LV_HOR_RES_MAX] = (color_p->full);
                    }
                    color_p++;
                }
            }
        }
    }
    lv_disp_flush_ready(disp_drv);
}
