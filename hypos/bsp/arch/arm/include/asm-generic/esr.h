/**
 * Hustler's Project
 *
 * File:  esr.h
 * Date:  2024/06/23
 * Usage:
 */

#ifndef _ASM_GENERIC_ESR_H
#define _ASM_GENERIC_ESR_H
// --------------------------------------------------------------

#include <common/type.h>

union hcpu_esr {
    register_t bits;

    struct {
        unsigned long iss:25;  /* Instruction Specific Syndrome */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    };

    /* Common to all conditional exception classes (0x0N, except 0x00). */
    struct esr_cond {
        unsigned long iss:20;  /* Instruction Specific Syndrome */
        unsigned long cc:4;    /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } cond;

    struct esr_wfi_wfe {
        unsigned long ti:1;    /* Trapped instruction */
        unsigned long sbzp:19;
        unsigned long cc:4;    /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } wfi_wfe;

    /* reg, reg0, reg1 are 4 bits on AArch32, the fifth bit is sbzp. */
    struct esr_cp32 {
        unsigned long read:1;  /* Direction */
        unsigned long crm:4;   /* CRm */
        unsigned long reg:5;   /* Rt */
        unsigned long crn:4;   /* CRn */
        unsigned long op1:3;   /* Op1 */
        unsigned long op2:3;   /* Op2 */
        unsigned long cc:4;    /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } cp32; /* HSR_EC_CP15_32, CP14_32, CP10 */

    struct esr_cp64 {
        unsigned long read:1;   /* Direction */
        unsigned long crm:4;    /* CRm */
        unsigned long reg1:5;   /* Rt1 */
        unsigned long reg2:5;   /* Rt2 */
        unsigned long sbzp2:1;
        unsigned long op1:4;    /* Op1 */
        unsigned long cc:4;     /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;    /* Instruction length */
        unsigned long ec:6;     /* Exception Class */
    } cp64; /* HSR_EC_CP15_64, HSR_EC_CP14_64 */

     struct esr_cp {
        unsigned long coproc:4; /* Number of coproc accessed */
        unsigned long sbz0p:1;
        unsigned long tas:1;    /* Trapped Advanced SIMD */
        unsigned long res0:14;
        unsigned long cc:4;     /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;    /* Instruction length */
        unsigned long ec:6;     /* Exception Class */
    } cp; /* HSR_EC_CP */

    struct esr_smc32 {
        unsigned long res0:19;  /* Reserved */
        unsigned long ccknownpass:1; /* Instruction passed conditional check */
        unsigned long cc:4;    /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } smc32; /* HSR_EC_SMC32 */

    struct esr_sysreg {
        unsigned long read:1;   /* Direction */
        unsigned long crm:4;    /* CRm */
        unsigned long reg:5;    /* Rt */
        unsigned long crn:4;    /* CRn */
        unsigned long op1:3;    /* Op1 */
        unsigned long op2:3;    /* Op2 */
        unsigned long op0:2;    /* Op0 */
        unsigned long res0:3;
        unsigned long len:1;    /* Instruction length */
        unsigned long ec:6;
    } sysreg; /* HSR_EC_SYSREG */

    struct esr_iabt {
        unsigned long ifsc:6;  /* Instruction fault status code */
        unsigned long res0:1;  /* RES0 */
        unsigned long s1ptw:1; /* Stage 2 fault during stage 1 translation */
        unsigned long res1:1;  /* RES0 */
        unsigned long eat:1;   /* External abort type */
        unsigned long fnv:1;   /* FAR not Valid */
        unsigned long res2:14;
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } iabt; /* HSR_EC_INSTR_ABORT_* */

    struct esr_dabt {
        unsigned long dfsc:6;  /* Data Fault Status Code */
        unsigned long write:1; /* Write / not Read */
        unsigned long s1ptw:1; /* Stage 2 fault during stage 1 translation */
        unsigned long cache:1; /* Cache Maintenance */
        unsigned long eat:1;   /* External Abort Type */
        unsigned long fnv:1;   /* FAR not Valid */

        unsigned long sbzp0:3;
        unsigned long ar:1;    /* Acquire Release */
        unsigned long sf:1;    /* Sixty Four bit register */

        unsigned long reg:5;   /* Register */
        unsigned long sign:1;  /* Sign extend */
        unsigned long size:2;  /* Access Size */
        unsigned long valid:1; /* Syndrome Valid */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } dabt; /* HSR_EC_DATA_ABORT_* */

    /* Contain the common bits between DABT and IABT */
    struct esr_xabt {
        unsigned long fsc:6;    /* Fault status code */
        unsigned long pad1:1;   /* Not common */
        unsigned long s1ptw:1;  /* Stage 2 fault during stage 1 translation */
        unsigned long pad2:1;   /* Not common */
        unsigned long eat:1;    /* External abort type */
        unsigned long fnv:1;    /* FAR not Valid */
        unsigned long pad3:14;  /* Not common */
        unsigned long len:1;    /* Instruction length */
        unsigned long ec:6;     /* Exception Class */
    } xabt;


