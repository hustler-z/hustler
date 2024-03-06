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
 * FilePath: spiffs_qspi_mount_example.h
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-08-1 18:47:54
 * Description:  This file is for sfud spiffs mount example function definition.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add sfud spiffs mount example
 *  1.1    zhangyan     2023/8/8     modify
 *  1.2    liqiaozhong  2023/10/8    divide example into spi and qspi parts
 */

#ifndef  SPIFFS_QSPI_MOUNT_EXAMPLE_H
#define  SPIFFS_QSPI_MOUNT_EXAMPLE_H
/***************************** Include Files *********************************/
#ifdef __cplusplus
extern "C" 
{
#endif
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/* entry function */
int FSpiffsQspiMountExample(void);

#ifdef __cplusplus
}
#endif

#endif