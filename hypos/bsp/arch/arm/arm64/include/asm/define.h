/**
 * Hustler's Project
 *
 * File:  define.h
 * Date:  2024/05/20
 * Usage: common macros for assembly codes
 */

#ifndef _ASM_DEFINE_H
#define _ASM_DEFINE_H

#include <org/bitops.h>

// --------------------------------------------------------------
#define BAD_SYNC              0
#define BAD_IRQ               1
#define BAD_FIQ               2
#define BAD_ERROR             3
#define GOOD_TRAP             4

#define __H                   1
#define __G                   0

/* Exception Level: CurrentEL [3:2] */
#define ARM64_EL0T            0x00000000
#define ARM64_EL1T            0x00000004
#define ARM64_EL1H            0x00000005
#define ARM64_EL2T            0x00000008
#define ARM64_EL2H            0x00000009
#define ARM64_EL3T            0x0000000C
#define ARM64_EL3H            0x0000000D

#define DAIF_DEBUG_MASK       0x8
#define DAIF_ERROR_MASK       0x4
#define DAIF_IRQ_MASK         0x2
#define DAIF_FIQ_MASK         0x1

#define __OP32 "w"
#define ASM_REG(index)        asm("x" # index)
// --------------------------------------------------------------

#ifdef __ASSEMBLY__

#ifndef CODE_FILL
#define CODE_FILL           ~0
#endif

#ifndef DATA_ALIGN
#define DATA_ALIGN          0
#endif

#ifndef DATA_FILL
#define DATA_FILL           ~0
#endif

#define SYM_ALIGN(align...) .balign align

#define SYM_L_GLOBAL(name) .globl name; .hidden name
#define SYM_L_WEAK(name)   .weak name
#define SYM_L_LOCAL(name)  /* nothing */

#define SYM_T_FUNC         STT_FUNC
#define SYM_T_DATA         STT_OBJECT
#define SYM_T_NONE         STT_NOTYPE

#define SYM(name, typ, linkage, align...)     \
    .type name, SYM_T_ ## typ;                \
    SYM_L_ ## linkage(name);                  \
    SYM_ALIGN(align);                         \
    name:

#define count_args_(dot, a1, a2, a3, a4, a5, a6, a7, a8, x, ...) x
#define count_args(args...) \
    count_args_(., ## args, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define ARG1_(x, y...) (x)
#define ARG2_(x, y...) ARG1_(y)
#define ARG3_(x, y...) ARG2_(y)
#define ARG4_(x, y...) ARG3_(y)

#define ARG__(nr) ARG ## nr ## _
#define ARG_(nr)  ARG__(nr)
#define LASTARG(x, y...) ARG_(count_args(x, ## y))(x, ## y)

#define ARM_FUNCTION_ALIGNMENT  (4)
#define count_args_exp(args...) count_args(args)
#if count_args_exp(CODE_FILL)
#define DO_CODE_ALIGN(align...) LASTARG(ARM_FUNCTION_ALIGNMENT, ## align), \
                                CODE_FILL
#else
#define DO_CODE_ALIGN(align...) LASTARG(ARM_FUNCTION_ALIGNMENT, ## align)
#endif

#define GLOBAL(name)         \
    .globl name;             \
    name:

#define ENTRY(name, align)   \
    .globl name;             \
    .balign align;           \
    name:

#define FUNC(name, align...) \
        SYM(name, FUNC, GLOBAL, DO_CODE_ALIGN(align))
#define LABEL(name, align...) \
        SYM(name, NONE, GLOBAL, DO_CODE_ALIGN(align))
#define DATA(name, align...) \
        SYM(name, DATA, GLOBAL, LASTARG(DATA_ALIGN, ## align), DATA_FILL)

#define FUNC_LOCAL(name, align...) \
        SYM(name, FUNC, LOCAL, DO_CODE_ALIGN(align))
#define LABEL_LOCAL(name, align...) \
        SYM(name, NONE, LOCAL, DO_CODE_ALIGN(align))
#define DATA_LOCAL(name, align...) \
        SYM(name, DATA, LOCAL, LASTARG(DATA_ALIGN, ## align), DATA_FILL)

#define END(name)                \
    .size name, . - name

// --------------------------------------------------------------
#define __HEAD   .section .boot.head, "ax", %progbits
#define __ENTRY  .section .boot.entry, "ax", %progbits
#define __TTBL   .section .boot.ttbl, "ax", %progbits
// --------------------------------------------------------------

/* :lo12: A special notation is used to
 *        add only the lowest 12 bits of
 *        the label to the adrp address.
 *        4KB page is addressable with
 *        12 bits.
 * adrp - Form PC-relative address to 4KB
 *        page.
 *
 * Load address of <sym> into <reg>, <sym> being in the range
 * +/- 4GB of the PC.
 */
.macro  adr_l, dst, sym
    adrp \dst, \sym
    add  \dst, \dst, :lo12:\sym
.endm

/* Stack pushing/popping register pairs
 */
.macro push, xn, xm
    stp \xn, \xm, [sp, #-16]!
.endm

.macro pop,  xn, xm
    ldp \xn, \xm, [sp], #16
.endm

// --------------------------------------------------------------

#endif /* __ASSEMBLY__ */

#endif /* _ASM_DEFINE_H */
