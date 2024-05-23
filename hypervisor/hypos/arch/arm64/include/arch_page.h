/**
 * Hustler's Project
 *
 * File:  arch_page.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_PAGE_H
#define _ARCH_PAGE_H
// ---------------------------------------------------------

#include <arch_bitops.h>

/*
 * Granularity | PAGE_SHIFT | LEVELS OF LOOKUP
 * -------------------------------------------
 * 4K          | 12         | 4
 * 16K         | 14         | 4
 * 64K         | 16         | 3
 */
#define PAGE_SHIFT          12
#define PAGE_SIZE           (_AC(1, L) << PAGE_SHIFT)
#define PAGE_MASK           (~(PAGE_SIZE-1))
#define PAGE_OFFSET(ptr)    ((unsigned long)(ptr) & ~PAGE_MASK)
#define PAGE_ALIGN(x)       (((x) + PAGE_SIZE - 1) & PAGE_MASK)

// ---------------------------------------------------------
#endif /* _ARCH_PAGE_H */
