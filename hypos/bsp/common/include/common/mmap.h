/**
 * Hustler's Project
 *
 * File:  mmap.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _COMMON_MMAP_H
#define _COMMON_MMAP_H
// ------------------------------------------------------------------------

#include <common/type.h>

static inline void *map_hypmem(void *addr, unsigned long size)
{
    return NULL;
}

static inline void unmap_hypmem(void *addr)
{

}

void *map_devmem(paddr_t paddr);

// ------------------------------------------------------------------------
#endif /* _COMMON_MMAP_H */


