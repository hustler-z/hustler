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
 * FilePath: fbitmap.h
 * Created Date: 2023-10-31 18:09:18
 * Last Modified: 2023-11-15 09:42:05
 * Description:  This file is for bitmap
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0      huanghe     2023/11/10      first release
 */

#ifndef FBITMAP_H
#define FBITMAP_H

#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned long FBitPerWordType ;

#define FBIT_INVALID_BIT_INDEX (sizeof(FBitPerWordType))



void FBitMapSet(FBitPerWordType *bitmap,u16 pos) ;

void FBitMapClear(FBitPerWordType *bitmap,u16 pos) ;

u16 FBitMapHighGet(FBitPerWordType bitmap) ;

u16 FBitMapLowGet(FBitPerWordType bitmap) ;


void FBitMapSetNBits(FBitPerWordType *bitmap, u32 start, u32 nums_set) ;


void FBitMapClrNBits(FBitPerWordType *bitmap, u32 start, u32 nums_clear) ;

s32 FBitMapFfz(FBitPerWordType *bitmap, u32 num_bits) ;

void FBitMapCopyClearTail(FBitPerWordType *dst,const FBitPerWordType *src ,u32 nbits) ;


#ifdef __cplusplus
}
#endif


#endif

