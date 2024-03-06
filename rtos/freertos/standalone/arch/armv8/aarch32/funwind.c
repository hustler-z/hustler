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
 * FilePath: funwind.c
 * Created Date: 2023-11-24 13:20:28
 * Last Modified: 2023-12-07 11:28:40
 * Description:  This file is for aarch32 unwind function 
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      huanghe   2023-11-24        first release
 */
#include "funwind.h"
#include "ftypes.h"
#include "ferror_code.h"

#include "fdebug.h"

#define FUNWIND_DEBUG_TAG "UNWIND"
#define FUNWIND_DEBUG(format, ...)     FT_DEBUG_PRINT_D(FUNWIND_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUNWIND_INFO(format, ...)      FT_DEBUG_PRINT_I(FUNWIND_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUNWIND_WARN(format, ...)      FT_DEBUG_PRINT_W(FUNWIND_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUNWIND_ERROR(format, ...)     FT_DEBUG_PRINT_E(FUNWIND_DEBUG_TAG, format, ##__VA_ARGS__)


extern const struct FUnwindIndexEntries __exidx_start[] ;
extern const struct FUnwindIndexEntries __exidx_end[] ;

extern u32 __supervisor_stack_end; 
extern u32 __supervisor_stack; 

extern u32 __irq_stack_end; 
extern u32 __irq_stack; 

extern u32 __sys_stack_end; 
extern u32 __sys_stack; 

extern u32 __abort_stack_end;
extern u32 __abort_stack;

extern u32 __fiq_stack_end; 
extern u32 __fiq_stack; 

extern u32 __undef_stack_end;  
extern u32 __undef_stack; 



#define UNWIND_EXIDX_CANTUNWIND 1
/* for generic model */
#define FUNWIND_PREL31_TO_ADDR(ptr) \
    ({                                       \
        	/* sign-extend to 32 bits */			\
        long offset = (((long)*(ptr)) << 1) >> 1;	\
        (u32)(ptr) + offset;			\
    })


static const struct FUnwindIndexEntries * __origin_unwind_idx = NULL ;
// const struct FUnwindIndexEntries * origin  = NULL ;
extern const struct FUnwindIndexEntries __exidx_start[] ;
extern const struct FUnwindIndexEntries __exidx_end[] ;


static const struct FUnwindIndexEntries *FUnwindFindOrigin(const struct FUnwindIndexEntries * start,
                                                        const struct FUnwindIndexEntries * stop)
{
	FUNWIND_DEBUG("%s(%x, %x)", __func__, start, stop);
	while (start < stop)
	{
		const struct FUnwindIndexEntries * mid = start + ((stop - start) >> 1);

		/* The first word contains a prel31 offset to the start of a function, with bit 31 clear. */
		if(mid->addr_offset >= 0x40000000)
		{
			start = mid + 1;
		}
		else
		{
			stop = mid ;
		}
	}
	FUNWIND_DEBUG("%s -> %x", __func__, stop);
	return stop;
}

static const struct FUnwindIndexEntries * FUnwindSearchLinkIndex(uintptr addr,
                                                        const struct FUnwindIndexEntries * start,
                                                        const struct FUnwindIndexEntries * origin,
                                                        const struct FUnwindIndexEntries * stop)
{
    uintptr addr_prel31 ;

	FUNWIND_DEBUG("%s(%08lx, %x, %x, %x)",
            __func__, addr, start, origin, stop);

    if(addr < (uintptr)start)
    {
        stop = origin;
    }
    else
    {
        start = origin;
    }

    addr_prel31 = (addr - (uintptr)start)  & 0x7fffffff;
    
    while (start < stop - 1)
    {
        const struct FUnwindIndexEntries * mid = start + ((stop - start) >> 1);
        
        /*
		 * As addr_prel31 is relative to start an offset is needed to
		 * make it relative to mid.
		 */
        if (addr_prel31 - ((uintptr)mid - (uintptr)start) <
				mid->addr_offset)
        {
            stop = mid;
        }
		else {
			/* keep addr_prel31 relative to start */
			addr_prel31 -= ((uintptr)mid -
					(uintptr)start);
			start = mid;
		}
    }
    
    if(start->addr_offset <= addr_prel31)
    {
        return start ;
    }
    else
    {
		FUNWIND_WARN("unwind: Unknown symbol address %08lx", addr);
        return NULL ;
    }

}

