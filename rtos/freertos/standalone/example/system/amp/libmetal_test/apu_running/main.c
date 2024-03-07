/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:56:24
 * Description:  This file is for running shell loop 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */


#include <stdio.h>
#include "sdkconfig.h"
#include "ftypes.h"
#include "shell_port.h"

extern int FLibmetalExample(void);

int main(void)
{
#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
    printf("start letter shell test...\r\n");
    LSUserShellLoop();  
#else
    /*run the atomic example*/
    FLibmetalExample();
#endif
    return 0;
}



