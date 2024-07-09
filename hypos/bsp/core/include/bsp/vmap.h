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
#include <common/type.h>
#include <common/compiler.h>

enum vmap_region {
    VMAP_DEFAULT,
    VMAP_HYPOS,
    VMAP_REGION_NR,
};

void vm_init_type(enum vmap_region type, void *start, void *end);

void *__vmap(const pfn_t *pfn, unsigned int granularity, unsigned int nr,
             unsigned int align, unsigned int flags, enum vmap_region type);
void *vmap(const pfn_t *pfn, unsigned int nr);
void *vmap_contig(pfn_t pfn, unsigned int nr);
void vunmap(const void *va);

void *vmalloc(size_t size);
void *hypos_vmalloc(size_t size);

void *vzalloc(size_t size);
void vfree(void *va);

void __iomem *ioremap(paddr_t pa, size_t len);

unsigned int vmap_size(const void *va);

static inline void iounmap(void __iomem *va)
{
    unsigned long addr = (unsigned long)(void __force *)va;

    vunmap((void *)(addr & PAGE_MASK));
}

int vmap_setup(void);

// --------------------------------------------------------------
#endif /* _BSP_VMAP_H */
