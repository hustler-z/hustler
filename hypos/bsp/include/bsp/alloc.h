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
#include <common/type.h>
#include <asm/ttbl.h>
#include <bsp/page.h>



void *halloc(size_t len);
void hfree(void *mem);
void *hcalloc(size_t nrmb, size_t size);
void *hrealloc(void *p, size_t size);

struct page *halloc_pages(unsigned int order,
                          unsigned int flags);
struct page *halloc_page(unsigned int flags);
void hfree_pages(struct page *page,
                 unsigned int order);
void hfree_page(struct page *page);

void *_hmalloc(unsigned long size, unsigned long align);
static inline void *_hmalloc_array(unsigned long size,
                                   unsigned long align,
                                   unsigned long num)
{
    if (size && num > UINT_MAX / size)
        return NULL;
    return _hmalloc(size * num, align);
}

#define hmalloc_array(_type, _num) \
    ((_type *)_hmalloc_array(sizeof(_type), __alignof__(_type), _num))

void hmfree(void *p);
// --------------------------------------------------------------
#endif /* _BSP_ALLOC_H */
