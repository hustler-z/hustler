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

/* XXX: Holds syndrome information for an exception taken to ELx
 *
 * When the value of ID_AA64MMFR0_EL1.PARange indicates that
 * the implementation does not support a 52 bit PA size, if a
 * translation table lookup uses this register when the Effective
 * value of VTCR_EL2.PS is 0b110 and the value of register bits[5:2]
 * is nonzero, an Address size fault is generated.
 */
union hcpu_esr {
    register_t bits;

    struct {
        /* XXX: Instruction Specific Syndrome
         * Architecturally, this field can be defined independently
         * for each defined Exception class. However, in practice,
         * some ISS encodings are used for more than one Exception class.
         * Typically, an ISS encoding has a number of subfields. When an
         * ISS subfield holds a register number, the value returned in
         * that field is the AArch64 view of the register number.
         */
        unsigned long iss:25;
        unsigned long len:1;   /* Instruction length */
        /* XXX: Exception Class
         * Indicates the reason for the exception that this register
         * holds information about.
         */
        unsigned long ec:6;
    };

    struct {
        unsigned long dfsc:6;  /* Data Fault Status Code (bit[5:0]) */
        unsigned long wnr:1;   /* Write not Read (bit 6) */
         /* For a stage 2 fault, indicates whether the fault was a
          * stage 2 fault on an access made for a stage 1 translation
          * table walk. (bit 7)
          */
        unsigned long s1ptw:1;
        unsigned long cm:1;   /* Cache Maintenance (bit 8) */
        unsigned long ea:1;   /* External Abort Type (bit 9) */
        unsigned long fnv:1;  /* FAR not Valid (bit 10) */
        unsigned long set:2;  /* Synchronous Error Type (bit [12:11]) */
        /* Indicates if the Fault came from use of VNCR_EL2 by EL1 code
         * (bit 13)
         */
        unsigned long vncr:1;
        unsigned long ar:1;   /* Acquire/Release (bit 14) */
        /* Width of the Register accessed by the Instruction is Sixty-Four
         * (bit 15)
         */
        unsigned long sf:1;
        unsigned long srt:5;  /* Syndrome Register Transfer (bit [20:16]) */
        unsigned long sse:1;  /* Syndrome Sign Extend (bit 21) */
        unsigned long sas:2;  /* Syndrome Access Size (bit [23:22]) */
        unsigned long isv:1;  /* Instruction Syndrome Valid (bit 24) */
        unsigned long len:1;
        unsigned long ec:6;
    } dabt; /* Data Abort ISS Encoding */

    struct {
        unsigned long ifsc:6; /* Instruction Fault Status Code (bit [5:0]) */
        unsigned long res0:1;
        unsigned long s1ptw:1;
        unsigned long res1:1;
        unsigned long ea:1;   /* (bit 9) */
        unsigned long fnv:1;  /* (bit 10) */
        unsigned long set:2;  /* (bit [12:11]) */
        unsigned long res2:12;
        unsigned long len:1;
        unsigned long ec:6;
    } iabt; /* Instruction Abort ISS Encoding */

    /* TODO */
};

