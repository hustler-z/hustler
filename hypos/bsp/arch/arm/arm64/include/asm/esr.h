/**
 * Hustler's Project
 *
 * File:  esr.h
 * Date:  2024/07/11
 * Usage:
 */

#ifndef _ASM_ESR_H
#define _ASM_ESR_H
// --------------------------------------------------------------

/* AArch 64 System Register Encodings */
#define __ESR_SYSREG_c0  0
#define __ESR_SYSREG_c1  1
#define __ESR_SYSREG_c2  2
#define __ESR_SYSREG_c3  3
#define __ESR_SYSREG_c4  4
#define __ESR_SYSREG_c5  5
#define __ESR_SYSREG_c6  6
#define __ESR_SYSREG_c7  7
#define __ESR_SYSREG_c8  8
#define __ESR_SYSREG_c9  9
#define __ESR_SYSREG_c10 10
#define __ESR_SYSREG_c11 11
#define __ESR_SYSREG_c12 12
#define __ESR_SYSREG_c13 13
#define __ESR_SYSREG_c14 14
#define __ESR_SYSREG_c15 15

#define __ESR_SYSREG_0   0
#define __ESR_SYSREG_1   1
#define __ESR_SYSREG_2   2
#define __ESR_SYSREG_3   3
#define __ESR_SYSREG_4   4
#define __ESR_SYSREG_5   5
#define __ESR_SYSREG_6   6
#define __ESR_SYSREG_7   7

