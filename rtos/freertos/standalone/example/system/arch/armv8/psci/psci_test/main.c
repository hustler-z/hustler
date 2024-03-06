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
 * Last Modified: 2023-07-05 18:43:58
 * Description:  This file is for
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
#include "fsleep.h"
#include "psci_test.h"


int main(void)
{
    fsleep_seconds(2) ;
#if defined(CONFIG_USE_LETTER_SHELL)
    LSUserShellLoop();
#else
    FPsciFeatureTest() ;
    fsleep_seconds(10) ;
    FPsciHotplugTest() ;
#endif
}