#define ESR_ELx_EC_UNKNOWN	    (0x00)
#define ESR_ELx_EC_WFx		    (0x01)
/* Unallocated EC: 0x02 */
#define ESR_ELx_EC_CP15_32	    (0x03)
#define ESR_ELx_EC_CP15_64	    (0x04)
#define ESR_ELx_EC_CP14_MR	    (0x05)
#define ESR_ELx_EC_CP14_LS	    (0x06)
#define ESR_ELx_EC_FP_ASIMD	    (0x07)
#define ESR_ELx_EC_CP10_ID	    (0x08)	/* EL2 only */
#define ESR_ELx_EC_PAC		    (0x09)	/* EL2 and above */
/* Unallocated EC: 0x0A - 0x0B */
#define ESR_ELx_EC_CP14_64	    (0x0C)
#define ESR_ELx_EC_BTI		    (0x0D)
#define ESR_ELx_EC_ILL		    (0x0E)
/* Unallocated EC: 0x0F - 0x10 */
#define ESR_ELx_EC_SVC32	    (0x11)
#define ESR_ELx_EC_HVC32	    (0x12)	/* EL2 only */
#define ESR_ELx_EC_SMC32	    (0x13)	/* EL2 and above */
/* Unallocated EC: 0x14 */
#define ESR_ELx_EC_SVC64	    (0x15)
#define ESR_ELx_EC_HVC64	    (0x16)	/* EL2 and above */
#define ESR_ELx_EC_SMC64	    (0x17)	/* EL2 and above */
#define ESR_ELx_EC_SYS64	    (0x18)
#define ESR_ELx_EC_SVE		    (0x19)
#define ESR_ELx_EC_ERET		    (0x1a)	/* EL2 only */
/* Unallocated EC: 0x1B */
#define ESR_ELx_EC_FPAC		    (0x1C)	/* EL1 and above */
/* Unallocated EC: 0x1D - 0x1E */
#define ESR_ELx_EC_IMP_DEF	    (0x1f)	/* EL3 only */
#define ESR_ELx_EC_IABT_LOW	    (0x20)
#define ESR_ELx_EC_IABT_CUR	    (0x21)
#define ESR_ELx_EC_PC_ALIGN	    (0x22)
/* Unallocated EC: 0x23 */
#define ESR_ELx_EC_DABT_LOW	    (0x24)
#define ESR_ELx_EC_DABT_CUR	    (0x25)
#define ESR_ELx_EC_SP_ALIGN	    (0x26)
#define ESR_ELx_EC_MEM_OPT      (0x27)
#define ESR_ELx_EC_FP_EXC32	    (0x28)
/* Unallocated EC: 0x29 - 0x2B */
#define ESR_ELx_EC_FP_EXC64	    (0x2C)
/* Unallocated EC: 0x2D - 0x2E */
#define ESR_ELx_EC_SERROR	    (0x2F)
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
#define UL(x)                   x ## UL
// --------------------------------------------------------------
#define ESR_ELx_EC_SHIFT	    (26)
#define ESR_ELx_EC_WIDTH	    (6)
#define ESR_ELx_EC_MASK		    (UL(0x3F) << ESR_ELx_EC_SHIFT)
#define ESR_ELx_EC(esr)		    (((esr) & ESR_ELx_EC_MASK) >> ESR_ELx_EC_SHIFT)

#define ESR_ELx_IL_SHIFT	    (25)
#define ESR_ELx_IL		        (UL(1) << ESR_ELx_IL_SHIFT)
#define ESR_ELx_ISS_MASK	    (ESR_ELx_IL - 1)

#define ESR_ELx_WNR_SHIFT	    (6)
#define ESR_ELx_WNR		        (UL(1) << ESR_ELx_WNR_SHIFT)

/* Asynchronous Error Type */
#define ESR_ELx_IDS_SHIFT	    (24)
#define ESR_ELx_IDS		        (UL(1) << ESR_ELx_IDS_SHIFT)
#define ESR_ELx_AET_SHIFT	    (10)
#define ESR_ELx_AET		        (UL(0x7) << ESR_ELx_AET_SHIFT)

#define ESR_ELx_AET_UC		    (UL(0) << ESR_ELx_AET_SHIFT)
#define ESR_ELx_AET_UEU		    (UL(1) << ESR_ELx_AET_SHIFT)
#define ESR_ELx_AET_UEO		    (UL(2) << ESR_ELx_AET_SHIFT)
#define ESR_ELx_AET_UER		    (UL(3) << ESR_ELx_AET_SHIFT)
#define ESR_ELx_AET_CE		    (UL(6) << ESR_ELx_AET_SHIFT)

/* Shared ISS field definitions for Data/Instruction aborts */
#define ESR_ELx_SET_SHIFT	    (11)
#define ESR_ELx_SET_MASK	    (UL(3) << ESR_ELx_SET_SHIFT)
#define ESR_ELx_FnV_SHIFT	    (10)
#define ESR_ELx_FnV		        (UL(1) << ESR_ELx_FnV_SHIFT)
#define ESR_ELx_EA_SHIFT	    (9)
#define ESR_ELx_EA		        (UL(1) << ESR_ELx_EA_SHIFT)
#define ESR_ELx_S1PTW_SHIFT	    (7)
#define ESR_ELx_S1PTW		    (UL(1) << ESR_ELx_S1PTW_SHIFT)

