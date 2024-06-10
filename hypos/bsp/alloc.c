/**
 * Hustler's Project
 *
 * File:  alloc.c
 * Date:  2024/05/22
 * Usage: memory management system setup
 *        (Initialization, Allocation, Deallocation)
 */

#include <bsp/alloc.h>
#include <lib/strops.h>

// --------------------------------------------------------------


/* hypervisor: small chunk of memory allocator
 * a.k.a. bite-size allocator
 */
void *balloc(size_t len)
{
    void *bvaddr = NULL;

    /* TODO
     */
    return bvaddr;
}

void *bcalloc(size_t mb_nr, size_t mb_size)
{
    size_t size = mb_nr * mb_size;
    void *bvaddr;

    bvaddr = balloc(size);
    if (!bvaddr)
        return bvaddr;

    memset(bvaddr, '\0', size);

    return bvaddr;
}

void bfree(void *mem)
{

}

// --------------------------------------------------------------
