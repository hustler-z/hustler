/**
 * Hustler's Project
 *
 * File:  gicv3.c
 * Date:  2024/05/20
 * Usage:
 */

#include <asm/gicv3.h>
#include <asm/sysregs.h>
#include <asm/barrier.h>
#include <lib/strops.h>
#include <generic/type.h>

// --------------------------------------------------------------
inline void gic_arch_enable_irqs(void)
{
    asm volatile ("msr DAIFClr, #2" : : : "memory");
}

inline void gic_arch_disable_irqs(void)
{
    asm volatile ("msr DAIFSet, #2" : : : "memory");
}

#ifdef  ICC_ELx
void set_icc_sre_el1(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL1(SRE));
    isb();
}

unsigned int get_icc_sre_el1(void)
{
    return READ_SYSREG(ICC_EL1(SRE));
}

void set_icc_sre_el2(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL2(SRE));
    isb();
}

unsigned int get_icc_sre_el2(void)
{
    return READ_SYSREG(ICC_EL2(SRE));
}

void set_icc_sre_el3(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL3(SRE));
    isb();
}

unsigned int get_icc_sre_el3(void)
{
    return READ_SYSREG(ICC_EL3(SRE));
}

/* Interrupt Controller Interrupt Group 0 Enable Register */
void enable_irq_grp0_el1(void)
{
    WRITE_SYSREG(ENABLE, ICC_EL1(IGRPEN0));
    isb();
}

void disable_irq_grp0_el1(void)
{
    WRITE_SYSREG(DISABLE, ICC_EL1(IGRPEN0));
    isb();
}

void enable_irq_grp1_el1(void)
{
    unsigned int igen = READ_SYSREG(ICC_EL1(IGRPEN1));
    igen |= ENABLE;
    WRITE_SYSREG(igen, ICC_EL1(IGRPEN1));
    isb();
}

void disable_irq_grp1_el1(void)
{
    unsigned int igen = READ_SYSREG(ICC_EL1(IGRPEN1));
    igen &= ~ENABLE;
    WRITE_SYSREG(igen, ICC_EL1(IGRPEN1));
    isb();
}

void enable_irq_grp1_el3(void)
{
    unsigned int igen = READ_SYSREG(ICC_EL3(IGRPEN1));
    igen |= ENABLE;
    WRITE_SYSREG(igen, ICC_EL3(IGRPEN1));
    isb();
}

void disable_irq_grp1_el3(void)
{
    unsigned int igen = READ_SYSREG(ICC_EL3(IGRPEN1));
    igen &= ~ENABLE;
    WRITE_SYSREG(igen, ICC_EL3(IGRPEN1));
    isb();
}

unsigned int get_icc_ctlr_el1(void)
{
    return READ_SYSREG(ICC_EL1(CTLR));
}

void set_icc_ctlr_el1(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL1(CTLR));
    isb();
}

unsigned int get_icc_ctlr_el3(void)
{
    return READ_SYSREG(ICC_EL3(CTLR));
}

void set_icc_ctlr_el3(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL3(CTLR));
    isb();
}

unsigned int get_iar0_el1(void)
{
    return READ_SYSREG(ICC_EL1(IAR0));
}

void set_eoir0_el1(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL1(EOIR0));
    isb();
}

void set_dir_el1(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL1(DIR));
    isb();
}

unsigned int get_iar1_el1(void)
{
    return READ_SYSREG(ICC_EL1(IAR1));
}

void set_eoir1_el1(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL1(EOIR1));
    isb();
}

void set_priority_mask(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL1(PMR));
}

unsigned int get_bpr0_el1(void)
{
    return READ_SYSREG(ICC_EL1(BPR0));
}

void set_bpr0_el1(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL1(BPR0));
    isb();
}

unsigned int get_bpr1_el1(void)
{
    return READ_SYSREG(ICC_EL1(BPR1));
}

void set_bpr1_el1(unsigned int value)
{
    WRITE_SYSREG(value, ICC_EL1(BPR1));
    isb();
}

unsigned int get_running_priority(void)
{
    return READ_SYSREG(ICC_EL1(RPR));
}

void send_sgi_grp0(unsigned int id,
        unsigned int mode,
        unsigned int targets)
{
    id |= mode;
    id |= targets;
    WRITE_SYSREG(id, ICC_EL1(SGI0R));
}

void send_sgi_grp1(unsigned int id,
        unsigned int mode,
        unsigned int targets)
{
    id |= mode;
    id |= targets;
    WRITE_SYSREG(id, ICC_EL1(SGI1R));
}

void send_alias_sgi_grp1(unsigned int id,
        unsigned int mode,
        unsigned int targets)
{
    id |= mode;
    id |= targets;
    WRITE_SYSREG(id, ICC_EL1(ASGI1R));
}
#endif /* ICC_ELx */
// --------------------------------------------------------------