static u32 FUnwindGetByte(struct FUnwindRegs *regs_p)
{
    u32 ret;

    if(regs_p->entries <= 0)
    {
        return 0 ;
    }

    ret = (*regs_p->insn >> (regs_p->byte * 8)) & 0xff;

	if (regs_p->byte == 0) 
    {
		regs_p->insn++;
		regs_p->entries--;
		regs_p->byte = 3;
	} 
    else
    {
        regs_p->byte--;
    }
		
	return ret;
}



/* Before poping a register check whether it is feasible or not */
static int FUnwindPopRegister(struct FUnwindRegs *regs_p,
				u32 **vsp, u32 reg)
{
	if (!regs_p->check_each_pop)
	{
		if (*vsp >= (u32 *)regs_p->sp_high)
		{
			return FUNWIND_INSN_ERROR;
		}
	}
		
	regs_p->regs[reg] = *(*vsp);
	(*vsp)++;
	return FUNWIND_SUCCESS;
}

static int FUwindExecPopR4TorN(struct FUnwindRegs *regs_p,
					unsigned long insn)
{
	unsigned long *vsp = (unsigned long *)regs_p->arm_sp;
	int reg;

	/* pop R4-R[4+bbb] */
	for (reg = 4; reg <= 4 + (insn & 7); reg++)
	{
		if (FUnwindPopRegister(regs_p, &vsp, reg))
		{
				return FUNWIND_INSN_ERROR;
		}
	}
		
	if (insn & 0x8)
	{
		if (FUnwindPopRegister(regs_p, &vsp, 14))
		{
			return FUNWIND_INSN_ERROR;
		}
	}
		
	regs_p->arm_sp = (unsigned long)vsp;

	return FUNWIND_SUCCESS;
}

static FError FUnwindExecPopSubsetR4ToR13(struct FUnwindRegs *regs_p,
						u32 mask)
{
	u32 *vsp = (u32 *)regs_p->arm_sp;
	int load_sp, reg = 4;

	load_sp = mask & (1 << (13 - 4));
	while (mask) 
	{
		if (mask & 1)
		{
			if (FUnwindPopRegister(regs_p, &vsp, reg))
			{
				return FUNWIND_INSN_ERROR;
			}
		}
			
		mask >>= 1;
		reg++;
	}
	if (!load_sp)
	{
		regs_p->arm_sp = (u32)vsp;
	}
		

	return FUNWIND_SUCCESS;
}

static FError FUnwindExecPopSubsetR0ToR3(struct FUnwindRegs *regs_p,
						unsigned long mask)
{
	unsigned long *vsp = (unsigned long *)regs_p->arm_sp;
	int reg = 0;

	/* pop R0-R3 according to mask */
	while (mask) 
	{
		if (mask & 1)
		{
			if (FUnwindPopRegister(regs_p, &vsp, reg))
			{
				return FUNWIND_INSN_ERROR;
			}
		}
		mask >>= 1;
		reg++;
	}
	regs_p->arm_sp = (unsigned long)vsp;

	return FUNWIND_SUCCESS;
}