/* Shared ISS fault status code(IFSC/DFSC) for Data/Instruction aborts */
#define ESR_ELx_FSC_MASK        (_AC(0x03, U) << 0)
#define ESR_ELx_FSC		        (0x3F)
#define ESR_ELx_FSC_TYPE	    (0x3C)
#define ESR_ELx_FSC_LEVEL	    (0x03)
// Address Size Fault
#define ESR_ELx_FSC_ADDRSx      (0x00)
// Translation Fault
#define ESR_ELx_FSC_FAULTx	    (0x04)
// Access Flag Fault
#define ESR_ELx_FSC_ACCESSx	    (0x08)
// Permission Fault
#define ESR_ELx_FSC_PERMx	    (0x0C)
// Synchronous External Abort, not on Translation Table Walk
#define ESR_ELx_FSC_EXTABT	    (0x10)
// Synchronous Tag Check Fault
#define ESR_ELx_FSC_MTE		    (0x11)
#define ESR_ELx_FSC_SERROR	    (0x11)
// Synchronous External Abort on Translation Table Fault (level -2)
#define ESR_ELx_FSC_EXTABT_N2   (0x12)
// Synchronous External Abort on Translation Table Fault (level -1)
#define ESR_ELx_FSC_EXTABT_N1   (0x13)
// Synchronous External Abort
#define ESR_ELx_FSC_EXTABTx     (0x14)
// Synchronous Parity, not on Translation Table Walk
#define ESR_ELx_FSC_PARITY      (0x18)
// Synchronous Parity, (level -1)
#define ESR_ELx_FSC_PARITY_N1   (0x1B)
// Synchronous Parity
#define ESR_ELx_FSC_PARITYx	    (0x1C)
// Alignment Fault
#define ESR_ELx_FSC_ALIGN       (0x21)
// Granule Protection Fault
#define ESR_ELx_FSC_PROT_N2     (0x22)
#define ESR_ELx_FSC_PROT_N1     (0x23)
// Granule Protection Fault
#define ESR_ELx_FSC_PROTx       (0x24)
// Granule Protection Fault, not on Translation Table Walk
#define ESR_ELx_FSC_PROT        (0x28)
// Address Size Fault, (level -1)
#define ESR_ELx_FSC_ADDR_N1     (0x29)
// Translation Fault (level -2)
#define ESR_ELx_FSC_TRAN_N2     (0x2A)
// Translation Fault (level -2)
#define ESR_ELx_FSC_TRAN_N1     (0x2B)
// Address Size Fault, (level -2)
#define ESR_ELx_FSC_ADDR_N2     (0x2C)
// TLB Conflict Abort
#define ESR_ELx_FSC_TLB_CFLCT   (0x30)
// Unsupported Atomic Hardware Update Fault
#define ESR_ELx_FSC_NO_AHUF     (0x31)
// Lockdown (Implementation Defined)
#define ESR_ELx_FSC_LKD         (0x34)
// Unsupported Exclusive or Atomic Access (Implementation Defined)
#define ESR_ELx_FSC_NO_EAA      (0x35)

/* ISS field definitions for Data Aborts */
#define ESR_ELx_ISV_SHIFT	    (24)
#define ESR_ELx_ISV		        (UL(1) << ESR_ELx_ISV_SHIFT)
#define ESR_ELx_SAS_SHIFT	    (22)
#define ESR_ELx_SAS		        (UL(3) << ESR_ELx_SAS_SHIFT)
#define ESR_ELx_SSE_SHIFT	    (21)
#define ESR_ELx_SSE		        (UL(1) << ESR_ELx_SSE_SHIFT)
#define ESR_ELx_SRT_SHIFT	    (16)
#define ESR_ELx_SRT_MASK	    (UL(0x1F) << ESR_ELx_SRT_SHIFT)
#define ESR_ELx_SF_SHIFT	    (15)
#define ESR_ELx_SF		        (UL(1) << ESR_ELx_SF_SHIFT)
#define ESR_ELx_AR_SHIFT	    (14)
#define ESR_ELx_AR		        (UL(1) << ESR_ELx_AR_SHIFT)
#define ESR_ELx_CM_SHIFT	    (8)
#define ESR_ELx_CM		        (UL(1) << ESR_ELx_CM_SHIFT)
#define ESR_ELx_WNR_SHIFT       (6)
#define ESR_ELx_WNR             (UL(1) << ESR_ELx_WNR_SHIFT)

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
