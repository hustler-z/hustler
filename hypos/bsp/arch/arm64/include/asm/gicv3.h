/**
 * Hustler's Project
 *
 * File:  gicv3.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_GICV3_H
#define _ARCH_GICV3_H
// --------------------------------------------------------------

#ifndef __ASSEMBLY__

/* CPU Interfaces
 * -----------------------------------------------------------------------
 * Physical CPU Interface | Virtualization Control | Virtual CPU Interface
 *        ICC_*_ELx       |      ICH_*_EL2         |       ICV_*_EL1
 * -----------------------------------------------------------------------
 *            |IRQ               |FIQ                  |vIRQ      |vFIQ
 *            ▼                  ▼                     ▼          ▼
 * +----------------------------------------------+ +--------------------+
 * |                Physical PE                   | |     Virtual PE     |
 * +----------------------------------------------+ +--------------------+
 *
 * The hypervisor executing at EL2 uses the regular ICC_*_ELx registers to
 * handle physical interrupts.
 *
 * The hypervisor has access to additional registers to control the
 * virtualization features via ICH_*_EL2:
 * (a) Enabling and disabling the virtual CPU interface.
 * (b) Accessing virtual register state to enable context switching.
 * (c) Configuring maintenance interrupts.
 * (d) Controlling virtual interrupts for the currently scheduled vPE.
 *
 * Software executing in a virtualized environment uses the ICV_*_EL1
 * registers to handle virtual interrupts.
 * -----------------------------------------------------------------------
 */

#define ENABLE           (0x01)
#define DISABLE          (0x00)

#define ICC_EL1(index)   ICC_ ## index ## _EL1
#define ICC_EL2(index)   ICC_ ## index ## _EL2
#define ICC_EL3(index)   ICC_ ## index ## _EL3

#define ICH_EL2(index)   ICH_ ## index ## _EL2

#define ICV_EL1(index)   ICV_ ## index ## _EL1

#define ICC_ELx
#ifdef  ICC_ELx
void set_icc_sre_el1(unsigned int value);
unsigned int get_icc_sre_el1(void);
void set_icc_sre_el2(unsigned int value);
unsigned int get_icc_sre_el2(void);
void set_icc_sre_el3(unsigned int value);
unsigned int get_icc_sre_el3(void);
void enable_irq_grp0_el1(void);
void disable_irq_grp0_el1(void);
void enable_irq_grp1_el1(void);
void disable_irq_grp1_el1(void);
void enable_irq_grp1_el3(void);
void disable_irq_grp1_el3(void);
unsigned int get_icc_ctlr_el1(void);
void set_icc_ctlr_el1(unsigned int value);
unsigned int get_icc_ctlr_el3(void);
void set_icc_ctlr_el3(unsigned int value);
unsigned int get_iar0_el1(void);
void set_eoir0_el1(unsigned int value);
void set_dir_el1(unsigned int value);
unsigned int get_iar1_el1(void);
void set_eoir1_el1(unsigned int value);
void set_priority_mask(unsigned int value);
unsigned int get_bpr0_el1(void);
void set_bpr0_el1(unsigned int value);
unsigned int get_bpr1_el1(void);
void set_bpr1_el1(unsigned int value);
unsigned int get_running_priority(void);
void send_sgi_grp0(unsigned int id, unsigned int mode,
        unsigned int targets);
void send_sgi_grp1(unsigned int id, unsigned int mode,
        unsigned int targets);
void send_alias_sgi_grp1(unsigned int id, unsigned int mode,
        unsigned int targets);
#endif /* ICC_ELx */

#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------

#endif /* _ARCH_GICV3_H */
