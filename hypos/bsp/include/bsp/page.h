/**
 * Hustler's Project
 *
 * File:  page.h
 * Date:  2024/06/25
 * Usage:
 */

#ifndef _BSP_PAGE_H
#define _BSP_PAGE_H
// --------------------------------------------------------------
#include <asm-generic/globl.h>
#include <asm/page.h>
#include <lib/list.h>

struct pglist {
    unsigned long next, prev;
};

struct page {
    struct pglist list;
    unsigned long page_count;
};

struct pglist_head {
    struct page *next, *tail;
};

#define page_head ((struct page *)HYPOS_MEMCHUNK_START)

extern unsigned long page_head_idx;
extern unsigned long pfn_idx_bottom_mask;
extern unsigned long pfn_top_mask;
extern unsigned long pfn_hole_mask;
extern unsigned long pfn_idx_hole_shift;
extern unsigned long pa_va_bottom_mask;
extern unsigned long pa_top_mask;

#define PGLIST_HEAD_INIT(name) { NULL, NULL }
#define PGLIST_HEAD(name) \
    struct pglist_head name = PGLIST_HEAD_INIT(name)
#define INIT_PGLIST_HEAD(head) ((head)->tail = (head)->next = NULL)
#define INIT_PGLIST_ENTRY(ent) ((ent)->prev = (ent)->next = 0)

#define page_to_idx(pg)     ((pg) - page_head)
#define idx_to_page(idx)    (page_head + (idx))

static inline bool
pglist_empty(const struct pglist_head *head)
{
    return !head->next;
}

static inline struct page *
pglist_first(const struct pglist_head *head)
{
    return head->next;
}

static inline struct page *
pglist_last(const struct pglist_head *head)
{
    return head->tail;
}

static inline struct page *
pglist_next(const struct page *page,
            const struct pglist_head *head)
{
    return page != head->tail ? idx_to_page(page->list.next) : NULL;
}

static inline struct page *
pglist_prev(const struct page *page,
               const struct pglist_head *head)
{
    return page != head->next ? idx_to_page(page->list.prev) : NULL;
}

static inline
void pglist_add(struct page *page, struct pglist_head *head)
{
    if (head->next) {
        page->list.next = page_to_idx(head->next);
        head->next->list.prev = page_to_idx(page);
    } else {
        head->tail = page;
        page->list.next = 0;
    }

    page->list.prev = 0;
    head->next = page;
}

static inline bool
__pglist_del_head(struct page *page, struct pglist_head *head,
                     struct page *next, struct page *prev)
{
    if (head->next == page) {
        if (head->tail != page) {
            next->list.prev = 0;
            head->next = next;
        } else
            head->tail = head->next = NULL;
        return 1;
    }

    if (head->tail == page) {
        prev->list.next = 0;
        head->tail = prev;
        return 1;
    }

    return 0;
}

static inline void
pglist_del(struct page *page, struct pglist_head *head)
{
    struct page *next = idx_to_page(page->list.next);
    struct page *prev = idx_to_page(page->list.prev);

    if (!__pglist_del_head(page, head, next, prev)) {
        next->list.prev = page->list.prev;
        prev->list.next = page->list.next;
    }
}

static inline struct page *
pglist_remove_head(struct pglist_head *head)
{
    struct page *page = head->next;

    if (page)
        pglist_del(page, head);

    return page;
}

static inline void
pglist_move(struct pglist_head *dst, struct pglist_head *src)
{
    if (!pglist_empty(src)) {
        *dst = *src;
        INIT_PGLIST_HEAD(src);
    }
}

static inline void
pglist_splice(struct pglist_head *list, struct pglist_head *head)
{
    struct page *first, *last, *at;

    if (pglist_empty(list))
        return;

    if (pglist_empty(head)) {
        head->next = list->next;
        head->tail = list->tail;
        return;
    }

    first = list->next;
    last = list->tail;
    at = head->next;

    ASSERT(first->list.prev == 0);
    ASSERT(first->list.prev == at->list.prev);
    head->next = first;

    last->list.next = page_to_idx(at);
    at->list.prev = page_to_idx(last);
}

#define pglist_for_each(pos, head) \
    for ((pos) = (head)->next; (pos); (pos) = pglist_next(pos, head))
#define pglist_for_each_safe(pos, tmp, head) \
    for ((pos) = (head)->next; \
         (pos) ? ((tmp) = pglist_next(pos, head), 1) : 0; \
         (pos) = (tmp) )
#define pglist_for_each_safe_reverse(pos, tmp, head) \
    for ((pos) = (head)->tail; \
         (pos) ? ((tmp) = pglist_prev(pos, head), 1) : 0; \
         (pos) = (tmp))

static inline
unsigned long pfn_to_idx(unsigned long pfn)
{
    return (pfn & pfn_idx_bottom_mask) |
        ((pfn & pfn_top_mask) >> pfn_idx_hole_shift);
}

static inline
unsigned long idx_to_pfn(unsigned long idx)
{
    return (idx & pfn_idx_bottom_mask) |
        ((idx << pfn_idx_hole_shift) & pfn_top_mask);
}

#define __pfn_to_idx(pfn)              pfn_to_idx(to_pfn(pfn))
#define __idx_tp_pfn(idx)              to_pfn_t(idx_to_pfn(idx))

#define pfn_to_page(pfn) \
    (page_head + (pfn_to_idx(to_pfn(pfn)) - page_head_idx))

#define page_to_pfn(pg) \
    to_pfn_t(idx_to_pfn((unsigned long)((pg) - page_head) +\
                page_head_idx))

#define vmap_to_pfn(va)    pa_to_pfn(va_to_pa((vaddr_t)(va)))
#define vmap_to_page(va)   pfn_to_page(vmap_to_pfn(va))
// --------------------------------------------------------------
#endif /* _BSP_PAGE_H */
