/**
 * Hustler's Project
 *
 * File:  system.h
 * Date:  2024/07/11
 * Usage:
 */

#ifndef _ORG_SYSTEM_H
#define _ORG_SYSTEM_H
// --------------------------------------------------------------

/* --------------------------------------------------------------
 * Built-in Functions for Atomic Memory Access (GNU C)
 *
 * type __sync_fetch_and_add(type *_p, type _v) atomically
 * adds the value of _v to the variable that _p points to.
 * The result is stored in the address that is specified by
 * _p. a full memory barrier is created when this function
 * is invoked.
 *
 * The function returns the initial value of the varible
 * that _p points to.
 *
 * XXX: return value = *_p; *_p++; (fetch then add)
 *
 *           ldxr    w1, [x0]
 *           add     w2, w1, w3
 *           stlxr   w4, w2, [x0]
 *
 * ldxr  - Load Exclusive Register
 * stlxr - Store-Release Exclusive Register
 *
 * XXX: Load-Acquire, Store-Release Technique
 * --------------------------------------------------------------
 */
#define gnu_fetch_and_add(x, v) __sync_fetch_and_add(x, v)

// --------------------------------------------------------------
#endif /* _ORG_SYSTEM_H */
