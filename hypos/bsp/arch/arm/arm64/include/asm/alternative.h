/**
 * Hustler's Project
 *
 * File:  alternative.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ASM_ALTERNATIVE_H
#define _ASM_ALTERNATIVE_H

// ------------------------------------------------------------------------
#ifndef __ASSEMBLY__
#include <bsp/stringify.h>

#define ALTINSTR_ENTRY(feature, cb)                               \
    " .word 661b - .\n"             /* label */                   \
    " .if " __stringify(cb) " == 0\n"                             \
    " .word 663f - .\n"             /* new instruction */         \
    " .else\n"                                                    \
    " .word " __stringify(cb) "- .\n"       /* callback */        \
    " .endif\n"                                                   \
    " .hword " __stringify(feature) "\n"        /* feature bit */ \
    " .byte 662b-661b\n"                /* source len */          \
    " .byte 664f-663f\n"                /* replacement len */

#define __ALTERNATIVE_CFG(oldinstr, newinstr, feature, cfg_enabled, cb) \
    ".if "__stringify(cfg_enabled)" == 1\n"                             \
    "661:\n\t"                                                          \
    oldinstr "\n"                                                       \
    "662:\n"                                                            \
    ".pushsection .altinstructions,\"a\"\n"                             \
    ALTINSTR_ENTRY(feature,cb)                                          \
    ".popsection\n"                                                     \
    " .if " __stringify(cb) " == 0\n"                                   \
    ".pushsection .altinstr_replacement, \"ax\"\n"                      \
    "663:\n\t"                                                          \
    newinstr "\n"                                                       \
    "664:\n\t"                                                          \
    ".popsection\n\t"                                                   \
    ".org   . - (664b-663b) + (662b-661b)\n\t"                          \
    ".org   . - (662b-661b) + (664b-663b)\n"                            \
    ".else\n\t"                                                         \
    "663:\n\t"                                                          \
    "664:\n\t"                                                          \
    ".endif\n"                                                          \
    ".endif\n"

#define _ALTERNATIVE_CFG(oldinstr, newinstr, feature, cfg, ...) \
    __ALTERNATIVE_CFG(oldinstr, newinstr, feature, cfg, 0)

#else /* __ASSEMBLY__ */

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

#endif

#define ALTERNATIVE(oldinstr, newinstr, ...)   \
    _ALTERNATIVE_CFG(oldinstr, newinstr, __VA_ARGS__, 1)

// ------------------------------------------------------------------------
#endif /* _ASM_ALTERNATIVE_H */
