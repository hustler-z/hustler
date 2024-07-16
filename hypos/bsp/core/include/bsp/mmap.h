/**
 * Hustler's Project
 *
 * File:  mmap.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _BSP_MMAP_H
#define _BSP_MMAP_H
// ------------------------------------------------------------------------

#include <bsp/type.h>

static inline void *map_hypmem(void *addr, unsigned long size)
{
    return NULL;
}

static inline void unmap_hypmem(void *addr)
{

}

void *map_devmem(paddr_t paddr);

// ------------------------------------------------------------------------
#endif /* _BSP_MMAP_H */


