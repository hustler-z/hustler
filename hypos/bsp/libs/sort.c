/**
 * Hustler's Project
 *
 * File:  sort.c
 * Date:  2024/06/07
 * Usage:
 */

#include <bsp/compiler.h>
#include <lib/sort.h>
#include <lib/strops.h>
#include <lib/math.h>

// --------------------------------------------------------------
void qsort(void  *base,
    size_t nel,
    size_t width,
    int (*comp)(const void *, const void *))
{
    size_t wgap, i, j, k;
    char tmp;

    if ((nel > 1) && (width > 0)) {
        wgap = 0;
        do {
            wgap = 3 * wgap + 1;
        } while (wgap < (nel-1)/3);

        wgap *= width;
        nel *= width;
        do {
            i = wgap;
            do {
                j = i;
                do {
                    register char *a;
                    register char *b;

                    j -= wgap;
                    a = j + ((char *)base);
                    b = a + wgap;
                    if ((*comp)(a, b) <= 0) {
                        break;
                    }
                    k = width;
                    do {
                        tmp = *a;
                        *a++ = *b;
                        *b++ = tmp;
                    } while (--k);
                } while (j >= wgap);
                i += width;
            } while (i < nel);
            wgap = (wgap - width)/3;
        } while (wgap);
    }
}

#define MAX_LIST_LENGTH_BITS 20

static struct list_head *merge(void *priv,
				int (*cmp)(void *priv, struct list_head *a,
					struct list_head *b),
				struct list_head *a, struct list_head *b)
{
    struct list_head head, *tail = &head;

    while (a && b) {
        if ((*cmp)(priv, a, b) <= 0) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = a ? : b;
    return head.next;
}

static void merge_and_restore_back_links(void *priv,
				int (*cmp)(void *priv, struct list_head *a,
					struct list_head *b),
				struct list_head *head,
				struct list_head *a, struct list_head *b)
{
    struct list_head *tail = head;

    while (a && b) {
        if ((*cmp)(priv, a, b) <= 0) {
            tail->next = a;
            a->prev = tail;
            a = a->next;
        } else {
            tail->next = b;
            b->prev = tail;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = a ? : b;

    do {
        (*cmp)(priv, tail->next, tail->next);

        tail->next->prev = tail;
        tail = tail->next;
    } while (tail->next);

    tail->next = head;
    head->prev = tail;
}

void list_sort(void *priv, struct list_head *head,
        int (*cmp)(void *priv, struct list_head *a,
        struct list_head *b))
{
    struct list_head *part[MAX_LIST_LENGTH_BITS + 1];
    int lev;
    int max_lev = 0;
    struct list_head *list;

    if (list_empty(head))
        return;

    memset(part, 0, sizeof(part));

    head->prev->next = NULL;
    list = head->next;

    while (list) {
        struct list_head *cur = list;
        list = list->next;
        cur->next = NULL;

        for (lev = 0; part[lev]; lev++) {
            cur = merge(priv, cmp, part[lev], cur);
            part[lev] = NULL;
        }
        if (lev > max_lev) {
            if (unlikely(lev >= ARRAY_SIZE(part)-1))
                lev--;
            max_lev = lev;
        }
        part[lev] = cur;
    }

    for (lev = 0; lev < max_lev; lev++)
        if (part[lev])
            list = merge(priv, cmp, part[lev], list);

    merge_and_restore_back_links(priv, cmp, head,
            part[max_lev], list);
}

void *bsearch(const void *key, const void *base, size_t num,
              size_t size, int (*cmp)(const void *key,
              const void *elt))
{
    size_t start = 0, end = num;
    int result;

    while (start < end) {
        size_t mid = start + (end - start) / 2;

        result = cmp(key, base + mid * size);
        if ( result < 0 )
            end = mid;
        else if ( result > 0 )
            start = mid + 1;
        else
            return (void *)base + mid * size;
    }

    return NULL;
}

void sort(void *base, size_t num, size_t size,
          int (*cmp)(const void *a, const void *b),
          void (*swap)(void *a, void *b, size_t size))
{
    /* pre-scale counters for performance */
    size_t i = (num / 2) * size, n = num * size, c, r;

    /* heapify */
    while (i > 0) {
        for (r = i -= size; r * 2 + size < n; r = c) {
            c = r * 2 + size;
            if ((c < n - size) && (cmp(base + c,
                                       base + c + size) < 0))
                c += size;
            if (cmp(base + r, base + c) >= 0)
                break;
            swap(base + r, base + c, size);
        }
    }

    /* sort */
    for (i = n; i > 0; ) {
        i -= size;
        swap(base, base + i, size);
        for (r = 0; r * 2 + size < i; r = c) {
            c = r * 2 + size;
            if ((c < i - size) && (cmp(base + c,
                                       base + c + size) < 0))
                c += size;
            if (cmp(base + r, base + c) >= 0)
                break;
            swap(base + r, base + c, size);
        }
    }
}

// --------------------------------------------------------------
