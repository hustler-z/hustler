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

struct arch_globl {
    unsigned long timer_rate_hz;
    unsigned int tbu;
    unsigned int tbl;
    unsigned long lastinc;
    unsigned long long timer_reset_value;

    unsigned long tlb_addr;
    unsigned long tlb_size;

    /* ARM64 */
    unsigned long tlb_fillptr;
    unsigned long tlb_emerg;
    unsigned int first_block_level;
    int has_hafdbs;
};

#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#endif /* _ARCH_GLOBAL_H */
