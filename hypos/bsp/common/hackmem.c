/**
 * Hustler's Project
 *
 * File:  allocation.c
 * Date:  2024/05/22
 * Usage: memory management system setup
 *        (Initialization, Allocation, Deallocation)
 */

#include <asm-generic/section.h>
#include <asm/ttbl.h>
#include <bsp/hackmem.h>
#include <bsp/panic.h>
#include <bsp/debug.h>
#include <lib/strops.h>

// --------------------------------------------------------------
struct page *halloc_pages(unsigned int order,
                         unsigned int flags)
{
    struct page *page = NULL;

    return page;
}

void hfree_pages(struct page *page,
                 unsigned int order)
{

}

struct page *halloc_page(unsigned int flags)
{
    return halloc_pages(0, flags);
}

void hfree_page(struct page *page)
{
    hfree_pages(page, 0);
}
// --------------------------------------------------------------
void *halloc(size_t len)
{
    void *hvaddr = NULL;

    /* TODO
     */
    return hvaddr;
}

void *hcalloc(size_t nrmb, size_t size)
{
    size_t total = nrmb * size;
    void *hvaddr = NULL;

    hvaddr = halloc(total);
    if (!hvaddr)
        return hvaddr;

    memset(hvaddr, '\0', size);

    return hvaddr;
}

void *hrealloc(void *p, size_t size)
{
    void *hvaddr = NULL;

    /* TODO
     */
    return hvaddr;
}

void hfree(void *mem)
{

}
// --------------------------------------------------------------
void *_hmalloc(unsigned long size, unsigned long align)
{
    void *p = NULL;

    return p;
}


void hmfree(void *p)
{

}
// --------------------------------------------------------------
int __bootfunc hackmem_setup(void)
{
    return 0;
}
// --------------------------------------------------------------
