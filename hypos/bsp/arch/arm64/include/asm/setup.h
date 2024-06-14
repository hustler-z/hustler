/**
 * Hustler's Project
 *
 * File:  setup.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_SETUP_H
#define _ARCH_SETUP_H
// --------------------------------------------------------------
#ifndef __ASSEMBLY__

#include <asm/hypregs.h>

struct arch_stack {
    unsigned char *stack;
    unsigned int  cpuid;
};

int arch_setup(void);

#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#endif /* _ARCH_SETUP_H */
