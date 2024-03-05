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
 * FilePath: fdp_edid.h
 * Created Date: 2023-10-12 09:27:04
 * Last Modified: Tue Oct 31 2023
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
  *  1.0  Wangzq     2023/10/12  Modify the format and establish the version
 */

#ifndef FDP_EDID_H
#define FDP_EDID_H
/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fdp.h"
#ifdef __cplusplus
extern "C"
{
#endif

/************************** Function Prototypes ******************************/
/* Get edid information form sink*/
FError FDpGetEdid(FDpCtrl *instance_p, u8 *buffer);

/*translate the edid information to the struct*/
FError FDpParseDpEdidDtdList(u8 *buffer, Auxtable *list);

FError FDpParseEdidDtdTable(u8 *buffer, Auxtable *list);

#ifdef __cplusplus
}
#endif

#endif