static FError FUnwindExecInsn(struct FUnwindRegs *regs_p)
{
    u32 insn = FUnwindGetByte(regs_p);
	FError ret = FUNWIND_SUCCESS;

	FUNWIND_DEBUG("%s: insn = %08lx\n", __func__, insn);

	if ((insn & 0xc0) == 0x00)
	{
		regs_p->arm_sp += ((insn & 0x3f) << 2) + 4;
	}
	else if ((insn & 0xc0) == 0x40)
	{
		regs_p->arm_sp -= ((insn & 0x3f) << 2) + 4;
	}
	else if ((insn & 0xf0) == 0x80) 
	{
		u32 mask;

		insn = (insn << 8) | FUnwindGetByte(regs_p);
		mask = insn & 0x0fff;
		if (mask == 0) 
		{
            FUNWIND_WARN("unwind: 'Refuse to unwind' instruction %04lx",
                    insn);
			return FUNWIND_INSN_ERROR;
		}

		ret = FUnwindExecPopSubsetR4ToR13(regs_p, mask);
		if (ret)
			goto error;
	} 
	else if ((insn & 0xf0) == 0x90 &&
		   (insn & 0x0d) != 0x0d)
	{
		regs_p->arm_sp = regs_p->regs[insn & 0x0f];
	}
	else if ((insn & 0xf0) == 0xa0) 
	{
		ret = FUwindExecPopR4TorN(regs_p, insn);
		if (ret)
		{
			goto error;
		}
	} else if (insn == 0xb0) 
	{
		if (regs_p->arm_pc == 0)
		{
			regs_p->arm_pc = regs_p->arm_lr;
		}
		/* no further processing */
		regs_p->entries = 0;
	} else if (insn == 0xb1) {
		u32 mask = FUnwindGetByte(regs_p);

		if (mask == 0 || mask & 0xf0) 
		{
            FUNWIND_WARN("unwind: Spare encoding %04lx",
                    (insn << 8) | mask);
			return FUNWIND_INSN_ERROR;
		}

		ret = FUnwindExecPopSubsetR0ToR3(regs_p, mask);
		if (ret)
			goto error;
	} else if (insn == 0xb2) {
		u32 uleb128 = FUnwindGetByte(regs_p);

		regs_p->arm_sp += 0x204 + (uleb128 << 2);
	} else {
		FUNWIND_WARN("unwind: Unhandled instruction %02lx", insn);
		return FUNWIND_INSN_ERROR;
	}

    FUNWIND_DEBUG("%s: fp = %08lx sp = %08lx lr = %08lx pc = %08lx", __func__,
            regs_p->arm_fp, regs_p->arm_sp, regs_p->arm_lr, regs_p->arm_pc);

error:
	return ret;
}

static const struct FUnwindIndexEntries *FUnwindSearchIndex(uintptr addr,
                                                        const struct FUnwindIndexEntries * start,
                                                        const struct FUnwindIndexEntries ** origin,
                                                        const struct FUnwindIndexEntries * stop)
{
    uintptr addr_prel31 ;
    const struct FUnwindIndexEntries * idx ;
	
    if(*origin == NULL)
    {
        *origin = FUnwindFindOrigin(start,stop) ;
    }
	
    idx = FUnwindSearchLinkIndex(addr,start,*origin,stop) ;

	FUNWIND_DEBUG("%s: idx = %x", __func__, idx);

    return idx ;
}   

FError FUnwindFrame(struct FUnwindStackFrame *frame,
					const struct FUnwindIndexEntries * start,
					const struct FUnwindIndexEntries ** origin,
					const struct FUnwindIndexEntries * stop)
{
    uintptr low ;
    const struct FUnwindIndexEntries * idx ;
    struct FUnwindRegs regs;

    /* store the highest address on the stack to avoid crossing it*/
    low = frame->sp ;
    regs.sp_high =  frame->sp_high;  /* user select what stack you want */
    
	FUNWIND_DEBUG("%s(pc = %08lx lr = %08lx sp = %08lx)", __func__,
            frame->pc, frame->lr, frame->sp);

    idx = FUnwindSearchIndex(frame->pc,start,origin,stop) ;

    if(idx == NULL)
    {
        return FUNWIND_IDX_NOT_FOUND;
    }

    regs.arm_fp = frame->fp;
	regs.arm_sp = frame->sp;
	regs.arm_lr = frame->lr;
	regs.arm_pc = 0;


    if(idx->insn == UNWIND_EXIDX_CANTUNWIND) /* run-time support code that associated frames cannot be unwound */
    {
        return FUNWIND_IDX_CANTUNWIND ;
    }
    else if((idx->insn & 1UL<<31) == 0 ) /* generic model  */
    {
        regs.insn =(const unsigned long *) FUNWIND_PREL31_TO_ADDR(&idx->insn) ;
    }
    else if((idx->insn & 0xff000000) == 0x80000000) /*  compact model */
    {
        regs.insn = &idx->insn ;
    }
    else
    {
		FUNWIND_WARN("unwind: Unsupported personality routine %08lx in the index at %x",
                idx->insn, idx);
        return FUNWIND_INSN_ERROR;
    }

    /* check the personality routine */
    if((*(const u32 *)(regs.insn) & 0xff000000) == 0x80000000)
    {
        regs.byte = 2;
		regs.entries = 1;
    }
    else if((*(const u32 *)(regs.insn) & 0xff000000) == 0x81000000)
    {
		regs.byte = 1;
		regs.entries = 1 + ((*(const u32 *)regs.insn & 0x00ff0000) >> 16);
    }
    else
    {
        return FUNWIND_INSN_ERROR;
    }

    regs.check_each_pop = 0;
    
    while (regs.entries > 0) 
	{
		int urc;
		if ((regs.sp_high - regs.arm_sp) < sizeof(regs.regs))
			regs.check_each_pop = 1;
		urc = FUnwindExecInsn(&regs);
		if (urc)
		{
			return urc;
		}
		if (regs.arm_sp < low || regs.arm_sp >= regs.sp_high)
		{
			return FUNWIND_INSN_ERROR;
		}
			
	}

    if (regs.arm_pc == 0)
	{
		regs.arm_pc = regs.arm_lr;
	}
		

	/* check for infinite loop */
	if (frame->pc == regs.arm_pc && frame->sp == regs.arm_sp)
	{
		return FUNWIND_INSN_ERROR;
	}
		
	frame->fp = regs.arm_fp;
	frame->sp = regs.arm_sp;
	frame->lr = regs.arm_lr;
	frame->pc = regs.arm_pc;

	return FUNWIND_SUCCESS;

}

