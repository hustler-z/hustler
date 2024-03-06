/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * FilePath: sys_arch.h
 * Date: 2022-07-18 13:25:02
 * LastEditTime: 2022-07-18 13:25:02
 * Description:  This file is part of the lwIP TCP/IP stack. 
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe      2022/11/1            first release
 */

#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include "sdkconfig.h"
#include "lwipopts.h"


#ifdef __cplusplus
extern "C"
{
#endif


sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t pval);

#ifdef __cplusplus
}
#endif



#endif /* __SYS_ARCH_H__ */
