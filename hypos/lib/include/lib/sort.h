/**
 * Hustler's Project
 *
 * File:  sort.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _LIB_SORT_H
#define _LIB_SORT_H
// ------------------------------------------------------------------------

#include <generic/type.h>
#include <lib/list.h>

void qsort(void  *base,
    size_t nel,
    size_t width,
    int (*comp)(const void *, const void *));

void list_sort(void *priv, struct list_head *head,
        int (*cmp)(void *priv, struct list_head *a,
        struct list_head *b));
// ------------------------------------------------------------------------
#endif /* _LIB_SORT_H */
