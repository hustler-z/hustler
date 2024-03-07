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
 * FilePath: lwip_ipv4_example.h
 * Created Date: 2023-11-21 16:00:10
 * Last Modified: 2023-11-21 16:01:20
 * Description:  This file is for lwip ipv4 example function definition.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/12/26          first release
 */
#ifndef  LWIP_IPV4_EXAMPLE_H
#define  LWIP_IPV4_EXAMPLE_H

/***************************** Include Files *********************************/
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* entry function for lwip ipv4 example */
int LwipIpv4TestCreate(void);
void LwipIpv4TestDeinit(void);

#ifdef __cplusplus
}
#endif

#endif