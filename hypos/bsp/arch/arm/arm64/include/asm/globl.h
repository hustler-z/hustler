/**
 * Hustler's Project
 *
 * File:  global.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _ARCH_GLOBAL_H
#define _ARCH_GLOBAL_H
// --------------------------------------------------------------
#ifndef __ASSEMBLY__
#include <common/type.h>

struct arch_globl {
    paddr_t boot_ttbr;
    paddr_t exec_ttbr;

    /* page allocation related */

};

#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#endif /* _ARCH_GLOBAL_H */
