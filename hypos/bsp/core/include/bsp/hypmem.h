/**
 * Hustler's Project
 *
 * File:  hackmem.h
 * Date:  2024/05/21
 * Usage:
 */

#ifndef _BSP_HACKMEM_H
#define _BSP_HACKMEM_H
// --------------------------------------------------------------
#include <bsp/type.h>
#include <bsp/panic.h>
#include <bsp/numa.h>
#include <asm/at.h>

// --------------------------------------------------------------
void *alloc(size_t len);
void free(void *mem);
void *calloc(size_t nrmb, size_t size);
void *realloc(void *p, size_t size);
// --------------------------------------------------------------
void *alloc_page(unsigned int flags);
void *alloc_pages(unsigned int order, unsigned int flags);
void free_pages(void *v, unsigned int order);
void free_page(void *v);

#define _HM_NO_REFCOUNT 0
#define  HM_NO_REFCOUNT (1U<<_HM_NO_REFCOUNT)
#define _HM_POPULATE_ON_DEMAND 1
#define  HM_POPULATE_ON_DEMAND (1U<<_HM_POPULATE_ON_DEMAND)
#define _HM_NO_DMA      3
#define  HM_NO_DMA      (1U<<_HM_NO_DMA)
#define _HM_EXACT_NODE  4
#define  HM_EXACT_NODE  (1U<<_HM_EXACT_NODE)
#define _HM_NO_OWNER    5
#define  HM_NO_OWNER    (1U<<_HM_NO_OWNER)
#define _HM_NO_TLBFLUSH 6
#define  HM_NO_TLBFLUSH (1U<<_HM_NO_TLBFLUSH)
#define _HM_NO_ICACHE_FLUSH 7
#define  HM_NO_ICACHE_FLUSH (1U<<_HM_NO_ICACHE_FLUSH)
#define _HM_NO_SCRUB    8
#define  HM_NO_SCRUB    (1U<<_HM_NO_SCRUB)
#define _HM_NODE        16
#define  HM_NODE_MASK   ((1U << (8 * sizeof(NODEID_T))) - 1)
#define  HM_NODE(N)     ((((N) + 1) & HM_NODE_MASK) << _HM_NODE)
#define  HM_GET_NODE(F) ((((F) >> _HM_NODE) - 1) & HM_NODE_MASK)
#define _HM_BITS        24
#define  HM_BITS(N)     ((N)<<_HM_BITS)

extern unsigned int memnode_shift;
extern unsigned long memnode_mapsize;
extern nid_t *memnode_map;
extern struct node_data node_data[];

static inline nid_t pfn_to_nid(pfn_t pfn)
{
    nid_t nid;
    unsigned long idx = pfn_to_idx(pfn);

    ASSERT((idx >> memnode_shift) < memnode_mapsize);
    nid = memnode_map[idx >> memnode_shift];
    ASSERT(nid < MAX_NUMNODES && node_data[nid].node_spanned_pages);

    return nid;
}

int hypmem_setup(void);
// --------------------------------------------------------------
void *_malloc(unsigned long size, unsigned long align);
void *zalloc(unsigned long size, unsigned long align);

#define malloc(_type) ((_type *)_malloc(sizeof(_type), __alignof__(_type)))
#define _zalloc(_type) ((_type *)zalloc(sizeof(_type), __alignof__(_type)))
static inline void *_malloc_array(unsigned long size,
                                  unsigned long align,
                                  unsigned long num)
{
    if (size && num > UINT_MAX / size)
        return NULL;
    return _malloc(size * num, align);
}

static inline void *zalloc_array(unsigned long size,
                                 unsigned long align,
                                 unsigned long num)
{
    if (size && num > UINT_MAX / size)
        return NULL;
    return zalloc(size * num, align);
}

#define malloc_array(_type, _num) \
    ((_type *)_malloc_array(sizeof(_type), __alignof__(_type), _num))
#define _zalloc_array(_type, _num) \
    ((_type *)zalloc_array(sizeof(_type), __alignof__(_type), _num))

void mfree(void *p);
// --------------------------------------------------------------
#endif /* _BSP_HACKMEM_H */
