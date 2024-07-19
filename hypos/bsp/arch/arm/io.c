/**
 * Hustler's Project
 *
 * File:  io.h
 * Date:  2024/07/12
 * Usage:
 */

#include <asm/io.h>
#include <bsp/vmap.h>

// --------------------------------------------------------------

void *ioremap_attr(hpa_t start, size_t len, unsigned int attributes)
{
    pfn_t pfn = pfn_set(PFN_DOWN(start));
    unsigned int offs = start & (PAGE_SIZE - 1);
    unsigned int nr = PFN_UP(offs + len);
    void *ptr = __vmap(&pfn, nr, 1, 1, attributes, VMAP_DEFAULT);

    if (ptr == NULL)
        return NULL;

    return ptr + offs;
}

void *ioremap(hpa_t pa, size_t len)
{
    return ioremap_attr(pa, len, PAGE_HYPOS_NOCACHE);
}

// --------------------------------------------------------------
