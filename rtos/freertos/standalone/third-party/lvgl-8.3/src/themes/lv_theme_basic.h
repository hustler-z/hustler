/*
 * Filename: d:\sdk\phytium-standalone-sdk-wzq\third-party\lvgl-8.3\src\themes\lv_theme_basic.h
 * Path: d:\sdk\phytium-standalone-sdk-wzq\third-party\lvgl-8.3\src\themes
 * Created Date: Tuesday, November 21st 2023, 11:04:16 am
 * Author: Simon Liu
 * 
 * Copyright (c) 2023 Phytium Information Technology, Inc.
 */



/**
 * @file lv_theme_basic.h
 *
 */

#ifndef LV_THEME_BASIC_H
#define LV_THEME_BASIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../core/lv_obj.h"
#include "../core/lv_theme.h"

#if LV_USE_THEME_BASIC

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the theme
 * @param disp pointer to display to attach the theme
 * @return a pointer to reference this theme later
 */
lv_theme_t * lv_theme_basic_init(lv_disp_t * disp);

/**
* Check if the theme is initialized
* @return true if default theme is initialized, false otherwise
*/
bool lv_theme_basic_is_inited(void);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_THEME_BASIC_H*/
