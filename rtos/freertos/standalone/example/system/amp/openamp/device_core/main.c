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
 * @FilePath: main.c
 * @Date: 2023-04-17 17:06:27
 * @LastEditTime: 2023-04-17 17:06:27
 * @Description:  This file is for 
 * 
 * @Modify History: 
 *  Ver     Who         Date        Changes
 * -----    ------      --------    --------------------------------------
 * 1.0  liushengming    2023/04/17  first release
 */
/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fpsci.h"
#include "fsleep.h"
#include "fprintk.h"
#include "fdebug.h"



/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define OPENAMP_MAIN_DEBUG_TAG "OPENAMP_MASTER_MAIN"
#define OPENAMP_MAIN_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)


/************************** Function Prototypes ******************************/

extern int rpmsg_demo_listening(int argc, char *argv[]) ;

int main(void)
{
    int ret = 0;
    
    while (!ret)
    {
        ret = rpmsg_demo_listening(0,NULL);
    }
}


