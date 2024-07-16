/**
 * Hustler's Project
 *
 * File:  refcount.c
 * Date:  2024/06/28
 * Usage:
 */

#include <bsp/refcount.h>
#include <bsp/type.h>
#include <bsp/panic.h>

// --------------------------------------------------------------
bool refcount_inc(struct refcount *r)
{
    unsigned long nval;
    unsigned long oval = gnu_atomic_load(&r->val);

    while (true) {
        nval = oval + 1;
        if (!oval)
            return false;
        if (gnu_atomic_compare_exchange(&r->val, &oval, nval))
            return true;
    }
}

bool refcount_dec(struct refcount *r)
{
    unsigned long nval;
    unsigned long oval = gnu_atomic_load(&r->val);

    while (true) {
        ASSERT(oval);
        nval = oval - 1;
        if (gnu_atomic_compare_exchange(&r->val, &oval, nval))
            return !nval;
    }
}
// --------------------------------------------------------------
