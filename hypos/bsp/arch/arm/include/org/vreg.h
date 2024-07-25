/**
 * Hustler's Project
 *
 * File:  vreg.h
 * Date:  2024/07/24
 * Usage:
 */

#ifndef _ORG_VREG_H
#define _ORG_VREG_H
// --------------------------------------------------------------
#include <org/esr.h>

/*
 * AArch32 Co-processor registers.
 *
 * Note that AArch64 requires many of these definitions in order to
 * support 32-bit guests.
 */
#define __ESR_CPREG_c0  0
#define __ESR_CPREG_c1  1
#define __ESR_CPREG_c2  2
#define __ESR_CPREG_c3  3
#define __ESR_CPREG_c4  4
#define __ESR_CPREG_c5  5
#define __ESR_CPREG_c6  6
#define __ESR_CPREG_c7  7
#define __ESR_CPREG_c8  8
#define __ESR_CPREG_c9  9
#define __ESR_CPREG_c10 10
#define __ESR_CPREG_c11 11
#define __ESR_CPREG_c12 12
#define __ESR_CPREG_c13 13
#define __ESR_CPREG_c14 14
#define __ESR_CPREG_c15 15

#define __ESR_CPREG_0   0
#define __ESR_CPREG_1   1
#define __ESR_CPREG_2   2
#define __ESR_CPREG_3   3
#define __ESR_CPREG_4   4
#define __ESR_CPREG_5   5
#define __ESR_CPREG_6   6
#define __ESR_CPREG_7   7

