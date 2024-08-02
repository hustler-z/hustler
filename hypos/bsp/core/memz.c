/**
 * Hustler's Project
 *
 * File:  memz.c
 * Date:  2024/05/22
 * Usage: memory management system setup
 *        (Initialization, Allocation, Deallocation)
 */

#include <org/section.h>
#include <org/membank.h>
#include <asm/ttbl.h>
#include <bsp/memz.h>
#include <bsp/config.h>
#include <bsp/panic.h>
#include <bsp/debug.h>
#include <lib/strops.h>

// --------------------------------------------------------------
#define HYPOS_MEMZONE 0
#define NR_ZONES    (PADDR_BITS - PAGE_SHIFT + 1)

#if IS_IMPLEMENTED(__BUDDY_ALLOCATOR_IMPL) /* BUDDY_ALLOCATOR */

static DEFINE_SPINLOCK(heap_lock);

#define bits_to_zone(b) (((b) < (PAGE_SHIFT + 1)) ? 1U : ((b) - PAGE_SHIFT))
#define page_to_zone(pg) (is_xen_heap_page(pg) ? HYPOS_MEMZONE :  \
                          (flsl(hfn_get(page_to_hfn(pg))) ? : 1))

typedef struct page_list_head heap_by_zone_and_order_t[NR_ZONES][MAX_ORDER+1];
static heap_by_zone_and_order_t *_heap[MAX_NUMNODES];
#define heap(node, zone, order) ((*_heap[node])[zone][order])

static unsigned long node_need_scrub[MAX_NUMNODES];

static unsigned long *avail[MAX_NUMNODES];
static long total_avail_pages;

static unsigned long init_node(int node, unsigned long hfn,
                                    unsigned long nr, bool *use_tail)
{
    /* First node to be discovered has its heap metadata statically alloced. */
    static heap_by_zone_and_order_t _heap_static;
    static unsigned long avail_static[NR_ZONES];
    unsigned long needed = (sizeof(**_heap) +
                            sizeof(**avail) * NR_ZONES +
                            PAGE_SIZE - 1) >> PAGE_SHIFT;
    int i, j;

    if (!first_node_initialised) {
        _heap[node] = &_heap_static;
        avail[node] = avail_static;
        first_node_initialised = true;
        needed = 0;
    } else if (*use_tail && nr >= needed &&
              arch_hfns_in_directmap(hfn + nr - needed, needed) &&
              (!xenheap_bits ||
               !((hfn + nr - 1) >> (xenheap_bits - PAGE_SHIFT)))) {
        _heap[node] = hfn_to_virt(hfn + nr - needed);
        avail[node] = hfn_to_virt(hfn + nr - 1) +
                      PAGE_SIZE - sizeof(**avail) * NR_ZONES;
    } else if (nr >= needed &&
               arch_hfns_in_directmap(hfn, needed) &&
               (!xenheap_bits ||
               !((hfn + needed - 1) >> (xenheap_bits - PAGE_SHIFT)))) {
        _heap[node] = hfn_to_virt(hfn);
        avail[node] = hfn_to_virt(hfn + needed - 1) +
                      PAGE_SIZE - sizeof(**avail) * NR_ZONES;
        *use_tail = false;
    } else if (get_order_from_bytes(sizeof(**_heap)) ==
               get_order_from_pages(needed)) {
        _heap[node] = alloc_xenheap_pages(get_order_from_pages(needed), 0);
        BUG_ON(!_heap[node]);
        avail[node] = (void *)_heap[node] + (needed << PAGE_SHIFT) -
                      sizeof(**avail) * NR_ZONES;
        needed = 0;
    } else {
        _heap[node] = xmalloc(heap_by_zone_and_order_t);
        avail[node] = xmalloc_array(unsigned long, NR_ZONES);
        BUG_ON(!_heap[node] || !avail[node]);
        needed = 0;
    }

    memset(avail[node], 0, NR_ZONES * sizeof(long));

    for (i = 0; i < NR_ZONES; i++)
        for (j = 0; j <= MAX_ORDER; j++)
            INIT_PAGE_LIST_HEAD(&heap(node, i, j));

    return needed;
}

/* Pages that need a scrub are added to tail, otherwise to head. */
static void page_list_add_scrub(struct page *pg, unsigned int node,
                                unsigned int zone, unsigned int order,
                                unsigned int first_dirty)
{
    PFN_ORDER(pg) = order;
    pg->u.free.first_dirty = first_dirty;
    pg->u.free.scrub_state = BUDDY_NOT_SCRUBBING;

    if (first_dirty != INVALID_DIRTY_IDX) {
        ASSERT(first_dirty < (1U << order));
        page_list_add_tail(pg, &heap(node, zone, order));
    } else
        page_list_add(pg, &heap(node, zone, order));
}

