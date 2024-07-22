/**
 * Hustler's Project
 *
 * File:  vmap.h
 * Date:  2024/06/25
 * Usage:
 */

#ifndef _BSP_VMAP_H
#define _BSP_VMAP_H
// --------------------------------------------------------------
#include <asm/ttbl.h>
#include <bsp/type.h>
#include <bsp/compiler.h>
#include <asm/page.h>
#include <asm/at.h>

enum vmap_region {
    VMAP_DEFAULT,
    VMAP_HYPOS,
    VMAP_REGION_NR,
};

void vm_init_type(enum vmap_region type, void *start, void *end);

void *__vmap(const hfn_t *hfn, unsigned int granularity, unsigned int nr,
             unsigned int align, unsigned int flags, enum vmap_region type);
void *vmap(const hfn_t *hfn, unsigned int nr);
void *vmap_contig(hfn_t hfn, unsigned int nr);
void vunmap(const void *va);

void *vmalloc(size_t size);
void *hypos_vmalloc(size_t size);

void *vzalloc(size_t size);
void vfree(void *va);

void __iomem *ioremap(hpa_t pa, size_t len);

unsigned int vmap_size(const void *va);

static inline void iounmap(void __iomem *va)
{
    unsigned long addr = (unsigned long)(void __force *)va;

    vunmap((void *)(addr & PAGE_MASK));
}

void *ioremap(hpa_t pa, size_t len);

void __iomem *ioremap_attr(hpa_t start, size_t len, unsigned int attributes);

static inline void __iomem *ioremap_nocache(hpa_t start, size_t len)
{
    return ioremap_attr(start, len, PAGE_HYPOS_NOCACHE);
}

static inline void __iomem *ioremap_cache(hpa_t start, size_t len)
{
    return ioremap_attr(start, len, PAGE_HYPOS);
}

static inline void __iomem *ioremap_wc(hpa_t start, size_t len)
{
    return ioremap_attr(start, len, PAGE_HYPOS_WC);
}

int vmap_setup(void);

// --------------------------------------------------------------
#endif /* _BSP_VMAP_H */
