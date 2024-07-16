/**
 * Hustler's Project
 *
 * File:  setup.h
 * Date:  2024/06/21
 * Usage:
 */

#ifndef _ASM_SETUP_H
#define _ASM_SETUP_H
// --------------------------------------------------------------

struct boot_setup {
    unsigned char *stack;
    unsigned int cpuid;
};

// --------------------------------------------------------------
#endif /* _ASM_SETUP_H */