/* SCRUB_PATTERN needs to be a repeating series of bytes. */
#ifndef NDEBUG
#define SCRUB_PATTERN        0xc2c2c2c2c2c2c2c2ULL
#else
#define SCRUB_PATTERN        0ULL
#endif
#define SCRUB_BYTE_PATTERN   (SCRUB_PATTERN & 0xff)

static void poison_one_page(struct page *pg)
{

}

static void check_one_page(struct page *pg)
{

}

static void check_and_stop_scrub(struct page *head)
{
    if (head->u.free.scrub_state == BUDDY_SCRUBBING) {
        typeof(head->u.free) pgfree;

        head->u.free.scrub_state = BUDDY_SCRUB_ABORT;
        spin_lock_kick();
        for (; ;) {
            /* Can't ACCESS_ONCE() a bitfield. */
            pgfree.val = ACCESS_ONCE(head->u.free.val);
            if (pgfree.scrub_state != BUDDY_SCRUB_ABORT)
                break;
            cpu_relax();
        }
    }
}

static struct page *get_free_buddy(unsigned int zone_lo,
                                        unsigned int zone_hi,
                                        unsigned int order,
                                        unsigned int memflags,
                                        const struct hypos *d)
{
    nodeid_t first, node = MEMF_get_node(memflags), req_node = node;
    nodemask_t nodemask = node_online_map;
    unsigned int j, zone, nodemask_retry = 0;
    struct page *pg;
    bool use_unscrubbed = (memflags & MEMF_no_scrub);

    if (d) {
        if (nodes_intersects(nodemask, d->node_affinity))
            nodes_and(nodemask, nodemask, d->node_affinity);
        else
            ASSERT_UNREACHABLE();
    }

    if (node == NUMA_NO_NODE) {
        if (d != NULL)
            node = cycle_node(d->last_alloc_node, nodemask);

        if (node >= MAX_NUMNODES)
            node = cpu_to_node(smp_processor_id());
    } else if (unlikely(node >= MAX_NUMNODES)) {
        ASSERT_UNREACHABLE();
        return NULL;
    }
    first = node;

    for (; ;) {
        zone = zone_hi;
        do {
            /* Check if target node can support the allocation. */
            if (!avail[node] || (avail[node][zone] < (1UL << order)))
                continue;

            /* Find smallest order which can satisfy the request. */
            for (j = order; j <= MAX_ORDER; j++) {
                if ((pg = page_list_remove_head(&heap(node, zone, j)))) {
                    if (pg->u.free.first_dirty == INVALID_DIRTY_IDX)
                        return pg;

                    if ((order == 0) || use_unscrubbed) {
                        check_and_stop_scrub(pg);
                        return pg;
                    }

                    page_list_add_tail(pg, &heap(node, zone, j));
                }
            }
        } while (zone-- > zone_lo); /* careful: unsigned zone may wrap */

        if ((memflags & MEMF_exact_node) && req_node != NUMA_NO_NODE)
            return NULL;

        /* Pick next node. */
        if (!nodemask_test(node, &nodemask)) {
            ASSERT(!nodemask_retry);
            first = node = first_node(nodemask);
            if ( node < MAX_NUMNODES )
                continue;
        } else if ((node = next_node(node, nodemask)) >= MAX_NUMNODES)
            node = first_node(nodemask);

        if (node == first) {
            if ((memflags & MEMF_exact_node) || nodemask_retry++)
                return NULL;
            nodes_andnot(nodemask, node_online_map, nodemask);
            first = node = first_node(nodemask);
            if (node >= MAX_NUMNODES)
                return NULL;
        }
    }
}

static void init_free_page_fields(struct page *pg)
{
    pg->u.inuse.type_info = PGT_TYPE_INFO_INITIALIZER;
    page_set_owner(pg, NULL);
}

/* Allocate 2^@order contiguous pages. */
static struct page *alloc_pages(
    unsigned int zone_lo, unsigned int zone_hi,
    unsigned int order, unsigned int memflags,
    struct hypos *d)
{
    nodeid_t node;
    unsigned int i, buddy_order, zone, first_dirty;
    unsigned long request = 1UL << order;
    struct page *pg;
    bool need_tlbflush = false;
    uint32_t tlbflush_timestamp = 0;
    unsigned int dirty_cnt = 0;
    hfn_t hfn;

    /* Make sure there are enough bits in memflags for nodeID. */
    BUILD_BUG_ON((_MEMF_bits - _MEMF_node) < (8 * sizeof(nodeid_t)));

    ASSERT(zone_lo <= zone_hi);
    ASSERT(zone_hi < NR_ZONES);

    if ( unlikely(order > MAX_ORDER) )
        return NULL;