/* These are used to decode traps with ESR.EC==ESR_EC_SYSREG */
#define ESR_SYSREG(op0,op1,crn,crm,op2) \
    (((__ESR_SYSREG_##op0) << ESR_SYSREG_OP0_SHIFT) | \
     ((__ESR_SYSREG_##op1) << ESR_SYSREG_OP1_SHIFT) | \
     ((__ESR_SYSREG_##crn) << ESR_SYSREG_CRN_SHIFT) | \
     ((__ESR_SYSREG_##crm) << ESR_SYSREG_CRM_SHIFT) | \
     ((__ESR_SYSREG_##op2) << ESR_SYSREG_OP2_SHIFT))

#define ESR_SYSREG_DCISW             ESR_SYSREG(1,0,c7,c6,2)
#define ESR_SYSREG_DCCSW             ESR_SYSREG(1,0,c7,c10,2)
#define ESR_SYSREG_DCCISW            ESR_SYSREG(1,0,c7,c14,2)

#define ESR_SYSREG_MDSCR_EL1         ESR_SYSREG(2,0,c0,c2,2)
#define ESR_SYSREG_MDRAR_EL1         ESR_SYSREG(2,0,c1,c0,0)
#define ESR_SYSREG_OSLAR_EL1         ESR_SYSREG(2,0,c1,c0,4)
#define ESR_SYSREG_OSLSR_EL1         ESR_SYSREG(2,0,c1,c1,4)
#define ESR_SYSREG_OSDLR_EL1         ESR_SYSREG(2,0,c1,c3,4)
#define ESR_SYSREG_DBGPRCR_EL1       ESR_SYSREG(2,0,c1,c4,4)
#define ESR_SYSREG_MDCCSR_EL0        ESR_SYSREG(2,3,c0,c1,0)
#define ESR_SYSREG_DBGDTR_EL0        ESR_SYSREG(2,3,c0,c4,0)
#define ESR_SYSREG_DBGDTRTX_EL0      ESR_SYSREG(2,3,c0,c5,0)
#define ESR_SYSREG_DBGDTRRX_EL0      ESR_SYSREG(2,3,c0,c5,0)

#define ESR_SYSREG_DBGBVRn_EL1(n)    ESR_SYSREG(2,0,c0,c##n,4)
#define ESR_SYSREG_DBGBCRn_EL1(n)    ESR_SYSREG(2,0,c0,c##n,5)
#define ESR_SYSREG_DBGWVRn_EL1(n)    ESR_SYSREG(2,0,c0,c##n,6)
#define ESR_SYSREG_DBGWCRn_EL1(n)    ESR_SYSREG(2,0,c0,c##n,7)

#define ESR_SYSREG_DBG_CASES(REG) case ESR_SYSREG_##REG##n_EL1(0):  \
                                  case ESR_SYSREG_##REG##n_EL1(1):  \
                                  case ESR_SYSREG_##REG##n_EL1(2):  \
                                  case ESR_SYSREG_##REG##n_EL1(3):  \
                                  case ESR_SYSREG_##REG##n_EL1(4):  \
                                  case ESR_SYSREG_##REG##n_EL1(5):  \
                                  case ESR_SYSREG_##REG##n_EL1(6):  \
                                  case ESR_SYSREG_##REG##n_EL1(7):  \
                                  case ESR_SYSREG_##REG##n_EL1(8):  \
                                  case ESR_SYSREG_##REG##n_EL1(9):  \
                                  case ESR_SYSREG_##REG##n_EL1(10): \
                                  case ESR_SYSREG_##REG##n_EL1(11): \
                                  case ESR_SYSREG_##REG##n_EL1(12): \
                                  case ESR_SYSREG_##REG##n_EL1(13): \
                                  case ESR_SYSREG_##REG##n_EL1(14): \
                                  case ESR_SYSREG_##REG##n_EL1(15)

#define ESR_SYSREG_SCTLR_EL1         ESR_SYSREG(3,0,c1, c0,0)
#define ESR_SYSREG_ACTLR_EL1         ESR_SYSREG(3,0,c1, c0,1)
#define ESR_SYSREG_TTBR0_EL1         ESR_SYSREG(3,0,c2, c0,0)
#define ESR_SYSREG_TTBR1_EL1         ESR_SYSREG(3,0,c2, c0,1)
#define ESR_SYSREG_TCR_EL1           ESR_SYSREG(3,0,c2, c0,2)
#define ESR_SYSREG_AFSR0_EL1         ESR_SYSREG(3,0,c5, c1,0)
#define ESR_SYSREG_AFSR1_EL1         ESR_SYSREG(3,0,c5, c1,1)
#define ESR_SYSREG_ESR_EL1           ESR_SYSREG(3,0,c5, c2,0)
#define ESR_SYSREG_FAR_EL1           ESR_SYSREG(3,0,c6, c0,0)
#define ESR_SYSREG_PMINTENSET_EL1    ESR_SYSREG(3,0,c9,c14,1)
#define ESR_SYSREG_PMINTENCLR_EL1    ESR_SYSREG(3,0,c9,c14,2)
#define ESR_SYSREG_MAIR_EL1          ESR_SYSREG(3,0,c10,c2,0)
#define ESR_SYSREG_AMAIR_EL1         ESR_SYSREG(3,0,c10,c3,0)
#define ESR_SYSREG_ICC_SGI1R_EL1     ESR_SYSREG(3,0,c12,c11,5)
#define ESR_SYSREG_ICC_ASGI1R_EL1    ESR_SYSREG(3,1,c12,c11,6)
#define ESR_SYSREG_ICC_SGI0R_EL1     ESR_SYSREG(3,2,c12,c11,7)
#define ESR_SYSREG_ICC_SRE_EL1       ESR_SYSREG(3,0,c12,c12,5)
#define ESR_SYSREG_CONTEXTIDR_EL1    ESR_SYSREG(3,0,c13,c0,1)

#define ESR_SYSREG_PMCR_EL0          ESR_SYSREG(3,3,c9,c12,0)
#define ESR_SYSREG_PMCNTENSET_EL0    ESR_SYSREG(3,3,c9,c12,1)
#define ESR_SYSREG_PMCNTENCLR_EL0    ESR_SYSREG(3,3,c9,c12,2)
#define ESR_SYSREG_PMOVSCLR_EL0      ESR_SYSREG(3,3,c9,c12,3)
#define ESR_SYSREG_PMSWINC_EL0       ESR_SYSREG(3,3,c9,c12,4)
#define ESR_SYSREG_PMSELR_EL0        ESR_SYSREG(3,3,c9,c12,5)
#define ESR_SYSREG_PMCEID0_EL0       ESR_SYSREG(3,3,c9,c12,6)
#define ESR_SYSREG_PMCEID1_EL0       ESR_SYSREG(3,3,c9,c12,7)

#define ESR_SYSREG_PMCCNTR_EL0       ESR_SYSREG(3,3,c9,c13,0)
#define ESR_SYSREG_PMXEVTYPER_EL0    ESR_SYSREG(3,3,c9,c13,1)
#define ESR_SYSREG_PMXEVCNTR_EL0     ESR_SYSREG(3,3,c9,c13,2)

#define ESR_SYSREG_PMUSERENR_EL0     ESR_SYSREG(3,3,c9,c14,0)
#define ESR_SYSREG_PMOVSSET_EL0      ESR_SYSREG(3,3,c9,c14,3)

#define ESR_SYSREG_CNTPCT_EL0        ESR_SYSREG(3,3,c14,c0,0)
#define ESR_SYSREG_CNTP_TVAL_EL0     ESR_SYSREG(3,3,c14,c2,0)
#define ESR_SYSREG_CNTP_CTL_EL0      ESR_SYSREG(3,3,c14,c2,1)
#define ESR_SYSREG_CNTP_CVAL_EL0     ESR_SYSREG(3,3,c14,c2,2)

/* Those registers are used when HCR_EL2.TID3 is set */
#define ESR_SYSREG_ID_PFR0_EL1       ESR_SYSREG(3,0,c0,c1,0)
#define ESR_SYSREG_ID_PFR1_EL1       ESR_SYSREG(3,0,c0,c1,1)
#define ESR_SYSREG_ID_PFR2_EL1       ESR_SYSREG(3,0,c0,c3,4)
#define ESR_SYSREG_ID_DFR0_EL1       ESR_SYSREG(3,0,c0,c1,2)
#define ESR_SYSREG_ID_DFR1_EL1       ESR_SYSREG(3,0,c0,c3,5)
#define ESR_SYSREG_ID_AFR0_EL1       ESR_SYSREG(3,0,c0,c1,3)
#define ESR_SYSREG_ID_MMFR0_EL1      ESR_SYSREG(3,0,c0,c1,4)
#define ESR_SYSREG_ID_MMFR1_EL1      ESR_SYSREG(3,0,c0,c1,5)
#define ESR_SYSREG_ID_MMFR2_EL1      ESR_SYSREG(3,0,c0,c1,6)
#define ESR_SYSREG_ID_MMFR3_EL1      ESR_SYSREG(3,0,c0,c1,7)
#define ESR_SYSREG_ID_MMFR4_EL1      ESR_SYSREG(3,0,c0,c2,6)
#define ESR_SYSREG_ID_MMFR5_EL1      ESR_SYSREG(3,0,c0,c3,6)
#define ESR_SYSREG_ID_ISAR0_EL1      ESR_SYSREG(3,0,c0,c2,0)
#define ESR_SYSREG_ID_ISAR1_EL1      ESR_SYSREG(3,0,c0,c2,1)
#define ESR_SYSREG_ID_ISAR2_EL1      ESR_SYSREG(3,0,c0,c2,2)
#define ESR_SYSREG_ID_ISAR3_EL1      ESR_SYSREG(3,0,c0,c2,3)
#define ESR_SYSREG_ID_ISAR4_EL1      ESR_SYSREG(3,0,c0,c2,4)
#define ESR_SYSREG_ID_ISAR5_EL1      ESR_SYSREG(3,0,c0,c2,5)
#define ESR_SYSREG_ID_ISAR6_EL1      ESR_SYSREG(3,0,c0,c2,7)
#define ESR_SYSREG_MVFR0_EL1         ESR_SYSREG(3,0,c0,c3,0)
#define ESR_SYSREG_MVFR1_EL1         ESR_SYSREG(3,0,c0,c3,1)
#define ESR_SYSREG_MVFR2_EL1         ESR_SYSREG(3,0,c0,c3,2)

#define ESR_SYSREG_ID_AA64PFR0_EL1   ESR_SYSREG(3,0,c0,c4,0)
#define ESR_SYSREG_ID_AA64PFR1_EL1   ESR_SYSREG(3,0,c0,c4,1)
#define ESR_SYSREG_ID_AA64DFR0_EL1   ESR_SYSREG(3,0,c0,c5,0)
#define ESR_SYSREG_ID_AA64DFR1_EL1   ESR_SYSREG(3,0,c0,c5,1)
#define ESR_SYSREG_ID_AA64ISAR0_EL1  ESR_SYSREG(3,0,c0,c6,0)
#define ESR_SYSREG_ID_AA64ISAR1_EL1  ESR_SYSREG(3,0,c0,c6,1)
#define ESR_SYSREG_ID_AA64MMFR0_EL1  ESR_SYSREG(3,0,c0,c7,0)
#define ESR_SYSREG_ID_AA64MMFR1_EL1  ESR_SYSREG(3,0,c0,c7,1)
#define ESR_SYSREG_ID_AA64MMFR2_EL1  ESR_SYSREG(3,0,c0,c7,2)
#define ESR_SYSREG_ID_AA64AFR0_EL1   ESR_SYSREG(3,0,c0,c5,4)
#define ESR_SYSREG_ID_AA64AFR1_EL1   ESR_SYSREG(3,0,c0,c5,5)
#define ESR_SYSREG_ID_AA64ZFR0_EL1   ESR_SYSREG(3,0,c0,c4,4)

// --------------------------------------------------------------
#endif /* _ASM_ESR_H */
