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
 * FilePath: main.c
 * Created Date: 2023-06-16 13:26:03
 * Last Modified: 2023-08-31 16:05:17
 * Description:  This file shows us the main source code of rc
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe     2023-06-16          init code
 */



#include <stdio.h>
#include "sdkconfig.h"
#include "ftypes.h"
#if defined(CONFIG_USE_LETTER_SHELL)
#include "shell_port.h"
#endif



int main(void)
{
#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
    LSUserShellLoop();    
#else
    /* if shell command is not enabled, run example one by one */
    (void )FPcieConfigReadExample() ;
#endif
}