    spin_lock(&heap_lock);

    if ((outstanding_claims + request > total_avail_pages) &&
        ((memflags & MEMF_no_refcount) ||
        !d || d->outstanding_pages < request)) {
        spin_unlock(&heap_lock);
        return NULL;
    }

    pg = get_free_buddy(zone_lo, zone_hi, order, memflags, d);
    /* Try getting a dirty buddy if we couldn't get a clean one. */
    if (!pg && !(memflags & MEMF_no_scrub))
        pg = get_free_buddy(zone_lo, zone_hi, order,
                            memflags | MEMF_no_scrub, d);
    if (!pg) {
        /* No suitable memory blocks. Fail the request. */
        spin_unlock(&heap_lock);
        return NULL;
    }

    node = page_to_nid(pg);
    zone = page_to_zone(pg);
    buddy_order = PFN_ORDER(pg);

    first_dirty = pg->u.free.first_dirty;

    /* We may have to halve the chunk a number of times. */
    while (buddy_order != order) {
        buddy_order--;
        page_list_add_scrub(pg, node, zone, buddy_order,
                            (1U << buddy_order) > first_dirty ?
                            first_dirty : INVALID_DIRTY_IDX);
        pg += 1U << buddy_order;

        if (first_dirty != INVALID_DIRTY_IDX) {
            /* Adjust first_dirty */
            if (first_dirty >= 1U << buddy_order)
                first_dirty -= 1U << buddy_order;
            else
                first_dirty = 0; /* We've moved past original first_dirty */
        }
    }

    ASSERT(avail[node][zone] >= request);
    avail[node][zone] -= request;
    total_avail_pages -= request;
    ASSERT(total_avail_pages >= 0);

    check_low_mem_virq();

    if (d != NULL)
        d->last_alloc_node = node;

    for (i = 0; i < (1 << order); i++) {
        /* Reference count must continuously be zero for free pages. */
        if ((pg[i].count_info & ~PGC_need_scrub) != PGC_state_free) {
            printk("pg[%u] MFN %"PRI_hfn" c=%#lx o=%u v=%#lx t=%#x\n",
                   i, hfn_get(page_to_hfn(pg + i)),
                   pg[i].count_info, pg[i].v.free.order,
                   pg[i].u.free.val, pg[i].tlbflush_timestamp);
            BUG();
        }

        /* PGC_need_scrub can only be set if first_dirty is valid */
        ASSERT(first_dirty != INVALID_DIRTY_IDX ||
               !(pg[i].count_info & PGC_need_scrub));

        /* Preserve PGC_need_scrub so we can check it after lock is dropped. */
        pg[i].count_info = PGC_state_inuse |
                           (pg[i].count_info & PGC_need_scrub);

        if (!(memflags & MEMF_no_tlbflush))
            accumulate_tlbflush(&need_tlbflush, &pg[i],
                                &tlbflush_timestamp);

        init_free_page_fields(&pg[i]);
    }

    spin_unlock(&heap_lock);

    if (first_dirty != INVALID_DIRTY_IDX ||
         (scrub_debug && !(memflags & MEMF_no_scrub))) {
        for (i = 0; i < (1U << order); i++) {
            if (test_and_clear_bit(_PGC_need_scrub, &pg[i].count_info)) {
                if (!(memflags & MEMF_no_scrub))
                    scrub_one_page(&pg[i]);

                dirty_cnt++;
            } else if (!(memflags & MEMF_no_scrub))
                check_one_page(&pg[i]);
        }

        if (dirty_cnt) {
            spin_lock(&heap_lock);
            node_need_scrub[node] -= dirty_cnt;
            spin_unlock(&heap_lock);
        }
    }

    if (need_tlbflush)
        filtered_flush_tlb_mask(tlbflush_timestamp);

    hfn = page_to_hfn(pg);
    for (i = 0; i < (1U << order); i++)
        flush_page_to_ram(hfn_get(hfn) + i,
                          !(memflags & MEMF_no_icache_flush));

    return pg;
}

