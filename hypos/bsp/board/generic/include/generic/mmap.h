/**
 * Hustler's Project
 *
 * File:  mmap.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _GENERIC_MMAP_H
#define _GENERIC_MMAP_H
// ------------------------------------------------------------------------

#include <generic/type.h>

static inline void *map_hypmem(void *addr, unsigned long size)
{
    return NULL;
}

static inline void unmap_hypmem(void *addr)
{

}

void *map_devmem(paddr_t paddr);

// ------------------------------------------------------------------------
#endif /* _GENERIC_MMAP_H */