    struct esr_brk {
        unsigned long comment:16;   /* Comment */
        unsigned long res0:9;
        unsigned long len:1;        /* Instruction length */
        unsigned long ec:6;         /* Exception Class */
    } brk;
};

#define ESR_ELx_EC_UNKNOWN	(0x00)
#define ESR_ELx_EC_WFx		(0x01)
/* Unallocated EC: 0x02 */
#define ESR_ELx_EC_CP15_32	(0x03)
#define ESR_ELx_EC_CP15_64	(0x04)
#define ESR_ELx_EC_CP14_MR	(0x05)
#define ESR_ELx_EC_CP14_LS	(0x06)
#define ESR_ELx_EC_FP_ASIMD	(0x07)
#define ESR_ELx_EC_CP10_ID	(0x08)	/* EL2 only */
#define ESR_ELx_EC_PAC		(0x09)	/* EL2 and above */
/* Unallocated EC: 0x0A - 0x0B */
#define ESR_ELx_EC_CP14_64	(0x0C)
#define ESR_ELx_EC_BTI		(0x0D)
#define ESR_ELx_EC_ILL		(0x0E)
/* Unallocated EC: 0x0F - 0x10 */
#define ESR_ELx_EC_SVC32	(0x11)
#define ESR_ELx_EC_HVC32	(0x12)	/* EL2 only */
#define ESR_ELx_EC_SMC32	(0x13)	/* EL2 and above */
/* Unallocated EC: 0x14 */
#define ESR_ELx_EC_SVC64	(0x15)
#define ESR_ELx_EC_HVC64	(0x16)	/* EL2 and above */
#define ESR_ELx_EC_SMC64	(0x17)	/* EL2 and above */
#define ESR_ELx_EC_SYS64	(0x18)
#define ESR_ELx_EC_SVE		(0x19)
#define ESR_ELx_EC_ERET		(0x1a)	/* EL2 only */
/* Unallocated EC: 0x1B */
#define ESR_ELx_EC_FPAC		(0x1C)	/* EL1 and above */
/* Unallocated EC: 0x1D - 0x1E */
#define ESR_ELx_EC_IMP_DEF	(0x1f)	/* EL3 only */
#define ESR_ELx_EC_IABT_LOW	(0x20)
#define ESR_ELx_EC_IABT_CUR	(0x21)
#define ESR_ELx_EC_PC_ALIGN	(0x22)
/* Unallocated EC: 0x23 */
#define ESR_ELx_EC_DABT_LOW	(0x24)
#define ESR_ELx_EC_DABT_CUR	(0x25)
#define ESR_ELx_EC_SP_ALIGN	(0x26)
#define ESR_ELx_EC_MEM_OPT  (0x27)
#define ESR_ELx_EC_FP_EXC32	(0x28)
/* Unallocated EC: 0x29 - 0x2B */
#define ESR_ELx_EC_FP_EXC64	(0x2C)
/* Unallocated EC: 0x2D - 0x2E */
#define ESR_ELx_EC_SERROR	(0x2F)
#define ESR_ELx_EC_BREAKPT_LOW	(0x30)
#define ESR_ELx_EC_BREAKPT_CUR	(0x31)
#define ESR_ELx_EC_SOFTSTP_LOW	(0x32)
#define ESR_ELx_EC_SOFTSTP_CUR	(0x33)
#define ESR_ELx_EC_WATCHPT_LOW	(0x34)
#define ESR_ELx_EC_WATCHPT_CUR	(0x35)
/* Unallocated EC: 0x36 - 0x37 */
#define ESR_ELx_EC_BKPT32	    (0x38)
/* Unallocated EC: 0x39 */
#define ESR_ELx_EC_VECTOR32	    (0x3A)	/* EL2 only */
/* Unallocated EC: 0x3B */
#define ESR_ELx_EC_BRK64	    (0x3C)
/* Unallocated EC: 0x3D - 0x3F */
#define ESR_ELx_EC_MAX		    (0x3F)
// --------------------------------------------------------------
#define UL(x)               x ## UL
// --------------------------------------------------------------
#define ESR_ELx_EC_SHIFT	(26)
#define ESR_ELx_EC_WIDTH	(6)
#define ESR_ELx_EC_MASK		(UL(0x3F) << ESR_ELx_EC_SHIFT)
#define ESR_ELx_EC(esr)		(((esr) & ESR_ELx_EC_MASK) >> ESR_ELx_EC_SHIFT)

#define ESR_ELx_IL_SHIFT	(25)
#define ESR_ELx_IL		    (UL(1) << ESR_ELx_IL_SHIFT)
#define ESR_ELx_ISS_MASK	(ESR_ELx_IL - 1)

#define ESR_ELx_WNR_SHIFT	(6)
#define ESR_ELx_WNR		    (UL(1) << ESR_ELx_WNR_SHIFT)

