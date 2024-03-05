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
 * @FilePath: lvgl_example.h
 * @Date: 2023-07-13 19:42:37
 * @LastEditTime: 2023-07-13 19:42:37
 * @Description:  This file is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  liusm      2023/07/12      example test init
 */
#ifndef  LVGL_EXAMPLE_H
#define  LVGL_EXAMPLE_H

#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

FError FMediaLvglExample(void);

void FTimerLoopInit(void);

#ifdef __cplusplus
}
#endif

#endif