static int reserve_offlined_page(struct page *head)
{
    unsigned int node = page_to_nid(head);
    int zone = page_to_zone(head), i,
               head_order = PFN_ORDER(head), count = 0;
    struct page *cur_head;
    unsigned int cur_order, first_dirty;

    ASSERT(spin_is_locked(&heap_lock));

    cur_head = head;

    check_and_stop_scrub(head);

    first_dirty = head->u.free.first_dirty;
    head->u.free.first_dirty = INVALID_DIRTY_IDX;

    page_list_del(head, &heap(node, zone, head_order));

    while (cur_head < (head + (1 << head_order))) {
        struct page *pg;
        int next_order;

        if (page_state_is(cur_head, offlined)) {
            cur_head++;
            if (first_dirty != INVALID_DIRTY_IDX && first_dirty)
                first_dirty--;
            continue;
        }

        next_order = cur_order = 0;

        while (cur_order < head_order) {
            next_order = cur_order + 1;

            if ((cur_head + (1 << next_order)) >= (head + ( 1 << head_order)))
                goto merge;

            for (i = (1 << cur_order), pg = cur_head + (1 << cur_order);
                  i < (1 << next_order);
                  i++, pg++)
                if (page_state_is(pg, offlined))
                    break;
            if (i == ( 1 << next_order)) {
                cur_order = next_order;
                continue;
            } else {
merge:
                page_list_add_scrub(cur_head, node, zone, cur_order,
                                    (1U << cur_order) > first_dirty ?
                                    first_dirty : INVALID_DIRTY_IDX);
                cur_head += (1 << cur_order);

                if (first_dirty != INVALID_DIRTY_IDX) {
                    if (first_dirty >=  1U << cur_order)
                        first_dirty -= 1U << cur_order;
                    else
                        first_dirty = 0;
                }

                break;
            }
        }
    }

    for (cur_head = head; cur_head < head + ( 1UL << head_order);
         cur_head++) {
        if (!page_state_is(cur_head, offlined))
            continue;

        avail[node][zone]--;
        total_avail_pages--;
        ASSERT(total_avail_pages >= 0);

        page_list_add_tail(cur_head,
                           test_bit(_PGC_broken, &cur_head->count_info) ?
                           &page_broken_list : &page_offlined_list);

        count++;
    }

    return count;
}

static nodemask_t node_scrubbing;

static unsigned int node_to_scrub(bool get_node)
{
    nodeid_t node = cpu_to_node(smp_processor_id()), local_node;
    nodeid_t closest = NUMA_NO_NODE;
    u8 dist, shortest = 0xff;

    if (node == NUMA_NO_NODE)
        node = 0;

    if (node_need_scrub[node] &&
        (!get_node || !node_test_and_set(node, node_scrubbing)))
        return node;

    local_node = node;
    for (; ;) {
        do {
            node = cycle_node(node, node_online_map);
        } while (!cpumask_empty(&node_to_cpumask(node)) &&
                 (node != local_node));

        if (node >= MAX_NUMNODES)
            break;

        if (node == local_node)
            break;

        if (node_need_scrub[node]) {
            if (!get_node)
                return node;

            dist = __node_distance(local_node, node);

            if ((dist < shortest || closest == NUMA_NO_NODE) &&
                !node_test_and_set(node, node_scrubbing)) {
                if (closest != NUMA_NO_NODE)
                    node_clear(closest, node_scrubbing);
                shortest = dist;
                closest = node;
            }
        }
    }

    return closest;
}

struct scrub_wait_state {
    struct page *pg;
    unsigned int first_dirty;
    bool drop;
};

static void scrub_continue(void *data)
{
    struct scrub_wait_state *st = data;

    if (st->drop)
        return;

    if (st->pg->u.free.scrub_state == BUDDY_SCRUB_ABORT) {
        /* There is a waiter for this buddy. Release it. */
        st->drop = true;
        st->pg->u.free.first_dirty = st->first_dirty;
        smp_wmb();
        st->pg->u.free.scrub_state = BUDDY_NOT_SCRUBBING;
    }
}