/* Asynchronous Error Type */
#define ESR_ELx_IDS_SHIFT	(24)
#define ESR_ELx_IDS		    (UL(1) << ESR_ELx_IDS_SHIFT)
#define ESR_ELx_AET_SHIFT	(10)
#define ESR_ELx_AET		    (UL(0x7) << ESR_ELx_AET_SHIFT)

#define ESR_ELx_AET_UC		(UL(0) << ESR_ELx_AET_SHIFT)
#define ESR_ELx_AET_UEU		(UL(1) << ESR_ELx_AET_SHIFT)
#define ESR_ELx_AET_UEO		(UL(2) << ESR_ELx_AET_SHIFT)
#define ESR_ELx_AET_UER		(UL(3) << ESR_ELx_AET_SHIFT)
#define ESR_ELx_AET_CE		(UL(6) << ESR_ELx_AET_SHIFT)

/* Shared ISS field definitions for Data/Instruction aborts */
#define ESR_ELx_SET_SHIFT	(11)
#define ESR_ELx_SET_MASK	(UL(3) << ESR_ELx_SET_SHIFT)
#define ESR_ELx_FnV_SHIFT	(10)
#define ESR_ELx_FnV		    (UL(1) << ESR_ELx_FnV_SHIFT)
#define ESR_ELx_EA_SHIFT	(9)
#define ESR_ELx_EA		    (UL(1) << ESR_ELx_EA_SHIFT)
#define ESR_ELx_S1PTW_SHIFT	(7)
#define ESR_ELx_S1PTW		(UL(1) << ESR_ELx_S1PTW_SHIFT)

/* Shared ISS fault status code(IFSC/DFSC) for Data/Instruction aborts */
#define ESR_ELx_FSC_MASK      (_AC(0x03, U) << 0)
#define ESR_ELx_FSC		      (0x3F)
#define ESR_ELx_FSC_TYPE	  (0x3C)
#define ESR_ELx_FSC_LEVEL	  (0x03)
#define ESR_ELx_FSC_EXTABT	  (0x10)
#define ESR_ELx_FSC_EXTABTx   (0x14)
#define ESR_ELx_FSC_MTE		  (0x11)
#define ESR_ELx_FSC_SERROR	  (0x11)
#define ESR_ELx_FSC_ACCESSx	  (0x08)
#define ESR_ELx_FSC_FAULTx	  (0x04)
#define ESR_ELx_FSC_PERMx	  (0x0C)
#define ESR_ELx_FSC_PARITY    (0x18)
#define ESR_ELx_FSC_PARITYe   (0x1B)
#define ESR_ELx_FSC_PARITYx	  (0x1C)
#define ESR_ELx_FSC_ADDRx     (0x00)
#define ESR_ELx_FSC_ALIGN     (0x21)
#define ESR_ELx_FSC_TLB_CFLCT (0x30)
#define ESR_ELx_FSC_LKD       (0x34)

/* ISS field definitions for Data Aborts */
#define ESR_ELx_ISV_SHIFT	(24)
#define ESR_ELx_ISV		    (UL(1) << ESR_ELx_ISV_SHIFT)
#define ESR_ELx_SAS_SHIFT	(22)
#define ESR_ELx_SAS		    (UL(3) << ESR_ELx_SAS_SHIFT)
#define ESR_ELx_SSE_SHIFT	(21)
#define ESR_ELx_SSE		    (UL(1) << ESR_ELx_SSE_SHIFT)
#define ESR_ELx_SRT_SHIFT	(16)
#define ESR_ELx_SRT_MASK	(UL(0x1F) << ESR_ELx_SRT_SHIFT)
#define ESR_ELx_SF_SHIFT	(15)
#define ESR_ELx_SF		    (UL(1) << ESR_ELx_SF_SHIFT)
#define ESR_ELx_AR_SHIFT	(14)
#define ESR_ELx_AR		    (UL(1) << ESR_ELx_AR_SHIFT)
#define ESR_ELx_CM_SHIFT	(8)
#define ESR_ELx_CM		    (UL(1) << ESR_ELx_CM_SHIFT)
#define ESR_ELx_WNR_SHIFT   (6)
#define ESR_ELx_WNR         (UL(1) << ESR_ELx_WNR_SHIFT)

/* ISS field definitions for exceptions taken in to Hyp */
#define ESR_ELx_CV		        (UL(1) << 24)
#define ESR_ELx_COND_SHIFT	    (20)
#define ESR_ELx_COND_MASK	    (UL(0xF) << ESR_ELx_COND_SHIFT)
#define ESR_ELx_WFx_ISS_TI	    (UL(1) << 0)
#define ESR_ELx_WFx_ISS_WFI	    (UL(0) << 0)
#define ESR_ELx_WFx_ISS_WFE	    (UL(1) << 0)
#define ESR_ELx_xVC_IMM_MASK	((1UL << 16) - 1)
// --------------------------------------------------------------
#endif /* _ASM_GENERIC_ESR_H */
