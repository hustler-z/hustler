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
 * FilePath: nested_interrupt_sgi_example.h
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-08-1 18:47:54
 * Description:  This file is for nested interrupt sgi test example function definition.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add nested interrupt sgi test example
 */

#ifndef  NESTED_INTERRUPT_SGI_EXAMPLE_H
#define  NESTED_INTERRUPT_SGI_EXAMPLE_H
/***************************** Include Files *********************************/
#ifdef __cplusplus
extern "C" 
{
#endif
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/* entry function for nested interrupt sgi test example */
int FNestedIntrSgiExample(void);

#ifdef __cplusplus
}
#endif

#endif