bool scrub_free_pages(void)
{
    struct page *pg;
    unsigned int zone;
    unsigned int cpu = smp_processor_id();
    bool preempt = false;
    nodeid_t node;
    unsigned int cnt = 0;

    node = node_to_scrub(true);
    if (node == NUMA_NO_NODE)
        return false;

    spin_lock(&heap_lock);

    for (zone = 0; zone < NR_ZONES; zone++) {
        unsigned int order = MAX_ORDER;

        do {
            while (!page_list_empty(&heap(node, zone, order))) {
                unsigned int i, dirty_cnt;
                struct scrub_wait_state st;

                /* Unscrubbed pages are always at the end of the list. */
                pg = page_list_last(&heap(node, zone, order));
                if (pg->u.free.first_dirty == INVALID_DIRTY_IDX)
                    break;

                ASSERT(pg->u.free.scrub_state == BUDDY_NOT_SCRUBBING);
                pg->u.free.scrub_state = BUDDY_SCRUBBING;

                spin_unlock(&heap_lock);

                dirty_cnt = 0;

                for (i = pg->u.free.first_dirty; i < (1U << order); i++) {
                    if (test_bit(_PGC_need_scrub, &pg[i].count_info)) {
                        scrub_one_page(&pg[i]);

                        pg[i].count_info &= ~PGC_need_scrub;
                        dirty_cnt++;
                        cnt += 100;
                    } else
                        cnt++;

                    if (pg->u.free.scrub_state == BUDDY_SCRUB_ABORT) {
                        pg->u.free.first_dirty = (i == (1U << order) - 1) ?
                            INVALID_DIRTY_IDX : i + 1;
                        smp_wmb();
                        pg->u.free.scrub_state = BUDDY_NOT_SCRUBBING;

                        spin_lock(&heap_lock);
                        node_need_scrub[node] -= dirty_cnt;
                        spin_unlock(&heap_lock);
                        goto out_nolock;
                    }

                    if (!cpu_online(cpu) || (cnt > 800 && softirq_pending(cpu))) {
                        preempt = true;
                        break;
                    }
                }

                st.pg = pg;
                st.first_dirty = (i >= (1U << order) - 1) ?
                    INVALID_DIRTY_IDX : i + 1;
                st.drop = false;
                spin_lock_cb(&heap_lock, scrub_continue, &st);

                node_need_scrub[node] -= dirty_cnt;

                if (st.drop)
                    goto out;

                if (i >= (1U << order) - 1) {
                    page_list_del(pg, &heap(node, zone, order));
                    page_list_add_scrub(pg, node, zone, order, INVALID_DIRTY_IDX);
                } else
                    pg->u.free.first_dirty = i + 1;

                pg->u.free.scrub_state = BUDDY_NOT_SCRUBBING;

                if (preempt || (node_need_scrub[node] == 0))
                    goto out;
            }
        } while (order-- != 0);
    }

out:
    spin_unlock(&heap_lock);

out_nolock:
    node_clear(node, node_scrubbing);
    return node_to_scrub(false) != NUMA_NO_NODE;
}

static bool mark_page_free(struct page *pg, hfn_t hfn)
{
    bool pg_offlined = false;

    ASSERT(hfn_get(hfn) == hfn_get(page_to_hfn(pg)));

    switch (pg->count_info & PGC_state) {
    case PGC_state_inuse:
        BUG_ON(pg->count_info & PGC_broken);
        pg->count_info = PGC_state_free;
        break;

    case PGC_state_offlining:
        pg->count_info = (pg->count_info & PGC_broken) |
                         PGC_state_offlined;
        pg_offlined = true;
        break;

    default:
        printk("pg MFN %"PRI_hfn" c=%#lx o=%u v=%#lx t=%#x\n",
               hfn_get(hfn),
               pg->count_info, pg->v.free.order,
               pg->u.free.val, pg->tlbflush_timestamp);
        BUG();
    }

    /* If a page has no owner it will need no safety TLB flush. */
    pg->u.free.need_tlbflush = (page_get_owner(pg) != NULL);
    if (pg->u.free.need_tlbflush)
        page_set_tlbflush_timestamp(pg);

    /* This page is not a guest frame any more. */
    page_set_owner(pg, NULL); /* set_ghfn_from_hfn snoops pg owner */
    set_ghfn_from_hfn(hfn_get(hfn), INVALID_M2P_ENTRY);

    return pg_offlined;
}

/* Free 2^@order set of pages. */
static void free_pages(
    struct page *pg, unsigned int order, bool need_scrub)
{
    unsigned long mask;
    hfn_t hfn = page_to_hfn(pg);
    unsigned int i, node = hfn_to_nid(hfn);
    unsigned int zone = page_to_zone(pg);
    bool pg_offlined = false;

    ASSERT(order <= MAX_ORDER);

    spin_lock(&heap_lock);

    for (i = 0; i < (1 << order); i++) {
        if (mark_page_free(&pg[i], hfn_add(hfn, i)))
            pg_offlined = true;

        if (need_scrub) {
            pg[i].count_info |= PGC_need_scrub;
            poison_one_page(&pg[i]);
        }
    }

    avail[node][zone] += 1 << order;
    total_avail_pages += 1 << order;
    if (need_scrub) {
        node_need_scrub[node] += 1 << order;
        pg->u.free.first_dirty = 0;
    } else
        pg->u.free.first_dirty = INVALID_DIRTY_IDX;

