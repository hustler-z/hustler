/**
 * Hustler's Project
 *
 * File:  cache.h
 * Date:  2024/06/27
 * Usage:
 */

#ifndef _ARCH_CACHE_H
#define _ARCH_CACHE_H
// --------------------------------------------------------------


#define icache_clear(op) \
    asm volatile ("ic "#op"\n" \
                  "isb\n" : : : "memory")




// --------------------------------------------------------------
#endif /* _ARCH_CACHE_H */