static u32 *FUwindCheckSpHigh(u32 sp)
{
	if (sp <= (u32)(&__supervisor_stack) && sp > (u32)(&__supervisor_stack_end)) 
	{
        return &__supervisor_stack;
    } else if (sp <= (u32)(&__irq_stack) && sp > (u32)(&__irq_stack_end)) 
	{
		return &__irq_stack;
    } else if (sp <= (u32)(&__sys_stack) && sp > (u32)(&__sys_stack_end)) 
	{
		return &__sys_stack;
    } else if (sp <= (u32)(&__abort_stack) && sp > (u32)(&__abort_stack_end)) 
	{
		return &__abort_stack;
    } else if (sp <= (u32)(&__fiq_stack) && sp > (u32)(&__fiq_stack_end)) 
	{
		return &__fiq_stack;
    } else if (sp <= (u32)(&__undef_stack) && sp > (u32)(&__undef_stack_end)) 
	{
		return &__undef_stack;
    }
    return 0;
}

/**
 * @name: FUnwindBacktrace
 * @msg: 执行堆栈回溯，用于调试和异常处理中确定调用栈的状态。
 * @return: 无返回值（void）
 * @note: 此函数被标记为_WEAK，它可以被同名函数覆盖。
 * @param {void *} regs: 如果需要自定义栈追溯起点，请按照struct FUnwindStackFrame * 数据结构进行传入 。
 */
_WEAK void FUnwindBacktrace(void * regs)
{
	FError ret ;
	const struct FUnwindIndexEntries * __exidx_origin = NULL;
	struct FUnwindStackFrame frame = {0} ;

	
	if(regs)
	{
		frame = *(struct FUnwindStackFrame *)regs ; 
	}
	else
	{
		frame.fp = (u32)__builtin_frame_address(0);
		__asm__ volatile ("mov %0, sp":"=r"(frame.sp));
		frame.lr = (u32)__builtin_return_address(0);
		__asm__ volatile ("mov %0, pc":"=r"(frame.pc));
	}
    
    printf("please use: addr2line -e <project name>.elf -a -f %08x", frame.pc);
	while (1)
	{
		/* Determine the sp_high */
		frame.sp_high = (u32)FUwindCheckSpHigh(frame.sp) ;
		if(frame.sp_high == 0)
		{
			FUNWIND_ERROR("SP is not vaild in cpu space") ;
			return ;
		}
		ret = FUnwindFrame(&frame,__exidx_start,&__exidx_origin,__exidx_end) ;

		if(ret)
		{
			break;
		}
		printf(" %08x", frame.pc);
	}
	printf("\n") ;
}