    /* Merge chunks as far as possible. */
    while (order < MAX_ORDER) {
        mask = 1UL << order;

        if ((hfn_get(page_to_hfn(pg)) & mask)) {
            struct page *predecessor = pg - mask;

            /* Merge with predecessor block? */
            if (!hfn_valid(page_to_hfn(predecessor)) ||
                !page_state_is(predecessor, free) ||
                (predecessor->count_info & PGC_no_buddy_merge) ||
                (PFN_ORDER(predecessor) != order) ||
                (page_to_nid(predecessor) != node))
                break;

            check_and_stop_scrub(predecessor);

            page_list_del(predecessor, &heap(node, zone, order));

            /* Update predecessor's first_dirty if necessary. */
            if (predecessor->u.free.first_dirty == INVALID_DIRTY_IDX &&
                pg->u.free.first_dirty != INVALID_DIRTY_IDX )
                predecessor->u.free.first_dirty = (1U << order) +
                                                  pg->u.free.first_dirty;

            pg = predecessor;
        } else {
            struct page *successor = pg + mask;

            /* Merge with successor block? */
            if (!hfn_valid(page_to_hfn(successor)) ||
                !page_state_is(successor, free) ||
                (successor->count_info & PGC_no_buddy_merge) ||
                (PFN_ORDER(successor) != order) ||
                (page_to_nid(successor) != node))
                break;

            check_and_stop_scrub(successor);

            /* Update pg's first_dirty if necessary. */
            if (pg->u.free.first_dirty == INVALID_DIRTY_IDX &&
                successor->u.free.first_dirty != INVALID_DIRTY_IDX)
                pg->u.free.first_dirty = (1U << order) +
                                         successor->u.free.first_dirty;

            page_list_del(successor, &heap(node, zone, order));
        }

        order++;
    }

    page_list_add_scrub(pg, node, zone, order, pg->u.free.first_dirty);

    if (pg_offlined)
        reserve_offlined_page(pg);

    spin_unlock(&heap_lock);
}

static unsigned long mark_page_offline(struct page *pg, int broken)
{
    unsigned long nx, x, y = pg->count_info;

    ASSERT(page_is_ram_type(hfn_get(page_to_hfn(pg)),
           RAM_TYPE_CONVENTIONAL));
    ASSERT(spin_is_locked(&heap_lock));

    do {
        nx = x = y;

        if (((x & PGC_state) != PGC_state_offlined) &&
            ((x & PGC_state) != PGC_state_offlining)) {
            nx &= ~PGC_state;
            nx |= (((x & PGC_state) == PGC_state_free)
                   ? PGC_state_offlined : PGC_state_offlining);
        }

        if (broken)
            nx |= PGC_broken;

        if (x == nx)
            break;
    } while ((y = cmpxchg(&pg->count_info, x, nx)) != x);

    return y;
}

static int reserve_heap_page(struct page *pg)
{
    struct page *head = NULL;
    unsigned int i, node = page_to_nid(pg);
    unsigned int zone = page_to_zone(pg);

    for (i = 0; i <= MAX_ORDER; i++) {
        struct page *tmp;

        if (page_list_empty(&heap(node, zone, i)))
            continue;

        page_list_for_each_safe(head, tmp, &heap(node, zone, i)) {
            if ((head <= pg) &&
                (head + (1UL << i) > pg))
                return reserve_offlined_page(head);
        }
    }

    return -EINVAL;

}

int offline_page(hfn_t hfn, int broken, uint32_t *status)
{
    unsigned long old_info = 0;
    struct hypos *owner;
    struct page *pg;

    if (!hfn_valid(hfn)) {
        printk("try to offline out of range page %"PRI_hfn"\n", hfn_get(hfn));
        return -EINVAL;
    }

    *status = 0;
    pg = hfn_to_page(hfn);

    if (is_xen_fixed_hfn(hfn)) {
        *status = PG_OFFLINE_XENPAGE | PG_OFFLINE_FAILED |
                  (DOMID_XEN << PG_OFFLINE_OWNER_SHIFT);
        return -EPERM;
    }

    if (!page_is_ram_type(hfn_get(hfn), RAM_TYPE_CONVENTIONAL)) {
        *status = PG_OFFLINE_FAILED | PG_OFFLINE_NOT_CONV_RAM;
        return -EINVAL;
    }

    if ((pg->count_info & PGC_broken) && (owner = page_get_owner(pg))) {
        *status = PG_OFFLINE_AGAIN;
        hypos_crash(owner);
        return 0;
    }

    spin_lock(&heap_lock);

    old_info = mark_page_offline(pg, broken);

    if (page_state_is(pg, offlined)) {
        reserve_heap_page(pg);

        spin_unlock(&heap_lock);

        *status = broken ? PG_OFFLINE_OFFLINED | PG_OFFLINE_BROKEN
                         : PG_OFFLINE_OFFLINED;
        return 0;
    }

    spin_unlock(&heap_lock);

    if ((owner = page_get_owner_and_reference(pg))) {
        if (p2m_pod_offline_or_broken_hit(pg)) {
            put_page(pg);
            p2m_pod_offline_or_broken_replace(pg);
            *status = PG_OFFLINE_OFFLINED;
        } else {
            *status = PG_OFFLINE_OWNED | PG_OFFLINE_PENDING |
                      (owner->hypos_id << PG_OFFLINE_OWNER_SHIFT);
            /* Release the reference since it will not be allocated anymore */
            put_page(pg);
        }
    } else if (old_info & PGC_xen_heap)
        *status = PG_OFFLINE_XENPAGE | PG_OFFLINE_PENDING |
                  (DOMID_XEN << PG_OFFLINE_OWNER_SHIFT);
    else
        *status = PG_OFFLINE_ANONYMOUS | PG_OFFLINE_FAILED |
                  (DOMID_INVALID << PG_OFFLINE_OWNER_SHIFT);

    if (broken)
        *status |= PG_OFFLINE_BROKEN;

    return 0;
}

