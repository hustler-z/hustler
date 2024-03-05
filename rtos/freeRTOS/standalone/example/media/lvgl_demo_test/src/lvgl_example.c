/*
 * @Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * @FilePath: lvgl_example.c
 * @Date: 2023-07-13 19:40:14
 * @LastEditTime: 2023-07-13 19:40:15
 * @Description:  This file is for
 *
 * @Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  liusm      2023/07/12    example test init
 */
#include <stdbool.h>
#include <stdio.h>
#include "fdc_common_hw.h"
#include "fdcdp.h"
#include "fdp_hw.h"
#include "ferror_code.h"
#include "fgeneric_timer.h"
#include "finterrupt.h"
#include "ftypes.h"
#include "fassert.h"
#include "fparameters.h"
#include "fsleep.h"
#include "shell_port.h"

#include "lv_port_disp.h"
#include "lvgl_disp_test.h"
#include "lvgl_example.h"
#include "lvgl.h"
#include "lv_conf.h"

#if LV_USE_DEMO_BENCHMARK
    #include "lv_demo_benchmark.h"
#endif

#if LV_USE_DEMO_STRESS
    #include "lv_demo_stress.h"
#endif

#if LV_USE_DEMO_WIDGETS
    #include "lv_demo_widgets.h"
#endif

#define LV_USE_DEMO_INACTIVE_TIME    100000
/************************** Function Prototypes ******************************/

#if LV_USE_DEMO_BENCHMARK
static void on_benchmark_finished(void)
{
    disp_enable_update();
    return ;
}
/**
 * @name: benchmark
 * @msg:  the benchmark demo of lvgl
 * @return Null
 */
void benchmark(void)
{
    disp_disable_update();
    lv_demo_benchmark_set_finished_cb(&on_benchmark_finished);
    lv_demo_benchmark_set_max_speed(true);
    lv_demo_benchmark();
    disp_enable_update();
    printf("lvgl benchmark is running \r\n");
    while (1)
    {
        if (lv_disp_get_inactive_time(NULL) < LV_USE_DEMO_INACTIVE_TIME)
        {
            lv_timer_handler(); //! run lv task at the max speed
            lv_hpd_detect();
        }
        else
        {
            printf("lvgl benchmark  is over \r\n");
            break;
        }
    }
}
#endif

#if LV_USE_DEMO_STRESS
void stress(void)
{
    disp_disable_update();
    lv_demo_stress();
    /* loop once to allow objects to be created */
    disp_enable_update();
    printf("lvgl stress is running \r\n");
    while (1)
    {
        if (lv_disp_get_inactive_time(NULL) < LV_USE_DEMO_INACTIVE_TIME)
        {
            lv_timer_handler(); //! run lv task at the max speed
            lv_hpd_detect();
        }
        else
        {
            printf("lvgl stress is over \r\n");
            break;
        }
    }
}
#endif

#if LV_USE_DEMO_WIDGETS
/**
 * @name: widgets
 * @msg:  the widgets demo of lvgl
 * @return Null
 */
void widgets(void)
{
    lv_demo_widgets();
    printf("lvgl widgets is running \r\n");
    while (1)
    {
        if (lv_disp_get_inactive_time(NULL) < LV_USE_DEMO_INACTIVE_TIME)
        {
            lv_timer_handler(); //! run lv task at the max speed
            lv_hpd_detect();
        }
        else
        {
            printf("lvgl widgets is over \r\n");
            break;
        }

    }

}
#endif
/* function of media example */
FError FMediaLvglExample(void)
{
    FError ret = FMEDIA_DCDP_SUCCESS;

    ret = FMediaDispInit();
    if (ret != FMEDIA_DCDP_SUCCESS)
    {
        printf("FMediaDispInit running error!\n");
        goto err;
    }
    FMediaLvInit();
    FMediaLvConfig();
#if LV_USE_DEMO_BENCHMARK
    benchmark();
#endif

#if LV_USE_DEMO_STRESS
    stress();
#endif

#if LV_USE_DEMO_WIDGETS
    widgets();
#endif

err:
    /* print message on example run result */
    if (0 == ret)
    {
        printf("%s@%d: Serial poll example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Serial poll example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }

    return ret;
}
