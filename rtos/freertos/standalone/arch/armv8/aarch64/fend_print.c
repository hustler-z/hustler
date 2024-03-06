/*
 * Copyright : (C) 2024 Phytium Information Technology, Inc.
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
 * FilePath: fend_print.c
 * Created Date: 2024-02-19 10:39:53
 * Last Modified: 2024-02-19 10:40:30
 * Description:  This file is for print end flag.
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0  liqiaozhong  2024/2/19  add end print message
 */

#include "sdkconfig.h"
#include "fprintk.h"

#ifdef CONFIG_USE_END_PRINT
void FEndPrint(void)
{
    f_printk("[system_shutdown]");
    while(1);
}
#endif