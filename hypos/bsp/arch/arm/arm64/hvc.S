/**
 * Hustler's Project
 *
 * File:  hvc.S
 * Date:  2024/07/12
 * Usage:
 *
 * XXX: Hypervisor Call causes an exception to EL2. Software
 *      executing at EL1 can use this instruction to call
 *      hypervisor to request a service.
 *
 * The HVC instruction is UNDEFINED:
 * (a) When EL3 is implemented and SCR_EL3.HCE is set to 0.
 * (b) When EL3 is not implemented and HCR_EL2.HCD is set to 1.
 * (c) When EL2 is not implemented.
 * (d) At EL1 if EL2 is not enabled in the current Security state.
 * (e) At EL0.
 *
 * On executing an HVC instruction, the PE records the exception
 * as a Hypervisor Call exception in the immediate argument.
 */

#include <asm/define.h>
#include <asm/offset.h>
// -------------------------------------------------------------













// -------------------------------------------------------------