/*
 * Online the memory.
 *   The caller should make sure end_hfn <= max_page,
 *   if not, expand_pages() should be called prior to online_page().
 */
unsigned int online_page(hfn_t hfn, uint32_t *status)
{
    unsigned long x, nx, y;
    struct page *pg;
    int ret;

    if (!hfn_valid(hfn)) {
        printk("call expand_pages() first\n");
        return -EINVAL;
    }

    pg = hfn_to_page(hfn);

    spin_lock(&heap_lock);

    y = pg->count_info;
    do {
        ret = *status = 0;

        if (y & PGC_broken) {
            ret = -EINVAL;
            *status = PG_ONLINE_FAILED |PG_ONLINE_BROKEN;
            break;
        }

        if ((y & PGC_state) == PGC_state_offlined) {
            page_list_del(pg, &page_offlined_list);
            *status = PG_ONLINE_ONLINED;
        } else if ( (y & PGC_state) == PGC_state_offlining)
            *status = PG_ONLINE_ONLINED;
        else
            break;

        x = y;
        nx = (x & ~PGC_state) | PGC_state_inuse;
    } while ((y = cmpxchg(&pg->count_info, x, nx)) != x);

    spin_unlock(&heap_lock);

    if ((y & PGC_state) == PGC_state_offlined)
        free_pages(pg, 0, false);

    return ret;
}

int query_page_offline(hfn_t hfn, uint32_t *status)
{
    struct page *pg;

    if (!hfn_valid(hfn) || !page_is_ram_type(hfn_get(hfn),
        RAM_TYPE_CONVENTIONAL)) {
        printk("call expand_pages() first\n");
        return -EINVAL;
    }

    *status = 0;
    spin_lock(&heap_lock);

    pg = hfn_to_page(hfn);

    if (page_state_is(pg, offlining))
        *status |= PG_OFFLINE_STATUS_OFFLINE_PENDING;
    if (pg->count_info & PGC_broken)
        *status |= PG_OFFLINE_STATUS_BROKEN;
    if (page_state_is(pg, offlined))
        *status |= PG_OFFLINE_STATUS_OFFLINED;

    spin_unlock(&heap_lock);

    return 0;
}

static void __pages_setup(const struct page *pg,
                             unsigned long nr_pages,
                             bool need_scrub)
{
    unsigned long s, e;
    unsigned int nid = page_to_nid(pg);

    s = hfn_get(page_to_hfn(pg));
    e = hfn_get(hfn_add(page_to_hfn(pg + nr_pages - 1), 1));
    if (unlikely(!avail[nid])) {
        bool use_tail = IS_ALIGNED(s, 1UL << MAX_ORDER) &&
                        (find_first_set_bit(e) <= find_first_set_bit(s));
        unsigned long n;

        n = init_node(nid, s, nr_pages, &use_tail);
        BUG_ON(n > nr_pages);
        if ( use_tail )
            e -= n;
        else
            s += n;
    }

    while (s < e) {
        unsigned int inc_order = min(MAX_ORDER, flsl(e - s) - 1);

        if (s)
            inc_order = min(inc_order, ffsl(s) - 1U);
        free_pages(hfn_to_page(_hfn(s)), inc_order, need_scrub);
        s += (1UL << inc_order);
    }
}

static void _pages_setup(
    struct page *pg, unsigned long nr_pages)
{
    unsigned long i;
    bool need_scrub = scrub_debug;

    if (!hfn_get(page_to_hfn(pg))) {
        if (nr_pages-- <= 1)
            return;
        pg++;
    }

    spin_lock(&heap_lock);
    first_valid_hfn = hfn_min(page_to_hfn(pg), first_valid_hfn);
    spin_unlock(&heap_lock);

    if (system_state < SYS_STATE_active && opt_bootscrub == BOOTSCRUB_IDLE)
        need_scrub = true;

    for (i = 0; i < nr_pages; ) {
        unsigned int zone = page_to_zone(pg);

        unsigned int nid = page_to_nid(pg);
        unsigned long left = nr_pages - i;
        unsigned long contig_pages;

        for (contig_pages = 1; contig_pages < left; contig_pages++) {
            if (zone != page_to_zone(pg + contig_pages))
                break;

            if (nid != (page_to_nid(pg + contig_pages)))
                break;
        }

        __pages_setup(pg, contig_pages, need_scrub);

        pg += contig_pages;
        i += contig_pages;
    }
}

