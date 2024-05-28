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

/* :lo12: A special notation is used to
 *        add only the lowest 12 bits of
 *        the label to the adrp address.
 *        4KB page is addressable with
 *        12 bits.
 */
.macro  adr_l, dst, sym
    adrp \dst, \sym
    add  \dst, \dst, :lo12:\sym
.endm

// ------------------------------------------------------------------------
.macro altinstruction_entry orig_offset repl_offset feature orig_len repl_len
    .word  \orig_offset - .
    .word  \repl_offset - .
    .hword \feature
    .byte  \orig_len
    .byte  \repl_len
.endm

.macro alternative_insn insn1, insn2, cap, enable = 1
    .if \enable
661:
    \insn1
662:
    .pushsection .altinstructions, "a"
    altinstruction_entry 661b, 663f, \cap, 662b-661b, 664f-663f
    .popsection
    .pushsection .altinstr_replacement, "ax"
663:
    \insn2
664:
    .popsection
    .org    . - (664b-663b) + (662b-661b)
    .org    . - (662b-661b) + (664b-663b)
    .endif
.endm

#define _ALTERNATIVE_CFG(insn1, insn2, cap, cfg, ...)   \
    alternative_insn insn1, insn2, cap, cfg
// ------------------------------------------------------------------------
#endif /* __ASSEMBLY__ */

#define ALTERNATIVE(oldinstr, newinstr, ...)   \
    _ALTERNATIVE_CFG(oldinstr, newinstr, __VA_ARGS__, 1)

// ------------------------------------------------------------------------
#endif /* _ARCH_DEFINE_H */
