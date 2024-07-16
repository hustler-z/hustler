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

void qsort(void  *base,
    size_t nel,
    size_t width,
    int (*comp)(const void *, const void *))
{
    size_t wgap, i, j, k;
    char tmp;

    if ((nel > 1) && (width > 0)) {
        // assert(nel <= ((size_t)(-1)) / width); /* check for overflow */
        wgap = 0;
        do {
            wgap = 3 * wgap + 1;
        } while (wgap < (nel-1)/3);
        /* From the above, we know that either wgap == 1 < nel or
         * ((wgap-1)/3 < (int) ((nel-1)/3) <= (nel-1)/3 ==> wgap < nel.
         */
        wgap *= width; /* So this can not overflow if wnel doesn't. */
        nel *= width;  /* Convert nel to 'wnel' */
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

/*
 * Returns a list organized in an intermediate format suited
 * to chaining of merge() calls: null-terminated, no reserved or
 * sentinel head node, "prev" links not maintained.
 */
static struct list_head *merge(void *priv,
				int (*cmp)(void *priv, struct list_head *a,
					struct list_head *b),
				struct list_head *a, struct list_head *b)
{
    struct list_head head, *tail = &head;

    while (a && b) {
        /* if equal, take 'a' - important for sort stability */
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

/*
 * Combine final list merge with restoration of standard doubly-linked
 * list structure.  This approach duplicates code from merge(), but
 * runs faster than the tidier alternatives of either a separate final
 * prev-link restoration pass, or maintaining the prev links
 * throughout.
 */
static void merge_and_restore_back_links(void *priv,
				int (*cmp)(void *priv, struct list_head *a,
					struct list_head *b),
				struct list_head *head,
				struct list_head *a, struct list_head *b)
{
    struct list_head *tail = head;

    while (a && b) {
        /* if equal, take 'a' - important for sort stability */
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
        /*
         * In worst cases this loop may run many iterations.
         * Continue callbacks to the client even though no
         * element comparison is needed, so the client's cmp()
         * routine can invoke cond_resched() periodically.
         */
        (*cmp)(priv, tail->next, tail->next);

        tail->next->prev = tail;
        tail = tail->next;
    } while (tail->next);

    tail->next = head;
    head->prev = tail;
}

/**
 * list_sort - sort a list
 * @priv: private data, opaque to list_sort(), passed to @cmp
 * @head: the list to sort
 * @cmp: the elements comparison function
 *
 * This function implements "merge sort", which has O(nlog(n))
 * complexity.
 *
 * The comparison function @cmp must return a negative value if @a
 * should sort before @b, and a positive value if @a should sort after
 * @b. If @a and @b are equivalent, and their original relative
 * ordering is to be preserved, @cmp must return 0.
 */
void list_sort(void *priv, struct list_head *head,
        int (*cmp)(void *priv, struct list_head *a,
        struct list_head *b))
{
    /* sorted partial lists - last slot is a sentinel */
    struct list_head *part[MAX_LIST_LENGTH_BITS + 1];
    int lev;  /* index into part[] */
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

    merge_and_restore_back_links(priv, cmp, head, part[max_lev], list);
}