int __bootfunc end_boot_allocator(void)
{
    unsigned int i;

    /* Pages that are free now go to the hypos sub-allocator. */
    for (i = 0; i < nr_memchunks; i++) {
        struct memchunks *r = &memchunks_list[i];
        if ((mc->start < mc->end) &&
            (hfn_to_nid(_hfn(mc->start)) == cpu_to_node(0))) {
            pages_setup(hfn_to_page(_hfn(mc->start)), mc->end - mc->start);
            mc->end = mc->start;
            break;
        }
    }
    for (i = nr_memchunks; i-- > 0; ) {
        struct memchunks *r = &memchunks_list[i];
        if (mc->start < mc->end)
            pages_setup(hfn_to_page(_hfn(mc->start)), mc->end - mc->start)
    }

    nr_memchunks = 0;

    if (!dma_bitsize && arch_want_default_dmazone())
        dma_bitsize = arch_get_dma_bitsize();

    return 0;
}
#else /* !BUDDY_ALLOCATOR */
static void _pages_setup(struct page *pg, unsigned long nr_pages)
{

}

static struct page *_alloc_pages(unsigned int zone_lo,
                                 unsigned int zone_hi,
                                 unsigned int order,
                                 unsigned int flags)
{
    return NULL;
}

static void _free_pages(struct page *pg, unsigned int order,
                        bool need_scrub)
{

}
#endif /* !BUDDY_ALLOCATOR */
// --------------------------------------------------------------

/* Buddy Allocator */
void pages_setup(hpa_t ps, hpa_t pe)
{
    ps = ROUND_PGUP(ps);
    pe = ROUND_PGDOWN(pe);
    if (pe <= ps)
        return;

    if (ps && !is_heap_hfn(hfn_add(pa_to_hfn(ps), -1)))
        ps += PAGE_SIZE;
    if (!is_heap_hfn(pa_to_hfn(pe)))
        pe -= PAGE_SIZE;

    _pages_setup(pa_to_page(ps), (pe - ps) >> PAGE_SHIFT);
}

void *alloc_pages(unsigned int order, unsigned int flags)
{
    struct page *pg;

    pg = _alloc_pages(HYPOS_MEMZONE, HYPOS_MEMZONE,
                          order, flags | HM_NO_SCRUB);
    if (unlikely(NULL_PTR(pg)))
        return NULL;

    return page_to_va(pg);
}

void free_pages(void *v, unsigned int order)
{
    if (!NULL_PTR(v))
        return;

    _free_pages(va_to_page(v), order, false);
}

void *alloc_page(unsigned int flags)
{
    return alloc_pages(0, 0);
}

void free_page(void *v)
{
    free_pages(v, 0);
}
// --------------------------------------------------------------

/* Small Size Allocator */
void *alloc(size_t len)
{
    void *vaddr = NULL;

    /* TODO
     */
    return vaddr;
}


void *calloc(size_t nrmb, size_t size)
{
    size_t total = nrmb * size;
    void *vaddr = NULL;

    vaddr = alloc(total);
    if (!vaddr)
        return vaddr;

    memset(vaddr, '\0', size);

    return vaddr;
}

void *realloc(void *p, size_t size)
{
    void *vaddr = NULL;

    /* TODO
     */
    return vaddr;
}

void free(void *mem)
{

}
// --------------------------------------------------------------
void *_malloc(unsigned long size, unsigned long align)
{
    void *p = NULL;

    return p;
}

void *zalloc(unsigned long size, unsigned long align)
{
    void *p = _malloc(size, align);

    return p ? memset(p, 0, size) : p;
}

void mfree(void *p)
{

}
// --------------------------------------------------------------
extern unsigned int nr_memchunks;
extern struct memchunk memchunks_list[];

int __bootfunc memz_setup(void)
{
    unsigned int i;

    for (i = 0; i < nr_memchunks; i++) {
        struct memchunk *mc = &memchunks_list[i];
        if ((mc->start < mc->end) &&
            (hfn_to_nid(hfn_set(mc->start)) == cpu_to_node(0))) {
            _pages_setup(hfn_to_page(hfn_set(mc->start)),
                    mc->end - mc->start);
            mc->end = mc->start;
            break;
        }
    }

    for (i = nr_memchunks; i-- > 0; ) {
        struct memchunk *mc = &memchunks_list[i];
        if (mc->start < mc->end)
            _pages_setup(hfn_to_page(hfn_set(mc->start)),
                    mc->end - mc->start);
    }

    nr_memchunks = 0;

    return 0;
}
// --------------------------------------------------------------
