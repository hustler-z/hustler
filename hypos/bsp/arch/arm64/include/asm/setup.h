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
#include <asm/ttbl.h>

struct init_info {
    unsigned char *stack;
    unsigned int  cpuid;
};

struct mcpu_info {
    struct hyp_regs regs;
};

extern ttbl_t boot_pgtbl0[PGTBL_TTBL_ENTRIES];
extern ttbl_t boot_pgtbl1[PGTBL_TTBL_ENTRIES];
extern ttbl_t boot_pgtbl2[PGTBL_TTBL_ENTRIES];
extern ttbl_t boot_pgtbl3[PGTBL_TTBL_ENTRIES * DATA_NR_ENTRIES(2)];

extern ttbl_t hypos_fixmap[PGTBL_TTBL_ENTRIES];

extern ttbl_t boot_idmap1[PGTBL_TTBL_ENTRIES];
extern ttbl_t boot_idmap2[PGTBL_TTBL_ENTRIES];
extern ttbl_t boot_idmap3[PGTBL_TTBL_ENTRIES];

#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#endif /* _ARCH_SETUP_H */
