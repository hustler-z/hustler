/**
 * Hustler's Project
 *
 * File:  sort.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _LIB_SORT_H
#define _LIB_SORT_H
// --------------------------------------------------------------

#include <bsp/type.h>
#include <lib/list.h>

void qsort(void  *base,
    size_t nel,
    size_t width,
    int (*comp)(const void *, const void *));

void list_sort(void *priv, struct list_head *head,
        int (*cmp)(void *priv, struct list_head *a,
        struct list_head *b));

void *bsearch(const void *key, const void *base, size_t num,
              size_t size, int (*cmp)(const void *key,
              const void *elt));

void sort(void *base, size_t num, size_t size,
          int (*cmp)(const void *a, const void *b),
          void (*swap)(void *a, void *b, size_t size));

// --------------------------------------------------------------
#endif /* _LIB_SORT_H */
