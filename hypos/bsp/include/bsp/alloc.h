/**
 * Hustler's Project
 *
 * File:  alloc.h
 * Date:  2024/05/21
 * Usage:
 */

#ifndef _BSP_ALLOC_H
#define _BSP_ALLOC_H
// --------------------------------------------------------------

#include <generic/type.h>
#include <asm/ttbl.h>

int bootmem_setup(paddr_t ps, paddr_t pe);
pfn_t boot_page_alloc(unsigned long nr_pfns,
                      unsigned long pfn_align);

void *balloc(size_t len);
void bfree(void *mem);
void *bcalloc(size_t mb_nr, size_t mb_size);

// --------------------------------------------------------------
#endif /* _BSP_ALLOC_H */
