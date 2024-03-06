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
 * FilePath: sys_arch_raw.c
 * Date: 2022-11-02 18:37:50
 * LastEditTime: 2022-11-02 18:37:50
 * Description:  This file is part of the lwIP TCP/IP stack.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe      2022/11/1            first release
 */
#include "arch/cc.h"
#include "lwip/sys.h"
#include "faarch.h"

/*
 * This optional function does a "fast" critical region protection and returns
 * the previous protection level. This function is only called during very short
 * critical regions. An embedded system which supports ISR-based drivers might
 * want to implement this function by disabling interrupts. Task-based systems
 * might want to implement this by using a mutex or disabling tasking. This
 * function should support recursive calls from the same task or interrupt. In
 * other words, sys_arch_protect() could be called while already protected. In
 * that case the return value indicates that it is already protected.
 * sys_arch_protect() is only required if your port is supporting an operating
 * system.
 */
sys_prot_t
sys_arch_protect(void)
{
	sys_prot_t cur;

	cur = MFCPSR();
	MTCPSR(cur | 0xC0);

	return cur;
}

/*
 * This optional function does a "fast" set of critical region protection to the
 * value specified by pval. See the documentation for sys_arch_protect() for
 * more information. This function is only required if your port is supporting
 * an operating system.
 */
void
sys_arch_unprotect(sys_prot_t lev)
{
	MTCPSR(lev);
}

