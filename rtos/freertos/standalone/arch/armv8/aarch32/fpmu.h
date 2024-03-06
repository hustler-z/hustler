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
 * FilePath: fpmu.h
 * Created Date: 2023-10-31 08:59:10
 * Last Modified: 2023-11-13 19:34:52
 * Description:  This file is for aarch32 pmu base interface
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      huanghe     2023/11/10      first release
 */

#ifndef FPMU_H
#define FPMU_H

#include "ftypes.h"
#include "ferror_code.h"
#include "faarch.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*
 * PMXEVTYPER: Event selection reg
 */
#define	FPMU_EVTYPE_MASK	0xc800ffff	/* Mask for writable bits */
#define	FPMU_EVTYPE_EVENT	0xffff		/* Mask for EVENT bits */


#define FPMU_ID_AA64DFR0_PMUVER_8_0		0x1
#define FPMU_ID_AA64DFR0_PMUVER_8_1		0x4
#define FPMU_ID_AA64DFR0_PMUVER_8_4		0x5
#define FPMU_ID_AA64DFR0_PMUVER_8_5		0x6
#define FPMU_ID_AA64DFR0_PMUVER_IMP_DEF	0xf

#define FPMCNT_ENCLR_ALL_MASK       (~0U)
#define PMINT_ENCLR_ALL_MASK    (~0U)


#define FPMU_PMCR_E	(1 << 0) /* Enable all counters */
#define FPMU_PMCR_P	(1 << 1) /* Reset all counters */
#define FPMU_PMCR_C	(1 << 2) /* Cycle counter reset */
#define FPMU_PMCR_D	(1 << 3) /* CCNT counts every 64th cpu cycle */
#define FPMU_PMCR_X	(1 << 4) /* Export to ETM */
#define FPMU_PMCR_DP	(1 << 5) /* Disable CCNT if non-invasive debug*/
#define FPMU_PMCR_LC	(1 << 6) /* Overflow on 64 bit cycle counter */
#define FPMU_PMCR_LP	(1 << 7) /* Long event counter enable */
#define PMCR_N_SHIFT    11
#define PMCR_N_MASK     0xf800 
#define PMCR_GET_N(x)  ((x & PMCR_N_MASK) >> PMCR_N_SHIFT)
#define	FPMU_PMCR_MASK	0xff	 /* Mask for writable bits */

#define FPMU_PMSELR 	15,0,9,12,5 
#define FPMU_PMXEVTYPER 15,0,9,13,1
#define FPMU_PMXEVCNTR  15,0,9,13,2
#define FPMU_PMINTENSET	15,0,9,14,1
#define FPMU_PMCNTENSET 15,0,9,12,1
#define FPMU_PMCNTENCLR 15,0,9,12,2
#define FPMU_PMINTENCLR 15,0,9,14,2
#define FPMU_PMOVSR		15,0,9,12,3
#define FPMU_PMCR		15,0,9,12,0
#define FPMU_PMCEID0	15,0,9,12,6
#define FPMU_PMCEID1	15,0,9,12,7
#define FPMU_PMCCFILTR  15,0,14,15,7
#define FPMU_PMCCNTR	15,0,9
#define FPMU_PMSWINC	15,0,9,12,4
#define FPMU_PMOVSCLR	15,0,9,12,3
#define FPMU_ID_DFR0	15,0,0,1,2

static inline void FPmuPmssel(u32 counter_id)
{
	AARCH32_WRITE_SYSREG_32(FPMU_PMSELR,counter_id) ;
	
}

static inline void FPmuWriteEventType(u32 counter_id,u32 event_type_config)
{
	FPmuPmssel(counter_id) ;
	
    event_type_config &= FPMU_EVTYPE_MASK ;
	AARCH32_WRITE_SYSREG_32(FPMU_PMXEVTYPER,event_type_config) ;
    
}

static inline u32 FPmuReadEventType(u32 counter_id)
{
	FPmuPmssel(counter_id) ;
	return AARCH32_READ_SYSREG_32(FPMU_PMXEVTYPER) ;
}


// #define RETURN_READ_PMEVCNTR(n) return AARCH32_READ_SYSREG_32(pmevcntr##n##_el0)
    
