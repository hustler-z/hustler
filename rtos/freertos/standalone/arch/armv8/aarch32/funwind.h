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
 * FilePath: funwind.h
 * Created Date: 2023-11-24 13:20:38
 * Last Modified: 2023-12-07 11:18:07
 * Description:  This file is for aarch32 unwind function 
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      huanghe   2023-11-24        first release
 */
#ifndef FUNWIND_H
#define FUNWIND_H

#include "ftypes.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FUNWIND_SUCCESS                FT_SUCCESS
#define FUNWIND_IDX_NOT_FOUND          FT_MAKE_ERRCODE(ErrModBsp, ErrCommGeneral, 1)
#define FUNWIND_IDX_CANTUNWIND         FT_MAKE_ERRCODE(ErrModBsp, ErrCommGeneral, 2)
#define FUNWIND_INSN_ERROR             FT_MAKE_ERRCODE(ErrModBsp, ErrCommGeneral, 3)
#define FUNWIND_FAILURE                FT_MAKE_ERRCODE(ErrModBsp, ErrCommGeneral, 4)


#define FUNWIND_R0_INDEX 0
#define FUNWIND_R1_INDEX 1
#define FUNWIND_R2_INDEX 2 
#define FUNWIND_R3_INDEX 3
#define FUNWIND_R4_INDEX 4
#define FUNWIND_R5_INDEX 5
#define FUNWIND_R6_INDEX 6
#define FUNWIND_R7_INDEX 7
#define FUNWIND_R8_INDEX 8
#define FUNWIND_R9_INDEX 9
#define FUNWIND_R10_INDEX 10
#define FUNWIND_R11_INDEX 11
#define FUNWIND_R12_INDEX 12
#define FUNWIND_R13_INDEX 13
#define FUNWIND_R14_INDEX 14
#define FUNWIND_R15_INDEX 15
#define FUNWIND_REG_LENGTH 16

#define FUNWIND_FP_INDEX  FUNWIND_R11_INDEX
#define FUNWIND_SP_INDEX  FUNWIND_R13_INDEX
#define FUNWIND_LR_INDEX  FUNWIND_R14_INDEX
#define FUNWIND_LR_INDEX  FUNWIND_R14_INDEX


struct FUnwindIndexEntries
{
    u32 addr_offset ; /* The first word contains a prel31 offset to the start of a function, with bit 31 clear. */
    u32 insn  ; /* The prel31 offset of the start of the table entry for this function, with bit 31 clear.
                    The exception-handling table entry itself with bit 31 set, if it can be encoded in 31 bits .
                    The special bit pattern EXIDX_CANTUNWIND (0x1), 
                    indicating to run-time support code that associated frames cannot be unwound. 
                    On encountering this pattern the language-independent unwinding routines return a failure code to their caller, 
                    which should take an appropriate action such as calling terminate() or abort().  */
} ;


struct FUnwindStackFrame
{
    u32 fp;
    u32 sp;
    u32 lr;
    u32 pc;
    
    u32 sp_high ;
} ;


struct FUnwindRegs
{
    u32 regs[FUNWIND_REG_LENGTH + 2] ; /* r0~r15 adds origin r0 and cpsr */

	const unsigned long *insn;	/* pointer to the current instructions word */
	unsigned long sp_high;		/* highest value of sp allowed */
	/*
	 * 1 : check for stack overflow for each register pop.
	 * 0 : save overhead if there is plenty of stack remaining.
	 */
	int check_each_pop;
	int entries;			/* number of entries left to interpret */
	int byte;			/* current byte number in the instructions word */
} ;



#define arm_cpsr    regs[16]
#define arm_pc      regs[FUNWIND_R15_INDEX]
#define arm_lr      regs[FUNWIND_R14_INDEX]
#define arm_sp      regs[FUNWIND_R13_INDEX]
#define arm_ip      regs[FUNWIND_R12_INDEX]
#define arm_fp      regs[FUNWIND_R11_INDEX]
#define arm_r10     regs[FUNWIND_R10_INDEX]
#define arm_r9      regs[FUNWIND_R9_INDEX]
#define arm_r8      regs[FUNWIND_R8_INDEX]
#define arm_r7      regs[FUNWIND_R7_INDEX]
#define arm_r6      regs[FUNWIND_R6_INDEX]
#define arm_r5      regs[FUNWIND_R5_INDEX]
#define arm_r4      regs[FUNWIND_R4_INDEX]
#define arm_r3      regs[FUNWIND_R3_INDEX]
#define arm_r2      regs[FUNWIND_R2_INDEX]
#define arm_r1      regs[FUNWIND_R1_INDEX]
#define arm_r0      regs[FUNWIND_R0_INDEX]
#define arm_orig_r0 regs[17]

FError FUnwindFrame(struct FUnwindStackFrame *frame,
					const struct FUnwindIndexEntries * start,
					const struct FUnwindIndexEntries ** origin,
					const struct FUnwindIndexEntries * stop) ;

void FUnwindBacktrace(void * regs) ;

#ifdef __cplusplus
}
#endif

#endif


