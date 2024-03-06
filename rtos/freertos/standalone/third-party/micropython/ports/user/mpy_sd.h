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
 * FilePath: mpy_sd.c
 * Created Date: 2023-12-01 15:22:57
 * Last Modified: 2024-01-25 16:37:16
 * Description:  This file is for the sd function of micropython
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */
#ifndef MPY_SD_H
#define MPY_SD_H


#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************
 DEFINE PUBLIC TYPES
 ******************************************************************************/
typedef struct
{
    mp_obj_base_t base;
    bool          enabled;
} pybsd_obj_t;

/******************************************************************************
 DECLARE EXPORTED DATA
 ******************************************************************************/
extern pybsd_obj_t pybsd_obj;
extern const mp_obj_type_t pyb_sd_type;
void pyb_sd_init_vfs(fs_user_mount_t *vfs);


#ifdef __cplusplus
}
#endif

#endif