static inline u32 FPmuReadCycleCnt(u32 counter_id)
{
	FPmuPmssel(counter_id) ;
	return AARCH32_READ_SYSREG_32(FPMU_PMXEVCNTR);
}

static inline void FPmuWriteCycleCnt(u32 counter_id,u32 value)
{
	FPmuPmssel(counter_id) ;
	AARCH32_WRITE_SYSREG_32(FPMU_PMXEVCNTR,value) ;
}


static inline void FPmuEnableEventIrq(u32 mask)
{
	AARCH32_WRITE_SYSREG_32(FPMU_PMINTENSET,mask) ;
}


static inline u32 FPmuGetEventIrq(void)
{
	return  AARCH32_READ_SYSREG_32(FPMU_PMINTENSET) ;
}

static inline void FPmuEnableCounter(u32 mask)
{
    ISB() ;
	
	AARCH32_WRITE_SYSREG_32(FPMU_PMCNTENSET,mask) ;
}

static inline void FPmuDisableCounterMask(u32 mask)
{
    /* The counter and interrupt enable registers are unknown at reset. */
	AARCH32_WRITE_SYSREG_32(FPMU_PMCNTENCLR,mask) ;
    /*
	 * Make sure the effects of disabling the counter are visible before we
	 * start configuring the event.
	 */
	ISB();   

}

static inline void FPmuDisableIntens(u32 mask)
{
    AARCH32_WRITE_SYSREG_32(FPMU_PMINTENCLR,mask) ;

	ISB();

    AARCH32_WRITE_SYSREG_32(FPMU_PMOVSR,mask) ;
	ISB();
}



static inline void FPmuPmcrWrire(u32 pmcr)
{
    pmcr &= FPMU_PMCR_MASK ;
    ISB();
    AARCH32_WRITE_SYSREG_32(FPMU_PMCR,pmcr) ;
}

static inline u32 FPmuPmcrRead(void)
{
    return AARCH32_READ_SYSREG_32(FPMU_PMCR) ;
}

static inline u64 FPmuEdfrRead(void)
{
    return  (AARCH32_READ_SYSREG_32(FPMU_ID_DFR0) >> 16) &0xf0 ;
}

static inline u32 FPmuPmceid0(void)
{
    return AARCH32_READ_SYSREG_32(FPMU_PMCEID0) ;
}


static inline u32 FPmuPmceid1(void)
{
    return AARCH32_READ_SYSREG_32(FPMU_PMCEID1) ;
}

static inline void FPmuPmccfiltrSet(u32 event_type_config)
{
    AARCH32_WRITE_SYSREG_32(FPMU_PMCCFILTR,event_type_config) ;
}

static inline u32 FPmuPmccfiltrGet(void)
{
    AARCH32_READ_SYSREG_32(FPMU_PMCCFILTR) ;
}


static inline void FPmuPmccntrSet(u64 value)
{
    AARCH32_WRITE_SYSREG_64(FPMU_PMCCNTR,value) ;
}


static inline u64 FPmuPmccntrGet(void)
{
    return AARCH32_READ_SYSREG_64(FPMU_PMCCNTR) ;
}

static inline void FPmuPmswincSet(u32 mask)
{
    AARCH32_WRITE_SYSREG_32(FPMU_PMSWINC,mask) ;
}


static inline u32 FPmuPmovsclrGet(void)
{
    return AARCH32_READ_SYSREG_32(FPMU_PMOVSCLR) ;
}

/**
 * @name: FPmuGetRestIrqFlags
 * @msg: Retrieves and clears the IRQ flags indicating which PMU counters have overflowed.
 * @return {u32} - The IRQ flags for overflowed counters.
 */
static inline u32 FPmuGetRestIrqFlags(void)
{
    u32 value ;
    /* Read bit check for overflowed */
    value = AARCH32_READ_SYSREG_32(FPMU_PMOVSCLR) ;

    value &= 0xffffffff ;
    AARCH32_WRITE_SYSREG_32(FPMU_PMOVSCLR,value) ;
    return value ;
}



#ifdef __cplusplus
}
#endif

#endif
