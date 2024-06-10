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

#include <generic/type.h>

void *balloc(size_t len);
void bfree(void *mem);
void *bcalloc(size_t mb_nr, size_t mb_size);

// --------------------------------------------------------------
#endif /* _BSP_ALLOC_H */