#define _ESR_CPREG32(cp,op1,crn,crm,op2)          \
    ((__ESR_CPREG_##crn) << ESR_CP32_CRN_SHIFT) | \
    ((__ESR_CPREG_##crm) << ESR_CP32_CRM_SHIFT) | \
    ((__ESR_CPREG_##op1) << ESR_CP32_OP1_SHIFT) | \
    ((__ESR_CPREG_##op2) << ESR_CP32_OP2_SHIFT)

#define _ESR_CPREG64(cp,op1,crm)                  \
    ((__ESR_CPREG_##crm) << ESR_CP64_CRM_SHIFT) | \
    ((__ESR_CPREG_##op1) << ESR_CP64_OP1_SHIFT)

/* Encode a register as per ESR ISS pattern */
#define ESR_CPREG32(X...) _ESR_CPREG32(X)
#define ESR_CPREG64(X...) _ESR_CPREG64(X)
// --------------------------------------------------------------
/*
 * Order registers by Coprocessor-> CRn-> Opcode 1-> CRm-> Opcode 2
 *
 * This matches the ordering used in the ARM as well as the
 * groupings which the CP registers are allocated in.
 *
 * This is slightly different to the form of the instruction
 * arguments, which are cp,opc1,crn,crm,opc2.
 */

/* Coprocessor 10 */
#define FPSID           p10,7,c0,c0,0   /* Floating-Point System ID Register */
#define FPSCR           p10,7,c1,c0,0   /* Floating-Point Status and Control Register */
#define MVFR0           p10,7,c7,c0,0   /* Media and VFP Feature Register 0 */
#define MVFR1           p10,7,c6,c0,0   /* Media and VFP Feature Register 1 */
#define MVFR2           p10,7,c5,c0,0   /* Media and VFP Feature Register 2 */
#define FPEXC           p10,7,c8,c0,0   /* Floating-Point Exception Control Register */
#define FPINST          p10,7,c9,c0,0   /* Floating-Point Instruction Register */
#define FPINST2         p10,7,c10,c0,0  /* Floating-point Instruction Register 2 */

// --------------------------------------------------------------
/* XXX: CP14 */

/* CP14 0: Debug Register interface */
#define DBGDIDR         p14,0,c0,c0,0   /* Debug ID Register */
#define DBGDSCRINT      p14,0,c0,c1,0   /* Debug Status and Control Internal */
#define DBGDSCREXT      p14,0,c0,c2,2   /* Debug Status and Control External */
#define DBGDTRRXINT     p14,0,c0,c5,0   /* Debug Data Transfer Register, Receive */
#define DBGDTRTXINT     p14,0,c0,c5,0   /* Debug Data Transfer Register, Transmit */
#define DBGVCR          p14,0,c0,c7,0   /* Vector Catch */
#define DBGBVR0         p14,0,c0,c0,4   /* Breakpoint Value 0 */
#define DBGBCR0         p14,0,c0,c0,5   /* Breakpoint Control 0 */
#define DBGWVR0         p14,0,c0,c0,6   /* Watchpoint Value 0 */
#define DBGWCR0         p14,0,c0,c0,7   /* Watchpoint Control 0 */
#define DBGBVR1         p14,0,c0,c1,4   /* Breakpoint Value 1 */
#define DBGBCR1         p14,0,c0,c1,5   /* Breakpoint Control 1 */
#define DBGOSLAR        p14,0,c1,c0,4   /* OS Lock Access */
#define DBGOSLSR        p14,0,c1,c1,4   /* OS Lock Status Register */
#define DBGOSDLR        p14,0,c1,c3,4   /* OS Double Lock */
#define DBGPRCR         p14,0,c1,c4,4   /* Debug Power Control Register */

/* CP14 CR0: */
#define TEECR           p14,6,c0,c0,0   /* ThumbEE Configuration Register */

/* CP14 CR1: */
#define DBGDRAR64       p14,0,c1        /* Debug ROM Address Register (64-bit access) */
#define DBGDRAR         p14,0,c1,c0,0   /* Debug ROM Address Register (32-bit access) */
#define TEEHBR          p14,6,c1,c0,0   /* ThumbEE Handler Base Register */
#define JOSCR           p14,7,c1,c0,0   /* Jazelle OS Control Register */

/* CP14 CR2: */
#define DBGDSAR64       p14,0,c2        /* Debug Self Address Offset Register (64-bit access) */
#define DBGDSAR         p14,0,c2,c0,0   /* Debug Self Address Offset Register (32-bit access) */
#define JMCR            p14,7,c2,c0,0   /* Jazelle Main Configuration Register */

// --------------------------------------------------------------
/* XXX: CP15 */

/* CP15 CR0: CPUID and Cache Type Registers */
#define MIDR            p15,0,c0,c0,0   /* Main ID Register */
#define CTR             p15,0,c0,c0,1   /* Cache Type Register */
#define MPIDR           p15,0,c0,c0,5   /* Multiprocessor Affinity Register */
#define ID_PFR0         p15,0,c0,c1,0   /* Processor Feature Register 0 */
#define ID_PFR1         p15,0,c0,c1,1   /* Processor Feature Register 1 */
#define ID_PFR2         p15,0,c0,c3,4   /* Processor Feature Register 2 */
#define ID_DFR0         p15,0,c0,c1,2   /* Debug Feature Register 0 */
#define ID_DFR1         p15,0,c0,c3,5   /* Debug Feature Register 1 */
#define ID_AFR0         p15,0,c0,c1,3   /* Auxiliary Feature Register 0 */
#define ID_MMFR0        p15,0,c0,c1,4   /* Memory Model Feature Register 0 */
#define ID_MMFR1        p15,0,c0,c1,5   /* Memory Model Feature Register 1 */
#define ID_MMFR2        p15,0,c0,c1,6   /* Memory Model Feature Register 2 */
#define ID_MMFR3        p15,0,c0,c1,7   /* Memory Model Feature Register 3 */
#define ID_MMFR4        p15,0,c0,c2,6   /* Memory Model Feature Register 4 */
#define ID_MMFR5        p15,0,c0,c3,6   /* Memory Model Feature Register 5 */
#define ID_ISAR0        p15,0,c0,c2,0   /* ISA Feature Register 0 */
#define ID_ISAR1        p15,0,c0,c2,1   /* ISA Feature Register 1 */
#define ID_ISAR2        p15,0,c0,c2,2   /* ISA Feature Register 2 */
#define ID_ISAR3        p15,0,c0,c2,3   /* ISA Feature Register 3 */
#define ID_ISAR4        p15,0,c0,c2,4   /* ISA Feature Register 4 */
#define ID_ISAR5        p15,0,c0,c2,5   /* ISA Feature Register 5 */
#define ID_ISAR6        p15,0,c0,c2,7   /* ISA Feature Register 6 */
#define CCSIDR          p15,1,c0,c0,0   /* Cache Size ID Registers */
#define CLIDR           p15,1,c0,c0,1   /* Cache Level ID Register */
#define CSSELR          p15,2,c0,c0,0   /* Cache Size Selection Register */
#define VPIDR           p15,4,c0,c0,0   /* Virtualization Processor ID Register */
#define VMPIDR          p15,4,c0,c0,5   /* Virtualization Multiprocessor ID Register */

/* CP15 CR1: System Control Registers */
#define SCTLR           p15,0,c1,c0,0   /* System Control Register */
#define ACTLR           p15,0,c1,c0,1   /* Auxiliary Control Register */
#define CPACR           p15,0,c1,c0,2   /* Coprocessor Access Control Register */
#define SCR             p15,0,c1,c1,0   /* Secure Configuration Register */
#define NSACR           p15,0,c1,c1,2   /* Non-Secure Access Control Register */
#define HSCTLR          p15,4,c1,c0,0   /* Hyp. System Control Register */
#define HCR             p15,4,c1,c1,0   /* Hyp. Configuration Register */
#define HDCR            p15,4,c1,c1,1   /* Hyp. Debug Configuration Register */
#define HCPTR           p15,4,c1,c1,2   /* Hyp. Coprocessor Trap Register */
#define HSTR            p15,4,c1,c1,3   /* Hyp. System Trap Register */

/* CP15 CR2: Translation Table Base and Control Registers */
#define TTBCR           p15,0,c2,c0,2   /* Translation Table Base Control Register */
#define TTBCR2          p15,0,c2,c0,3   /* Translation Table Base Control Register 2 */
#define TTBR0           p15,0,c2        /* Translation Table Base Reg. 0 */
#define TTBR1           p15,1,c2        /* Translation Table Base Reg. 1 */
#define HTTBR           p15,4,c2        /* Hyp. Translation Table Base Register */
#define TTBR0_32        p15,0,c2,c0,0   /* 32-bit access to TTBR0 */
#define TTBR1_32        p15,0,c2,c0,1   /* 32-bit access to TTBR1 */
#define HTCR            p15,4,c2,c0,2   /* Hyp. Translation Control Register */
#define VTCR            p15,4,c2,c1,2   /* Virtualization Translation Control Register */
#define VTTBR           p15,6,c2        /* Virtualization Translation Table Base Register */

/* CP15 CR3: Domain Access Control Register */
#define DACR            p15,0,c3,c0,0   /* Domain Access Control Register */

/* CP15 CR4: */
#define ICC_PMR         p15,0,c4,c6,0   /* Interrupt Priority Mask Register */

/* CP15 CR5: Fault Status Registers */
#define DFSR            p15,0,c5,c0,0   /* Data Fault Status Register */
#define IFSR            p15,0,c5,c0,1   /* Instruction Fault Status Register */
#define ADFSR           p15,0,c5,c1,0   /* Auxiliary Data Fault Status Register */
#define AIFSR           p15,0,c5,c1,1   /* Auxiliary Instruction Fault Status Register */
#define ESR             p15,4,c5,c2,0   /* Hyp. Syndrome Register */

/* CP15 CR6: Fault Address Registers */
#define DFAR            p15,0,c6,c0,0   /* Data Fault Address Register  */
#define IFAR            p15,0,c6,c0,2   /* Instruction Fault Address Register */
#define HDFAR           p15,4,c6,c0,0   /* Hyp. Data Fault Address Register */
#define HIFAR           p15,4,c6,c0,2   /* Hyp. Instruction Fault Address Register */
#define HPFAR           p15,4,c6,c0,4   /* Hyp. IPA Fault Address Register */

/* CP15 CR7: Cache and address translation operations */
#define PAR             p15,0,c7        /* Physical Address Register */

#define ICIALLUIS       p15,0,c7,c1,0   /* Invalidate all instruction caches to PoU inner shareable */
#define BPIALLIS        p15,0,c7,c1,6   /* Invalidate entire branch predictor array inner shareable */
#define ICIALLU         p15,0,c7,c5,0   /* Invalidate all instruction caches to PoU */
#define ICIMVAU         p15,0,c7,c5,1   /* Invalidate instruction caches by MVA to PoU */
#define BPIALL          p15,0,c7,c5,6   /* Invalidate entire branch predictor array */
#define BPIMVA          p15,0,c7,c5,7   /* Invalidate MVA from branch predictor array */
#define DCIMVAC         p15,0,c7,c6,1   /* Invalidate data cache line by MVA to PoC */
#define DCISW           p15,0,c7,c6,2   /* Invalidate data cache line by set/way */
#define ATS1CPR         p15,0,c7,c8,0   /* Address Translation Stage 1. Non-Secure Kernel Read */
#define ATS1CPW         p15,0,c7,c8,1   /* Address Translation Stage 1. Non-Secure Kernel Write */
#define ATS1CUR         p15,0,c7,c8,2   /* Address Translation Stage 1. Non-Secure User Read */
#define ATS1CUW         p15,0,c7,c8,3   /* Address Translation Stage 1. Non-Secure User Write */
#define ATS12NSOPR      p15,0,c7,c8,4   /* Address Translation Stage 1+2 Non-Secure Kernel Read */
#define ATS12NSOPW      p15,0,c7,c8,5   /* Address Translation Stage 1+2 Non-Secure Kernel Write */
#define ATS12NSOUR      p15,0,c7,c8,6   /* Address Translation Stage 1+2 Non-Secure User Read */
#define ATS12NSOUW      p15,0,c7,c8,7   /* Address Translation Stage 1+2 Non-Secure User Write */
#define DCCMVAC         p15,0,c7,c10,1  /* Clean data or unified cache line by MVA to PoC */
#define DCCSW           p15,0,c7,c10,2  /* Clean data cache line by set/way */
#define DCCMVAU         p15,0,c7,c11,1  /* Clean data cache line by MVA to PoU */
#define DCCIMVAC        p15,0,c7,c14,1  /* Data cache clean and invalidate by MVA */
#define DCCISW          p15,0,c7,c14,2  /* Clean and invalidate data cache line by set/way */
#define ATS1HR          p15,4,c7,c8,0   /* Address Translation Stage 1 Hyp. Read */
#define ATS1HW          p15,4,c7,c8,1   /* Address Translation Stage 1 Hyp. Write */

/* CP15 CR8: TLB maintenance operations */
#define TLBIALLIS       p15,0,c8,c3,0   /* Invalidate entire TLB innrer shareable */
#define TLBIMVAIS       p15,0,c8,c3,1   /* Invalidate unified TLB entry by MVA inner shareable */
#define TLBIASIDIS      p15,0,c8,c3,2   /* Invalidate unified TLB by ASID match inner shareable */
#define TLBIMVAAIS      p15,0,c8,c3,3   /* Invalidate unified TLB entry by MVA all ASID inner shareable */
#define ITLBIALL        p15,0,c8,c5,0   /* Invalidate instruction TLB */
#define ITLBIMVA        p15,0,c8,c5,1   /* Invalidate instruction TLB entry by MVA */
#define ITLBIASID       p15,0,c8,c5,2   /* Invalidate instruction TLB by ASID match */
#define DTLBIALL        p15,0,c8,c6,0   /* Invalidate data TLB */
#define DTLBIMVA        p15,0,c8,c6,1   /* Invalidate data TLB entry by MVA */
#define DTLBIASID       p15,0,c8,c6,2   /* Invalidate data TLB by ASID match */
#define TLBIALL         p15,0,c8,c7,0   /* invalidate unified TLB */
#define TLBIMVA         p15,0,c8,c7,1   /* invalidate unified TLB entry by MVA */
#define TLBIASID        p15,0,c8,c7,2   /* invalid unified TLB by ASID match */
#define TLBIMVAA        p15,0,c8,c7,3   /* invalidate unified TLB entries by MVA all ASID */
#define TLBIALLHIS      p15,4,c8,c3,0   /* Invalidate Entire Hyp. Unified TLB inner shareable */
#define TLBIMVAHIS      p15,4,c8,c3,1   /* Invalidate Unified Hyp. TLB by MVA inner shareable */
#define TLBIALLNSNHIS   p15,4,c8,c3,4   /* Invalidate Entire Non-Secure Non-Hyp. Unified TLB inner shareable */
#define TLBIALLH        p15,4,c8,c7,0   /* Invalidate Entire Hyp. Unified TLB */
#define TLBIMVAH        p15,4,c8,c7,1   /* Invalidate Unified Hyp. TLB by MVA */
#define TLBIALLNSNH     p15,4,c8,c7,4   /* Invalidate Entire Non-Secure Non-Hyp. Unified TLB */

/* CP15 CR9: Performance monitors */
#define PMCR            p15,0,c9,c12,0  /* Perf. Mon. Control Register */
#define PMCNTENSET      p15,0,c9,c12,1  /* Perf. Mon. Count Enable Set register */
#define PMCNTENCLR      p15,0,c9,c12,2  /* Perf. Mon. Count Enable Clear register */
#define PMOVSR          p15,0,c9,c12,3  /* Perf. Mon. Overflow Flag Status Register */
#define PMSWINC         p15,0,c9,c12,4  /* Perf. Mon. Software Increment register */
#define PMSELR          p15,0,c9,c12,5  /* Perf. Mon. Event Counter Selection Register */
#define PMCEID0         p15,0,c9,c12,6  /* Perf. Mon. Common Event Identification register 0 */
#define PMCEID1         p15,0,c9,c12,7  /* Perf. Mon. Common Event Identification register 1 */
#define PMCCNTR         p15,0,c9,c13,0  /* Perf. Mon. Cycle Count Register */
#define PMXEVTYPER      p15,0,c9,c13,1  /* Perf. Mon. Event Type Select Register */
#define PMXEVCNTR       p15,0,c9,c13,2  /* Perf. Mon. Event Count Register */
#define PMUSERENR       p15,0,c9,c14,0  /* Perf. Mon. User Enable Register */
#define PMINTENSET      p15,0,c9,c14,1  /* Perf. Mon. Interrupt Enable Set Register */
#define PMINTENCLR      p15,0,c9,c14,2  /* Perf. Mon. Interrupt Enable Clear Register */
#define PMOVSSET        p15,0,c9,c14,3  /* Perf. Mon. Overflow Flag Status Set register */

/* CP15 CR10: */
#define MAIR0           p15,0,c10,c2,0  /* Memory Attribute Indirection Register 0 AKA PRRR */
#define MAIR1           p15,0,c10,c2,1  /* Memory Attribute Indirection Register 1 AKA NMRR */
#define HMAIR0          p15,4,c10,c2,0  /* Hyp. Memory Attribute Indirection Register 0 */
#define HMAIR1          p15,4,c10,c2,1  /* Hyp. Memory Attribute Indirection Register 1 */
#define AMAIR0          p15,0,c10,c3,0  /* Aux. Memory Attribute Indirection Register 0 */
#define AMAIR1          p15,0,c10,c3,1  /* Aux. Memory Attribute Indirection Register 1 */

/* CP15 CR11: DMA Operations for TCM Access */

/* CP15 CR12:  */
#define ICC_SGI1R       p15,0,c12       /* Interrupt Controller SGI Group 1 */
#define ICC_ASGI1R      p15,1,c12       /* Interrupt Controller Alias SGI Group 1 Register */
#define ICC_SGI0R       p15,2,c12       /* Interrupt Controller SGI Group 0 */
#define VBAR            p15,0,c12,c0,0  /* Vector Base Address Register */
#define ICC_DIR         p15,0,c12,c11,1 /* Interrupt Controller Deactivate Interrupt Register */
#define HVBAR           p15,4,c12,c0,0  /* Hyp. Vector Base Address Register */

/*
 * CP15 CR12: Interrupt Controller Hyp Active Priorities Group 0 Registers,
 * n = 0 - 3
 */
#define __AP0Rx(x)      p15, 4, c12, c8, x
#define ICH_AP0R0       __AP0Rx(0)
#define ICH_AP0R1       __AP0Rx(1)
#define ICH_AP0R2       __AP0Rx(2)
#define ICH_AP0R3       __AP0Rx(3)

/*
 * CP15 CR12: Interrupt Controller Hyp Active Priorities Group 1 Registers,
 * n = 0 - 3
 */
#define __AP1Rx(x)      p15, 4, c12, c9, x
#define ICH_AP1R0       __AP1Rx(0)
#define ICH_AP1R1       __AP1Rx(1)
#define ICH_AP1R2       __AP1Rx(2)
#define ICH_AP1R3       __AP1Rx(3)

#define ICC_IAR1        p15,0,c12,c12,0  /* Interrupt Controller Interrupt Acknowledge Register 1 */
#define ICC_EOIR1       p15,0,c12,c12,1  /* Interrupt Controller End Of Interrupt Register 1 */
#define ICC_BPR1        p15,0,c12,c12,3  /* Interrupt Controller Binary Point Register 1 */
#define ICC_CTLR        p15,0,c12,c12,4  /* Interrupt Controller Control Register */
#define ICC_SRE         p15,0,c12,c12,5  /* Interrupt Controller System Register Enable register */
#define ICC_IGRPEN1     p15,0,c12,c12,7  /* Interrupt Controller Interrupt Group 1 Enable register */
#define ICC_ESRE        p15,4,c12,c9,5   /* Interrupt Controller Hyp System Register Enable register */
#define ICH_HCR         p15,4,c12,c11,0  /* Interrupt Controller Hyp Control Register */
#define ICH_VTR         p15,4,c12,c11,1  /* Interrupt Controller VGIC Type Register */
#define ICH_MISR        p15,4,c12,c11,2  /* Interrupt Controller Maintenance Interrupt State Register */
#define ICH_EISR        p15,4,c12,c11,3  /* Interrupt Controller End of Interrupt Status Register */
#define ICH_ELRSR       p15,4,c12,c11,5  /* Interrupt Controller Empty List Register Status Register */
#define ICH_VMCR        p15,4,c12,c11,7  /* Interrupt Controller Virtual Machine Control Register */

/* CP15 CR12: Interrupt Controller List Registers, n = 0 - 15 */
#define __LR0(x)        p15, 4, c12, c12, x
#define __LR8(x)        p15, 4, c12, c13, x

#define ICH_LR0         __LR0(0)
#define ICH_LR1         __LR0(1)
#define ICH_LR2         __LR0(2)
#define ICH_LR3         __LR0(3)
#define ICH_LR4         __LR0(4)
#define ICH_LR5         __LR0(5)
#define ICH_LR6         __LR0(6)
#define ICH_LR7         __LR0(7)
#define ICH_LR8         __LR8(0)
#define ICH_LR9         __LR8(1)
#define ICH_LR10        __LR8(2)
#define ICH_LR11        __LR8(3)
#define ICH_LR12        __LR8(4)
#define ICH_LR13        __LR8(5)
#define ICH_LR14        __LR8(6)
#define ICH_LR15        __LR8(7)

/* CP15 CR12: Interrupt Controller List Registers, n = 0 - 15 */
#define __LRC0(x)       p15, 4, c12, c14, x
#define __LRC8(x)       p15, 4, c12, c15, x

#define ICH_LRC0        __LRC0(0)
#define ICH_LRC1        __LRC0(1)
#define ICH_LRC2        __LRC0(2)
#define ICH_LRC3        __LRC0(3)
#define ICH_LRC4        __LRC0(4)
#define ICH_LRC5        __LRC0(5)
#define ICH_LRC6        __LRC0(6)
#define ICH_LRC7        __LRC0(7)
#define ICH_LRC8        __LRC8(0)
#define ICH_LRC9        __LRC8(1)
#define ICH_LRC10       __LRC8(2)
#define ICH_LRC11       __LRC8(3)
#define ICH_LRC12       __LRC8(4)
#define ICH_LRC13       __LRC8(5)
#define ICH_LRC14       __LRC8(6)
#define ICH_LRC15       __LRC8(7)

/* CP15 CR13:  */
#define FCSEIDR         p15,0,c13,c0,0  /* FCSE Process ID Register */
#define CONTEXTIDR      p15,0,c13,c0,1  /* Context ID Register */
#define TPIDRURW        p15,0,c13,c0,2  /* Software Thread ID, User, R/W */
#define TPIDRURO        p15,0,c13,c0,3  /* Software Thread ID, User, R/O */
#define TPIDRPRW        p15,0,c13,c0,4  /* Software Thread ID, Priveleged */
#define HTPIDR          p15,4,c13,c0,2  /* HYp Software Thread Id Register */

/* CP15 CR14:  */
#define CNTPCT          p15,0,c14       /* Time counter value */
#define CNTFRQ          p15,0,c14,c0,0  /* Time counter frequency */
#define CNTKCTL         p15,0,c14,c1,0  /* Time counter kernel control */
#define CNTP_TVAL       p15,0,c14,c2,0  /* Physical Timer value */
#define CNTP_CTL        p15,0,c14,c2,1  /* Physical Timer control register */
#define CNTVCT          p15,1,c14       /* Time counter value + offset */
#define CNTP_CVAL       p15,2,c14       /* Physical Timer comparator */
#define CNTV_CVAL       p15,3,c14       /* Virt. Timer comparator */
#define CNTVOFF         p15,4,c14       /* Time counter offset */
#define CNTHCTL         p15,4,c14,c1,0  /* Time counter hyp. control */
#define CNTHP_TVAL      p15,4,c14,c2,0  /* Hyp. Timer value */
#define CNTHP_CTL       p15,4,c14,c2,1  /* Hyp. Timer control register */
#define CNTV_TVAL       p15,0,c14,c3,0  /* Virt. Timer value */
#define CNTV_CTL        p15,0,c14,c3,1  /* Virt. TImer control register */
#define CNTHP_CVAL      p15,6,c14       /* Hyp. Timer comparator */

// --------------------------------------------------------------
#include <org/mmio.h>

typedef bool (*vreg_reg64_fn_t)(struct hcpu_regs *regs, u64 *r,
                                   bool read);
typedef bool (*vreg_reg_fn_t)(struct hcpu_regs *regs, register_t *r,
                                   bool read);

static inline bool vreg_emulate_cp32(struct hcpu_regs *regs,
                                     union hcpu_esr hsr,
                                     vreg_reg_fn_t fn)
{
    struct hcpu_cp32 cp32 = hsr.cp32;

    register_t r = 0;
    bool ret;

    if (!cp32.read)
        r = get_hcpu_reg(regs, cp32.reg);

    ret = fn(regs, &r, cp32.read);

    if (ret && cp32.read)
        set_hcpu_reg(regs, cp32.reg, r);

    return ret;
}

static inline bool vreg_emulate_cp64(struct hcpu_regs *regs,
                                     union hcpu_esr hsr,
                                     vreg_reg64_fn_t fn)
{
    struct hcpu_cp64 cp64 = hsr.cp64;

    u64 x = 0;
    bool ret;

    if (!cp64.read) {
        u32 r1 = get_hcpu_reg(regs, cp64.reg1);
        u32 r2 = get_hcpu_reg(regs, cp64.reg2);

        x = (u64)r1 | ((u64)r2 << 32);
    }

    ret = fn(regs, &x, cp64.read);

    if (ret && cp64.read) {
        set_hcpu_reg(regs, cp64.reg1, x & 0xFFFFFFFFU);
        set_hcpu_reg(regs, cp64.reg2, x >> 32);
    }

    return ret;
}

static inline bool vreg_emulate_sysreg(struct hcpu_regs *regs,
                                       union hcpu_esr hsr,
                                       vreg_reg_fn_t fn)
{
    struct hcpu_sysreg sysreg = hsr.sysreg;
    register_t r = 0;
    bool ret;

    if (!sysreg.read)
        r = get_hcpu_reg(regs, sysreg.reg);

    ret = fn(regs, &r, sysreg.read);

    if (ret && sysreg.read)
        set_hcpu_reg(regs, sysreg.reg, r);

    return ret;
}

#define VREG_REG_MASK(size) ((~0UL) >> (BITS_PER_LONG - ((1 << (size)) * 8)))

#define VREG_REG_HELPERS(sz, offmask)                                   \
static inline register_t vreg_reg##sz##_extract(u##sz reg,              \
                                                const mmio_info_t *info)\
{                                                                       \
    unsigned int offset = info->gpa & (offmask);                        \
                                                                        \
    reg >>= 8 * offset;                                                 \
    reg &= VREG_REG_MASK(info->dabt.sas);                               \
                                                                        \
    return reg;                                                         \
}                                                                       \
                                                                        \
static inline void vreg_reg##sz##_update(u##sz *reg,                    \
                                         register_t val,                \
                                         const mmio_info_t *info)       \
{                                                                       \
    unsigned int offset = info->gpa & (offmask);                        \
    u##sz mask = VREG_REG_MASK(info->dabt.sas);                         \
    unsigned int shift = offset * 8;                                    \
                                                                        \
    *reg &= ~(mask << shift);                                           \
    *reg |= ((u##sz)val & mask) << shift;                               \
}                                                                       \
                                                                        \
static inline void vreg_reg##sz##_setbits(u##sz *reg,                   \
                                          register_t bits,              \
                                          const mmio_info_t *info)      \
{                                                                       \
    unsigned int offset = info->gpa & (offmask);                        \
    u##sz mask = VREG_REG_MASK(info->dabt.sas);                         \
    unsigned int shift = offset * 8;                                    \
                                                                        \
    *reg |= ((u##sz)bits & mask) << shift;                              \
}                                                                       \
                                                                        \
static inline void vreg_reg##sz##_clearbits(u##sz *reg,                 \
                                            register_t bits,            \
                                            const mmio_info_t *info)    \
{                                                                       \
    unsigned int offset = info->gpa & (offmask);                        \
    u##sz mask = VREG_REG_MASK(info->dabt.sas);                         \
    unsigned int shift = offset * 8;                                    \
                                                                        \
    *reg &= ~(((u##sz)bits & mask) << shift);                           \
}

VREG_REG_HELPERS(64, 0x7)
VREG_REG_HELPERS(32, 0x3)

// --------------------------------------------------------------
#endif /* _ORG_VREG_H */
