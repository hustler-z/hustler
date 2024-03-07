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
 * FilePath: fdcdp_audio.c
 * Created Date: 2023-09-11 09:19:16
 * Last Modified: Tue Oct 31 2023
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ----------------------------- ----
 */
#ifndef FDP_AUDIO_H
#define FDP_AUDIO_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fdp.h"
#include "fparameters.h"
#ifdef __cplusplus
extern "C"
{
#endif
/**************************** Type Definitions *******************************/

typedef struct 
{
   u32 sample_rate;
   u32 link_rate;
   u16 m;
   u16 n;
}FDcDpAudioMN;

/************************** Function Prototypes ******************************/

/*get the m and n value with the link_rate*/
const FDcDpAudioMN *FDpAudioGetMN(FDpCtrl *instance, u32 link_rate);

/*set the audio config */
FError FDpAudioSetPara(FDpCtrl *instance);

/*enable the audio*/
FError FDpAudioEnable(FDpCtrl *instance);

#ifdef __cplusplus
}
#endif

#endif