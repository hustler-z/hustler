/**
 * Hustler's Project
 *
 * File:  refcount.h
 * Date:  2024/06/28
 * Usage:
 */

#ifndef _BSP_REFCOUNT_H
#define _BSP_REFCOUNT_H
// --------------------------------------------------------------
#include <bsp/compiler.h>
#include <bsp/type.h>

struct refcount {
    unsigned long val;
};
// --------------------------------------------------------------
static inline
unsigned long gnu_atomic_load(const unsigned long *p)
{
    return __gnu_atomic_load(p);
}

static inline
unsigned long gnu_atomic_load_u32(const unsigned int *p)
{
    return __gnu_atomic_load(p);
}

static inline
unsigned long gnu_atomic_load_s32(const int *p)
{
    return __gnu_atomic_load(p);
}
// --------------------------------------------------------------
static inline
void gnu_atomic_store(unsigned long *p, unsigned long v)
{
    __gnu_atomic_store(p, v);
}

static inline
void gnu_atomic_store_u32(unsigned int *p, unsigned int v)
{
    __gnu_atomic_store(p, v);
}

static inline
void gnu_atomic_store_s32(int *p, int v)
{
    __gnu_atomic_store(p, v);
}
// --------------------------------------------------------------
static inline
bool gnu_atomic_compare_exchange(unsigned long *p,
                                 unsigned long *oval,
                                 unsigned long nval)
{
    return __gnu_compare_exchange(p, oval, nval);
}

static inline
void refcount_set(struct refcount *r,
                  unsigned long v)
{
    gnu_atomic_store(&r->val, v);
}

static inline
unsigned long refcount_val(struct refcount *r)
{
    return gnu_atomic_load(&r->val);
}

bool refcount_inc(struct refcount *r);
bool refcount_dec(struct refcount *r);
// --------------------------------------------------------------
#endif /* _BSP_REFCOUNT_H */
