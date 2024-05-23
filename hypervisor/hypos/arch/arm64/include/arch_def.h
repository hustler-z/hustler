/**
 * Hustler's Project
 *
 * File:  arch_def.h
 * Date:  2024/05/20
 * Usage: common macros for assembly codes
 */

#ifndef _ARCH_DEFINE_H
#define _ARCH_DEFINE_H

#include <arch_bitops.h>

// ------------------------------------------------------------------------

/* Exception Level: CurrentEL [3:2] */
#define ARCH_ARM64_EL0T            0x00000000
#define ARCH_ARM64_EL1T            0x00000004
#define ARCH_ARM64_EL1H            0x00000005
#define ARCH_ARM64_EL2T            0x00000008
#define ARCH_ARM64_EL2H            0x00000009
#define ARCH_ARM64_EL3T            0x0000000C
#define ARCH_ARM64_EL3H            0x0000000D

// ------------------------------------------------------------------------

#ifdef __ASSEMBLY__

#define ALIGN .align 4,0x90

#define GLOBL_OBJ(name, a)       \
    .align a;                    \
    .type  name, %object;        \
    .globl name;                 \
    name:

#define GLOBAL(name)             \
    .globl name;                 \
    name:

#define ENTRY(name)              \
    .globl name;                 \
    ALIGN;                       \
    name:

#define END(name)                \
    .type name, %function;       \
    .size name, . - name

.macro  adr_l, dst, sym
    adrp \dst, \sym
    add  \dst, \dst, :lo12:\sym
.endm

#endif /* __ASSEMBLY__ */

// ------------------------------------------------------------------------
#endif /* _ARCH_DEFINE_H */
