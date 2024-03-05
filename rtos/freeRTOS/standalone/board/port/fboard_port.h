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
 * FilePath: fboard_port.h
 * Created Date: 2023-10-27 17:02:35
 * Last Modified: 2023-10-27 09:22:20
 * Description:  This file is for board layer code decoupling
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0     zhangyan  2023/10/27    first release
 */
#ifndef FBOARD_PORT_H
#define FBOARD_PORT_H

#include "fdebug.h"
#include "sdkconfig.h"

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_I(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_E(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_D(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_W
#define FT_DEBUG_PRINT_W(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_V
#define FT_DEBUG_PRINT_V(TAG, format, ...)
#endif

#endif