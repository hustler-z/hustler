/**
 * Hustler's Project
 *
 * File:  global.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _ASM_GLOBAL_H
#define _ASM_GLOBAL_H
// --------------------------------------------------------------
#ifndef __ASSEMBLY__
#include <bsp/type.h>

struct arch_globl {
    paddr_t boot_ttbr;
    paddr_t exec_ttbr;

    /* page allocation related */

};

#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#endif /* _ASM_GLOBAL_H */
