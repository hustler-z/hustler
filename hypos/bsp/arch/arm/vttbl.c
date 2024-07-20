/**
 * Hustler's Project
 *
 * File:  vttbl.c
 * Date:  2024/07/13
 * Usage:
 */

#include <bsp/config.h>

// --------------------------------------------------------------
#if IS_IMPLEMENTED(__VTTBL_IMPL)

unsigned int __read_mostly vttbl_root_order;
unsigned int __read_mostly vttbl_root_level;

static pfn_t __read_mostly empty_root_pfn;

static u64 generate_vttbr(uint16_t vmid, pfn_t root_pfn)
{
    return (pfn_to_maddr(root_pfn) | ((u64)vmid << 48));
}

static struct page *vttbl_alloc_page(struct hypos *d)
{
    struct page *pg;

    if (is_hardware_hypos(d)) {
        pg = alloc_domheap_page(NULL, 0);
        if (pg == NULL)
            MSGH(XENLOG_G_ERR "Failed to allocate VTTBL pages for hwdom.\n");
    } else {
        spin_lock(&d->arch.paging.lock);
        pg = page_list_remove_head(&d->arch.paging.vttbl_freelist);
        spin_unlock(&d->arch.paging.lock);
    }

    return pg;
}

static void vttbl_free_page(struct hypos *d, struct page *pg)
{
    if (is_hardware_hypos(d))
        free_page(pg);
    else {
        spin_lock(&d->arch.paging.lock);
        page_list_add_tail(pg, &d->arch.paging.vttbl_freelist);
        spin_unlock(&d->arch.paging.lock);
    }
}

int arch_get_paging_mempool_size(struct hypos *d, u64 *size)
{
    *size = (u64)ACCESS_ONCE(d->arch.paging.vttbl_total_pages) << PAGE_SHIFT;
    return 0;
}

int vttbl_set_allocation(struct hypos *d, unsigned long pages, bool *preempted)
{
    struct page *pg;

    ASSERT(spin_is_locked(&d->arch.paging.lock));

    for ( ; ; ) {
        if (d->arch.paging.vttbl_total_pages < pages) {
            /* Need to allocate more memory from domheap */
            pg = alloc_domheap_page(NULL, 0);
            if (pg == NULL) {
                MSGH("Failed to allocate VTTBL pages.\n");
                return -ENOMEM;
            }
            ACCESS_ONCE(d->arch.paging.vttbl_total_pages) =
                d->arch.paging.vttbl_total_pages + 1;
            page_list_add_tail(pg, &d->arch.paging.vttbl_freelist);
        } else if (d->arch.paging.vttbl_total_pages > pages) {
            /* Need to return memory to domheap */
            pg = page_list_remove_head(&d->arch.paging.vttbl_freelist);
            if(pg) {
                ACCESS_ONCE(d->arch.paging.vttbl_total_pages) =
                    d->arch.paging.vttbl_total_pages - 1;
                free_page(pg);
            } else {
                MSGH("Failed to free VTTBL pages, VTTBL freelist is empty.\n");
                return -ENOMEM;
            }
        } else
            break;

        /* Check to see if we need to yield and try again */
        if (preempted && general_preempt_check()) {
            *preempted = true;
            return -ERESTART;
        }
    }

    return 0;
}

int arch_set_paging_mempool_size(struct hypos *d, u64 size)
{
    unsigned long pages = size >> PAGE_SHIFT;
    bool preempted = false;
    int rc;

    if ((size & ~PAGE_MASK) ||          /* Non page-sized request? */
         pages != (size >> PAGE_SHIFT)) /* 32-bit overflow? */
        return -EINVAL;

    spin_lock(&d->arch.paging.lock);
    rc = vttbl_set_allocation(d, pages, &preempted);
    spin_unlock(&d->arch.paging.lock);

    ASSERT(preempted == (rc == -ERESTART));

    return rc;
}

int vttbl_teardown_allocation(struct hypos *d)
{
    int ret = 0;
    bool preempted = false;

    spin_lock(&d->arch.paging.lock);
    if (d->arch.paging.vttbl_total_pages != 0) {
        ret = vttbl_set_allocation(d, 0, &preempted);
        if (preempted) {
            spin_unlock(&d->arch.paging.lock);
            return -ERESTART;
        }
        ASSERT(d->arch.paging.vttbl_total_pages == 0);
    }
    spin_unlock(&d->arch.paging.lock);

    return ret;
}

void vttbl_dump_info(struct hypos *d)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(d);

    vttbl_read_lock(vttbl);
    MSGH("vttbl mappings for hypos %d (vmid %d):\n",
           d->hypos_id, vttbl->vmid);
    BUG_ON(vttbl->stats.mappings[0] || vttbl->stats.shattered[0]);
    MSGH("  1G mappings: %ld (shattered %ld)\n",
           vttbl->stats.mappings[1], vttbl->stats.shattered[1]);
    MSGH("  2M mappings: %ld (shattered %ld)\n",
           vttbl->stats.mappings[2], vttbl->stats.shattered[2]);
    MSGH("  4K mappings: %ld\n", vttbl->stats.mappings[3]);
    vttbl_read_unlock(vttbl);
}

void dump_vttbl_lookup(struct hypos *d, paddr_t addr)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(d);

    MSGH("dom%d IPA 0x%"PRIpaddr"\n", d->hypos_id, addr);

    MSGH("VTTBL @ %p pfn:%#"PRI_pfn"\n",
           vttbl->root, pfn_x(page_to_pfn(vttbl->root)));

    dump_pt_walk(page_to_maddr(vttbl->root), addr,
                 VTTBL_ROOT_LEVEL, VTTBL_ROOT_PAGES);
}

void vttbl_save_state(struct vcpu *p)
{
    p->arch.sctlr = READ_SYSREG(SCTLR_EL1);

    if (cpus_have_const_cap(ARM64_WORKAROUND_AT_SPECULATE)) {
        WRITE_SYSREG64(generate_vttbr(INVALID_VMID, empty_root_pfn),
                VTTBR_EL2);
        isb();
    }
}

void vttbl_restore_state(struct vcpu *n)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(n->hypos);
    uint8_t *last_vcpu_ran;

    if (is_idle_vcpu(n))
        return;

    WRITE_SYSREG(n->arch.sctlr, SCTLR_EL1);
    WRITE_SYSREG(n->arch.hcr_el2, HCR_EL2);

    asm volatile(ALTERNATIVE("nop", "isb", ARM64_WORKAROUND_AT_SPECULATE));
    WRITE_SYSREG64(vttbl->vttbr, VTTBR_EL2);

    last_vcpu_ran = &vttbl->last_vcpu_ran[smp_processor_id()];

    isb();

    if (*last_vcpu_ran != INVALID_VCPU_ID && *last_vcpu_ran != n->vcpu_id)
        flush_guest_tlb_local();

    *last_vcpu_ran = n->vcpu_id;
}

void vttbl_force_tlb_flush_sync(struct vttbl_hypos *vttbl)
{
    unsigned long flags = 0;
    u64 ovttbr;

    ASSERT(vttbl_is_write_locked(vttbl));

    ovttbr = READ_SYSREG64(VTTBR_EL2);
    if (ovttbr != vttbl->vttbr) {
        u64 vttbr;

        local_irq_save(flags);

        if (!cpus_have_const_cap(ARM64_WORKAROUND_AT_SPECULATE))
            vttbr = vttbl->vttbr;
        else
            vttbr = generate_vttbr(vttbl->vmid, empty_root_pfn);

        WRITE_SYSREG64(vttbr, VTTBR_EL2);

        /* Ensure VTTBR_EL2 is synchronized before flushing the TLBs */
        isb();
    }

    flush_guest_tlb();

    if (ovttbr != READ_SYSREG64(VTTBR_EL2)) {
        WRITE_SYSREG64(ovttbr, VTTBR_EL2);
        /* Ensure VTTBR_EL2 is back in place before continuing. */
        isb();
        local_irq_restore(flags);
    }

    vttbl->need_flush = false;
}

void vttbl_tlb_flush_sync(struct vttbl_hypos *vttbl)
{
    if (vttbl->need_flush)
        vttbl_force_tlb_flush_sync(vttbl);
}

static ttbl_t *vttbl_get_root_pointer(struct vttbl_hypos *vttbl,
                                    gfn_t gfn)
{
    unsigned long root_table;

    root_table = gfn_x(gfn) >> (XEN_PT_LEVEL_ORDER(VTTBL_ROOT_LEVEL) +
                                XEN_PT_LPAE_SHIFT);
    if (root_table >= VTTBL_ROOT_PAGES)
        return NULL;

    return __map_hypos_page(vttbl->root + root_table);
}

static vttbl_access_t vttbl_mem_access_radix_get(struct vttbl_hypos *vttbl,
                                                 gfn_t gfn)
{
    void *ptr;

    if (!vttbl->mem_access_enabled)
        return vttbl->default_access;

    ptr = radix_tree_lookup(&vttbl->mem_access_settings, gfn_x(gfn));
    if (!ptr)
        return vttbl_access_rwx;
    else
        return radix_tree_ptr_to_int(ptr);
}

static inline bool vttbl_is_valid(ttbl_t pte)
{
    return pte.vttbl.type != vttbl_invalid;
}

static inline bool vttbl_is_mapping(ttbl_t pte, unsigned int level)
{
    return vttbl_is_valid(pte) && ttbl_is_mapping(pte, level);
}

static inline bool vttbl_is_superpage(ttbl_t pte, unsigned int level)
{
    return vttbl_is_valid(pte) && ttbl_is_superpage(pte, level);
}

#define GUEST_TABLE_MAP_FAILED 0
#define GUEST_TABLE_SUPER_PAGE 1
#define GUEST_TABLE_NORMAL_PAGE 2

static int vttbl_create_table(struct vttbl_hypos *vttbl, ttbl_t *entry);

static int vttbl_next_level(struct vttbl_hypos *vttbl, bool read_only,
                          unsigned int level, ttbl_t **table,
                          unsigned int offset)
{
    ttbl_t *entry;
    int ret;
    pfn_t pfn;

    entry = *table + offset;

    if (!vttbl_is_valid(*entry)) {
        if (read_only)
            return GUEST_TABLE_MAP_FAILED;

        ret = vttbl_create_table(vttbl, entry);
        if (ret)
            return GUEST_TABLE_MAP_FAILED;
    }

    /* The function vttbl_next_level is never called at the 3rd level */
    ASSERT(level < 3);
    if (vttbl_is_mapping(*entry, level))
        return GUEST_TABLE_SUPER_PAGE;

    pfn = ttbl_get_pfn(*entry);

    unmap_hypos_page(*table);
    *table = map_hypos_page(pfn);

    return GUEST_TABLE_NORMAL_PAGE;
}

pfn_t vttbl_get_entry(struct vttbl_hypos *vttbl, gfn_t gfn,
                    vttbl_type_t *t, vttbl_access_t *a,
                    unsigned int *page_order,
                    bool *valid)
{
    paddr_t addr = gfn_to_gaddr(gfn);
    unsigned int level = 0;
    ttbl_t entry, *table;
    int rc;
    pfn_t pfn = INVALID_MFN;
    vttbl_type_t _t;
    DECLARE_OFFSETS(offsets, addr);

    ASSERT(vttbl_is_locked(vttbl));
    BUILD_BUG_ON(THIRD_MASK != PAGE_MASK);

    /* Allow t to be NULL */
    t = t ?: &_t;

    *t = vttbl_invalid;

    if (valid)
        *valid = false;

    if (gfn_x(gfn) > gfn_x(vttbl->max_mapped_gfn)) {
        for (level = VTTBL_ROOT_LEVEL; level < 3; level++)
            if ((gfn_x(gfn) & (XEN_PT_LEVEL_MASK(level) >> PAGE_SHIFT)) >
                 gfn_x(vttbl->max_mapped_gfn))
                break;

        goto out;
    }

    table = vttbl_get_root_pointer(vttbl, gfn);

    if (!table) {
        ASSERT_UNREACHABLE();
        level = VTTBL_ROOT_LEVEL;
        goto out;
    }

    for (level = VTTBL_ROOT_LEVEL; level < 3; level++) {
        rc = vttbl_next_level(vttbl, true, level, &table, offsets[level]);
        if (rc == GUEST_TABLE_MAP_FAILED)
            goto out_unmap;
        else if (rc != GUEST_TABLE_NORMAL_PAGE)
            break;
    }

    entry = table[offsets[level]];

    if (vttbl_is_valid(entry)) {
        *t = entry.vttbl.type;

        if (a)
            *a = vttbl_mem_access_radix_get(vttbl, gfn);

        pfn = ttbl_get_pfn(entry);

        pfn = pfn_add(pfn,
                      gfn_x(gfn) & ((1UL << XEN_PT_LEVEL_ORDER(level)) - 1));

        if (valid)
            *valid = ttbl_is_valid(entry);
    }

out_unmap:
    unmap_hypos_page(table);

out:
    if (page_order)
        *page_order = XEN_PT_LEVEL_ORDER(level);

    return pfn;
}

static void vttbl_set_permission(ttbl_t *e, vttbl_type_t t, vttbl_access_t a)
{
    /* First apply type permissions */
    switch (t) {
    case vttbl_ram_rw:
        e->vttbl.xn = 0;
        e->vttbl.write = 1;
        break;

    case vttbl_ram_ro:
        e->vttbl.xn = 0;
        e->vttbl.write = 0;
        break;

    case vttbl_iommu_map_rw:
    case vttbl_map_foreign_rw:
    case vttbl_grant_map_rw:
    case vttbl_mmio_direct_dev:
    case vttbl_mmio_direct_nc:
    case vttbl_mmio_direct_c:
        e->vttbl.xn = 1;
        e->vttbl.write = 1;
        break;

    case vttbl_iommu_map_ro:
    case vttbl_map_foreign_ro:
    case vttbl_grant_map_ro:
    case vttbl_invalid:
        e->vttbl.xn = 1;
        e->vttbl.write = 0;
        break;

    case vttbl_max_real_type:
        BUG();
        break;
    }

    /* Then restrict with access permissions */
    switch (a) {
    case vttbl_access_rwx:
        break;
    case vttbl_access_wx:
        e->vttbl.read = 0;
        break;
    case vttbl_access_rw:
        e->vttbl.xn = 1;
        break;
    case vttbl_access_w:
        e->vttbl.read = 0;
        e->vttbl.xn = 1;
        break;
    case vttbl_access_rx:
    case vttbl_access_rx2rw:
        e->vttbl.write = 0;
        break;
    case vttbl_access_x:
        e->vttbl.write = 0;
        e->vttbl.read = 0;
        break;
    case vttbl_access_r:
        e->vttbl.write = 0;
        e->vttbl.xn = 1;
        break;
    case vttbl_access_n:
    case vttbl_access_n2rwx:
        e->vttbl.read = e->vttbl.write = 0;
        e->vttbl.xn = 1;
        break;
    }
}

static ttbl_t pfn_to_vttbl_entry(pfn_t pfn, vttbl_type_t t, vttbl_access_t a)
{
    ttbl_t e = (ttbl_t) {
        .vttbl.af = 1,
        .vttbl.read = 1,
        .vttbl.table = 1,
        .vttbl.valid = 1,
        .vttbl.type = t,
    };

    BUILD_BUG_ON(vttbl_max_real_type > (1 << 4));

    switch (t) {
    case vttbl_mmio_direct_dev:
        e.vttbl.mattr = MATTR_DEV;
        e.vttbl.sh = LPAE_SH_OUTER;
        break;
    case vttbl_mmio_direct_c:
        e.vttbl.mattr = MATTR_MEM;
        e.vttbl.sh = LPAE_SH_OUTER;
        break;
    case vttbl_mmio_direct_nc:
        e.vttbl.mattr = MATTR_MEM_NC;
        e.vttbl.sh = LPAE_SH_OUTER;
        break;
    default:
        e.vttbl.mattr = MATTR_MEM;
        e.vttbl.sh = LPAE_SH_INNER;
        break;
    }

    vttbl_set_permission(&e, t, a);

    ASSERT(!(pfn_to_maddr(pfn) & ~PADDR_MASK));

    ttbl_set_pfn(e, pfn);

    return e;
}

/* Generate table entry with correct attributes. */
static ttbl_t page_to_vttbl_table(struct page *page)
{
    return pfn_to_vttbl_entry(page_to_pfn(page),
            vttbl_ram_rw, vttbl_access_rwx);
}

static inline void vttbl_write_pte(ttbl_t *p,
                                   ttbl_t pte, bool clean_pte)
{
    write_pte(p, pte);
    if (clean_pte)
        clean_dcache(*p);
}

static inline void vttbl_remove_pte(ttbl_t *p, bool clean_pte)
{
    ttbl_t pte;

    memset(&pte, 0x00, sizeof(pte));
    vttbl_write_pte(p, pte, clean_pte);
}

/* Allocate a new page table page and hook it in via the given entry. */
static int vttbl_create_table(struct vttbl_hypos *vttbl, ttbl_t *entry)
{
    struct page *page;
    ttbl_t *p;

    ASSERT(!vttbl_is_valid(*entry));

    page = vttbl_alloc_page(vttbl->hypos);
    if (page == NULL)
        return -ENOMEM;

    page_list_add(page, &vttbl->pages);

    p = __map_hypos_page(page);
    clear_page(p);

    if (vttbl->clean_pte)
        clean_dcache_va_range(p, PAGE_SIZE);

    unmap_hypos_page(p);

    vttbl_write_pte(entry, page_to_vttbl_table(page),
                    vttbl->clean_pte);

    return 0;
}

static int vttbl_mem_access_radix_set(struct vttbl_hypos *vttbl, gfn_t gfn,
                                    vttbl_access_t a)
{
    int rc;

    if (!vttbl->mem_access_enabled)
        return 0;

    if (vttbl_access_rwx == a) {
        radix_tree_delete(&vttbl->mem_access_settings, gfn_x(gfn));
        return 0;
    }

    rc = radix_tree_insert(&vttbl->mem_access_settings, gfn_x(gfn),
                           radix_tree_int_to_ptr(a));
    if (rc == -EEXIST) {
        /* If a setting already exists, change it to the new one */
        radix_tree_replace_slot(
            radix_tree_lookup_slot(
                &vttbl->mem_access_settings, gfn_x(gfn)),
            radix_tree_int_to_ptr(a));
        rc = 0;
    }

    return rc;
}

static void vttbl_put_l3_page(const ttbl_t pte)
{
    pfn_t pfn = ttbl_get_pfn(pte);

    ASSERT(vttbl_is_valid(pte));

    if (vttbl_is_foreign(pte.vttbl.type)) {
        ASSERT(pfn_valid(pfn));
        put_page(pfn_to_page(pfn));
    } else if (vttbl_is_ram(pte.vttbl.type) && is_xen_heap_pfn(pfn))
        page_set_xenheap_gfn(pfn_to_page(pfn), INVALID_GFN);
}

/* Free ttbl sub-tree behind an entry */
static void vttbl_free_entry(struct vttbl_hypos *vttbl,
                           ttbl_t entry, unsigned int level)
{
    unsigned int i;
    ttbl_t *table;
    pfn_t pfn;
    struct page *pg;

    /* Nothing to do if the entry is invalid. */
    if (!vttbl_is_valid(entry))
        return;

    if (vttbl_is_superpage(entry, level) || (level == 3)) {
        vttbl->stats.mappings[level]--;
        /* Nothing to do if the entry is a super-page. */
        if (level == 3)
            vttbl_put_l3_page(entry);
        return;
    }

    table = map_hypos_page(ttbl_get_pfn(entry));
    for (i = 0; i < XEN_PT_LPAE_ENTRIES; i++)
        vttbl_free_entry(vttbl, *(table + i), level + 1);

    unmap_hypos_page(table);

    vttbl_tlb_flush_sync(vttbl);

    pfn = ttbl_get_pfn(entry);
    ASSERT(pfn_valid(pfn));

    pg = pfn_to_page(pfn);

    page_list_del(pg, &vttbl->pages);
    vttbl_free_page(vttbl->hypos, pg);
}

static bool vttbl_split_superpage(struct vttbl_hypos *vttbl,
                                  ttbl_t *entry,
                                  unsigned int level,
                                  unsigned int target,
                                  const unsigned int *offsets)
{
    struct page *page;
    unsigned int i;
    ttbl_t pte, *table;
    bool rv = true;

    /* Convenience aliases */
    pfn_t pfn = ttbl_get_pfn(*entry);
    unsigned int next_level = level + 1;
    unsigned int level_order = XEN_PT_LEVEL_ORDER(next_level);

    ASSERT(level < target);
    ASSERT(vttbl_is_superpage(*entry, level));

    page = vttbl_alloc_page(vttbl->hypos);
    if ( !page )
        return false;

    page_list_add(page, &vttbl->pages);
    table = __map_hypos_page(page);

    for (i = 0; i < XEN_PT_LPAE_ENTRIES; i++) {
        ttbl_t *new_entry = table + i;

        pte = *entry;
        ttbl_set_pfn(pte, pfn_add(pfn, i << level_order));

        pte.vttbl.table = (next_level == 3);

        write_pte(new_entry, pte);
    }

    /* Update stats */
    vttbl->stats.shattered[level]++;
    vttbl->stats.mappings[level]--;
    vttbl->stats.mappings[next_level] += XEN_PT_LPAE_ENTRIES;

    if (next_level != target)
        rv = vttbl_split_superpage(vttbl, table + offsets[next_level],
                                 level + 1, target, offsets);

    if (vttbl->clean_pte)
        clean_dcache_va_range(table, PAGE_SIZE);

    unmap_hypos_page(table);

    vttbl_write_pte(entry, page_to_vttbl_table(page), vttbl->clean_pte);

    return rv;
}

static int __vttbl_set_entry(struct vttbl_hypos *vttbl,
                           gfn_t sgfn,
                           unsigned int page_order,
                           pfn_t spfn,
                           vttbl_type_t t,
                           vttbl_access_t a)
{
    unsigned int level = 0;
    unsigned int target = 3 - (page_order / XEN_PT_LPAE_SHIFT);
    ttbl_t *entry, *table, orig_pte;
    int rc;
    /* A mapping is removed if the MFN is invalid. */
    bool removing_mapping = pfn_eq(spfn, INVALID_MFN);
    DECLARE_OFFSETS(offsets, gfn_to_gaddr(sgfn));

    ASSERT(vttbl_is_write_locked(vttbl));
    ASSERT(target > 0 && target <= 3);

    table = vttbl_get_root_pointer(vttbl, sgfn);
    if (!table)
        return -EINVAL;

    for (level = VTTBL_ROOT_LEVEL; level < target; level++) {
        rc = vttbl_next_level(vttbl, removing_mapping,
                            level, &table, offsets[level]);
        if (rc == GUEST_TABLE_MAP_FAILED) {
            rc = removing_mapping ?  0 : -ENOENT;
            goto out;
        } else if (rc != GUEST_TABLE_NORMAL_PAGE)
            break;
    }

    entry = table + offsets[level];

    if (level < target) {
        /* We need to split the original page. */
        ttbl_t split_pte = *entry;

        ASSERT(vttbl_is_superpage(*entry, level));

        if (!vttbl_split_superpage(vttbl, &split_pte, level, target, offsets)) {
            vttbl->stats.mappings[level]++;

            /* Free the allocated sub-tree */
            vttbl_free_entry(vttbl, split_pte, level);

            rc = -ENOMEM;
            goto out;
        }

        vttbl_remove_pte(entry, vttbl->clean_pte);
        vttbl_force_tlb_flush_sync(vttbl);

        vttbl_write_pte(entry, split_pte, vttbl->clean_pte);

        /* then move to the level we want to make real changes */
        for ( ; level < target; level++) {
            rc = vttbl_next_level(vttbl, true, level, &table, offsets[level]);

            /*
             * The entry should be found and either be a table
             * or a superpage if level 3 is not targeted
             */
            ASSERT(rc == GUEST_TABLE_NORMAL_PAGE ||
                   (rc == GUEST_TABLE_SUPER_PAGE && target < 3));
        }

        entry = table + offsets[level];
    }

    ASSERT(level == target);

    orig_pte = *entry;

    ASSERT(!vttbl->mem_access_enabled || page_order == 0 ||
           vttbl->hypos->is_dying);

    ASSERT(!pfn_eq(INVALID_MFN, spfn) || (a == vttbl_access_rwx));

    rc = vttbl_mem_access_radix_set(vttbl, sgfn, a);
    if (rc)
        goto out;

    if (ttbl_is_valid(orig_pte) || removing_mapping)
        vttbl_remove_pte(entry, vttbl->clean_pte);

    if (removing_mapping)
        /* Flush can be deferred if the entry is removed */
        vttbl->need_flush |= !!ttbl_is_valid(orig_pte);
    else {
        ttbl_t pte = pfn_to_vttbl_entry(spfn, t, a);

        if (level < 3)
            pte.vttbl.table = 0; /* Superpage entry */

        if (ttbl_is_valid(orig_pte)) {
            if (likely(!vttbl->mem_access_enabled) ||
                VTTBL_CLEAR_PERM(pte) != VTTBL_CLEAR_PERM(orig_pte))
                vttbl_force_tlb_flush_sync(vttbl);
            else
                vttbl->need_flush = true;
        } else if ( !vttbl_is_valid(orig_pte) ) /* new mapping */
            vttbl->stats.mappings[level]++;

        vttbl_write_pte(entry, pte, vttbl->clean_pte);

        vttbl->max_mapped_gfn = gfn_max(vttbl->max_mapped_gfn,
                                      gfn_add(sgfn, (1UL << page_order) - 1));
        vttbl->lowest_mapped_gfn = gfn_min(vttbl->lowest_mapped_gfn, sgfn);
    }

    if (is_iommu_enabled(vttbl->hypos) &&
        (ttbl_is_valid(orig_pte) || ttbl_is_valid(*entry))) {
        unsigned int flush_flags = 0;

        if (ttbl_is_valid(orig_pte))
            flush_flags |= IOMMU_FLUSHF_modified;
        if (ttbl_is_valid(*entry))
            flush_flags |= IOMMU_FLUSHF_added;

        rc = iommu_iotlb_flush(vttbl->hypos, _dfn(gfn_x(sgfn)),
                               1UL << page_order, flush_flags);
    }
    else
        rc = 0;

    if (vttbl_is_valid(orig_pte) &&
        !pfn_eq(ttbl_get_pfn(*entry), ttbl_get_pfn(orig_pte)))
        vttbl_free_entry(vttbl, orig_pte, level);

out:
    unmap_hypos_page(table);

    return rc;
}

int vttbl_set_entry(struct vttbl_hypos *vttbl,
                  gfn_t sgfn,
                  unsigned long nr,
                  pfn_t spfn,
                  vttbl_type_t t,
                  vttbl_access_t a)
{
    int rc = 0;

    if (unlikely(vttbl->hypos->is_dying))
        return -ENOMEM;

    while (nr) {
        unsigned long mask;
        unsigned long order;

        mask = !pfn_eq(spfn, INVALID_MFN) ? pfn_x(spfn) : 0;
        mask |= gfn_x(sgfn) | nr;

        /* Always map 4k by 4k when memaccess is enabled */
        if (unlikely(vttbl->mem_access_enabled))
            order = THIRD_ORDER;
        else if (!(mask & ((1UL << FIRST_ORDER) - 1)))
            order = FIRST_ORDER;
        else if (!(mask & ((1UL << SECOND_ORDER) - 1)))
            order = SECOND_ORDER;
        else
            order = THIRD_ORDER;

        rc = __vttbl_set_entry(vttbl, sgfn, order, spfn, t, a);
        if (rc)
            break;

        sgfn = gfn_add(sgfn, (1 << order));
        if (!pfn_eq(spfn, INVALID_MFN))
           spfn = pfn_add(spfn, (1 << order));

        nr -= (1 << order);
    }

    return rc;
}

/* Invalidate all entries in the table. The vttbl should be write locked. */
static void vttbl_invalidate_table(struct vttbl_hypos *vttbl,
                                   pfn_t pfn)
{
    ttbl_t *table;
    unsigned int i;

    ASSERT(vttbl_is_write_locked(vttbl));

    table = map_hypos_page(pfn);

    for (i = 0; i < XEN_PT_LPAE_ENTRIES; i++) {
        ttbl_t pte = table[i];

        if (!pte.vttbl.valid)
            continue;

        pte.vttbl.valid = 0;

        vttbl_write_pte(&table[i], pte, vttbl->clean_pte);
    }

    unmap_hypos_page(table);

    vttbl->need_flush = true;
}

void vttbl_clear_root_pages(struct vttbl_hypos *vttbl)
{
    unsigned int i;

    vttbl_write_lock(vttbl);

    for (i = 0; i < VTTBL_ROOT_PAGES; i++)
        clear_and_clean_page(vttbl->root + i);

    vttbl_force_tlb_flush_sync(vttbl);

    vttbl_write_unlock(vttbl);
}

static void vttbl_invalidate_root(struct vttbl_hypos *vttbl)
{
    unsigned int i;

    ASSERT(!iommu_use_hap_pt(vttbl->hypos));

    vttbl_write_lock(vttbl);

    for (i = 0; i < VTTBL_ROOT_LEVEL; i++)
        vttbl_invalidate_table(vttbl, page_to_pfn(vttbl->root + i));

    vttbl_write_unlock(vttbl);
}

void vttbl_hypos_creation_finished(struct hypos *d)
{
    if (!iommu_use_hap_pt(d))
        vttbl_invalidate_root(vttbl_get_hostvttbl(d));
}

bool vttbl_resolve_translation_fault(struct hypos *d, gfn_t gfn)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(d);
    unsigned int level = 0;
    bool resolved = false;
    ttbl_t entry, *table;

    DECLARE_OFFSETS(offsets, gfn_to_gaddr(gfn));

    vttbl_write_lock(vttbl);

    if ( gfn_x(gfn) > gfn_x(vttbl->max_mapped_gfn) )
        goto out;

    table = vttbl_get_root_pointer(vttbl, gfn);

    if (!table) {
        ASSERT_UNREACHABLE();
        goto out;
    }

    for (level = VTTBL_ROOT_LEVEL; level <= 3; level++) {
        int rc;

        entry = table[offsets[level]];

        if (level == 3)
            break;

        if (!ttbl_is_valid(entry))
            break;

        rc = vttbl_next_level(vttbl, true, level,
                              &table, offsets[level]);
        if (rc == GUEST_TABLE_MAP_FAILED)
            goto out_unmap;
        else if (rc != GUEST_TABLE_NORMAL_PAGE)
            break;
    }

    if (ttbl_is_valid(entry)) {
        resolved = true;
        goto out_unmap;
    }

    if (!vttbl_is_valid(entry))
        goto out_unmap;

    if (ttbl_is_table(entry, level))
        vttbl_invalidate_table(vttbl, ttbl_get_pfn(entry));

    resolved = true;
    entry.vttbl.valid = 1;

    vttbl_write_pte(table + offsets[level], entry, vttbl->clean_pte);

out_unmap:
    unmap_hypos_page(table);

out:
    vttbl_write_unlock(vttbl);

    return resolved;
}

static struct page *vttbl_allocate_root(void)
{
    struct page *page;
    unsigned int i;

    page = alloc_domheap_pages(NULL, VTTBL_ROOT_ORDER, 0);
    if (page == NULL)
        return NULL;

    for (i = 0; i < VTTBL_ROOT_PAGES; i++)
        clear_and_clean_page(page + i);

    return page;
}

static int vttbl_alloc_table(struct hypos *d)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(d);

    vttbl->root = vttbl_allocate_root();
    if (!vttbl->root)
        return -ENOMEM;

    vttbl->vttbr = generate_vttbr(vttbl->vmid,
                                  page_to_pfn(vttbl->root));

    vttbl_write_lock(vttbl);
    vttbl_force_tlb_flush_sync(vttbl);
    vttbl_write_unlock(vttbl);

    return 0;
}

int vttbl_teardown(struct hypos *d)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(d);
    unsigned long count = 0;
    struct page *pg;
    int rc = 0;

    vttbl_write_lock(vttbl);

    while ((pg = page_list_remove_head(&vttbl->pages))) {
        vttbl_free_page(vttbl->hypos, pg);
        count++;
        /* Arbitrarily preempt every 512 iterations */
        if (!(count % 512) && hypercall_preempt_check()) {
            rc = -ERESTART;
            break;
        }
    }

    vttbl_write_unlock(vttbl);

    return rc;
}

void vttbl_final_teardown(struct hypos *d)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(d);

    /* vttbl not actually initialized */
    if (!vttbl->hypos)
        return;

    ASSERT(page_list_empty(&vttbl->pages));

    while (vttbl_teardown_allocation(d) == -ERESTART)
        continue; /* No preemption support here */
    ASSERT(page_list_empty(&d->arch.paging.vttbl_freelist));

    if (vttbl->root)
        free_pages(vttbl->root, VTTBL_ROOT_ORDER);

    vttbl->root = NULL;

    vttbl_free_vmid(d);

    radix_tree_destroy(&vttbl->mem_access_settings, NULL);

    vttbl->hypos = NULL;
}

int vttbl_init(struct hypos *d)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(d);
    int rc;
    unsigned int cpu;

    rwlock_init(&vttbl->lock);
    spin_lock_init(&d->arch.paging.lock);
    INIT_PAGE_LIST_HEAD(&vttbl->pages);
    INIT_PAGE_LIST_HEAD(&d->arch.paging.vttbl_freelist);

    vttbl->vmid = INVALID_VMID;
    vttbl->max_mapped_gfn = _gfn(0);
    vttbl->lowest_mapped_gfn = _gfn(ULONG_MAX);

    vttbl->default_access = vttbl_access_rwx;
    vttbl->mem_access_enabled = false;
    radix_tree_init(&vttbl->mem_access_settings);

    vttbl->clean_pte = is_iommu_enabled(d) &&
        !iommu_has_feature(d, IOMMU_FEAT_COHERENT_WALK);

    BUILD_BUG_ON((1 << (sizeof(vttbl->last_vcpu_ran[0]) * 8)) < MAX_VIRT_CPUS);
    BUILD_BUG_ON((1 << (sizeof(vttbl->last_vcpu_ran[0])* 8)) < INVALID_VCPU_ID);

    for_each_possible_cpu(cpu)
       vttbl->last_vcpu_ran[cpu] = INVALID_VCPU_ID;

    vttbl->hypos = d;

    rc = vttbl_alloc_vmid(d);
    if (rc)
        return rc;

    rc = vttbl_alloc_table(d);
    if (rc)
        return rc;

    return 0;
}

int relinquish_vttbl_mapping(struct hypos *d)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(d);
    unsigned long count = 0;
    vttbl_type_t t;
    int rc = 0;
    unsigned int order;
    gfn_t start, end;

    BUG_ON(!d->is_dying);
    /* No mappings can be added in the VTTBL after the VTTBL lock is released. */
    vttbl_write_lock(vttbl);

    start = vttbl->lowest_mapped_gfn;
    end = gfn_add(vttbl->max_mapped_gfn, 1);

    for ( ; gfn_x(start) < gfn_x(end);
          start = gfn_next_boundary(start, order)) {
        pfn_t pfn = vttbl_get_entry(vttbl, start, &t, NULL, &order, NULL);

        count++;

        if (!(count % 512) && hypercall_preempt_check()) {
            rc = -ERESTART;
            break;
        }

        if (!pfn_eq(pfn, INVALID_MFN)) {
            rc = __vttbl_set_entry(vttbl, start, order, INVALID_MFN,
                                 vttbl_invalid, vttbl_access_rwx);
            if (unlikely(rc)) {
                MSGH("Unable to remove mapping gfn=%#"PRI_gfn" order=%u "
                     "from the vttbl of hypos %d\n", gfn_x(start), order,
                     d->hypos_id);
                break;
            }
        }
    }

    vttbl->lowest_mapped_gfn = start;

    vttbl_write_unlock(vttbl);

    return rc;
}

void vttbl_flush_vm(struct vcpu *v)
{
    struct vttbl_hypos *vttbl = vttbl_get_hostvttbl(v->hypos);
    int rc;
    gfn_t start = _gfn(0);

    ASSERT(v == current);
    ASSERT(local_irq_is_enabled());
    ASSERT(v->arch.need_flush_to_ram);

    do {
        rc = vttbl_cache_flush_range(v->hypos, &start, _gfn(ULONG_MAX));
        if (rc == -ERESTART)
            do_softirq();
    } while (rc == -ERESTART);

    if (rc != 0)
        MSGH("VTTBL has not been correctly cleaned (rc = %d)\n", rc);

    vttbl_invalidate_root(vttbl);

    v->arch.need_flush_to_ram = false;
}

/* VTCR value to be configured by all CPUs. Set only once by the boot CPU */
static register_t __read_mostly vtcr;

static void setup_virt_paging_one(void *data)
{
    WRITE_SYSREG(vtcr, VTCR_EL2);

    if (cpus_have_cap(ARM64_WORKAROUND_AT_SPECULATE)) {
        WRITE_SYSREG64(generate_vttbr(INVALID_VMID, empty_root_pfn),
                VTTBR_EL2);
        WRITE_SYSREG(READ_SYSREG(HCR_EL2) | HCR_VM, HCR_EL2);
        isb();

        flush_all_guests_tlb_local();
    }
}

void __init setup_virt_paging(void)
{
    /* Setup Stage 2 address translation */
    register_t val = VTCR_RES1|VTCR_SH0_IS|VTCR_ORGN0_WBWA|VTCR_IRGN0_WBWA;

    static const struct {
        unsigned int pabits; /* Physical Address Size */
        unsigned int t0sz;   /* Desired T0SZ, minimum in comment */
        unsigned int root_order; /* Page order of the root of the vttbl */
        unsigned int sl0;    /* Desired SL0, maximum in comment */
    } pa_range_info[] __initconst = {
        [0] = { 32,      32/*32*/,  0,          1 },
        [1] = { 36,      28/*28*/,  0,          1 },
        [2] = { 40,      24/*24*/,  1,          1 },
        [3] = { 42,      22/*22*/,  3,          1 },
        [4] = { 44,      20/*20*/,  0,          2 },
        [5] = { 48,      16/*16*/,  0,          2 },
        [6] = { 52,      12/*12*/,  4,          2 },
        [7] = { 0 }  /* Invalid */
    };

    unsigned int i;
    unsigned int pa_range = 0x10; /* Larger than any possible value */

    if (pa_range_info[system_cpuinfo.mm64.pa_range].pabits
            < vttbl_ipa_bits)
        vttbl_ipa_bits = pa_range_info[system_cpuinfo.mm64.pa_range].pabits;

    if (system_cpuinfo.mm64.vmid_bits == MM64_VMID_16_BITS_SUPPORT)
        max_vmid = MAX_VMID_16_BIT;

    /* Choose suitable "pa_range" according to the resulted "vttbl_ipa_bits". */
    for (i = 0; i < ARRAY_SIZE(pa_range_info); i++) {
        if (vttbl_ipa_bits == pa_range_info[i].pabits) {
            pa_range = i;
            break;
        }
    }

    /* Check if we found the associated entry in the array */
    if (pa_range >= ARRAY_SIZE(pa_range_info) ||
            !pa_range_info[pa_range].pabits)
        exec_panic("%u-bit VTTBL is not supported\n", vttbl_ipa_bits);

    val |= VTCR_PS(pa_range);
    val |= VTCR_TG0_4K;

    /* Set the VS bit only if 16 bit VMID is supported. */
    if (MAX_VMID == MAX_VMID_16_BIT)
        val |= VTCR_VS;

    val |= VTCR_SL0(pa_range_info[pa_range].sl0);
    val |= VTCR_T0SZ(pa_range_info[pa_range].t0sz);

    vttbl_root_order = pa_range_info[pa_range].root_order;
    vttbl_root_level = 2 - pa_range_info[pa_range].sl0;

    vttbl_ipa_bits = 64 - pa_range_info[pa_range].t0sz;

    MSGH("VTTBL: %d-bit IPA with %d-bit PA and %d-bit VMID\n",
           vttbl_ipa_bits,
           pa_range_info[pa_range].pabits,
           ( MAX_VMID == MAX_VMID_16_BIT ) ? 16 : 8);

    MSGH("VTTBL: %d levels with order-%d root, VTCR 0x%"PRIregister"\n",
           4 - VTTBL_ROOT_LEVEL, VTTBL_ROOT_ORDER, val);

    vttbl_vmid_allocator_init();

    /* It is not allowed to concatenate a level zero root */
    BUG_ON(VTTBL_ROOT_LEVEL == 0 && VTTBL_ROOT_ORDER > 0);
    vtcr = val;

    if (cpus_have_cap(ARM64_WORKAROUND_AT_SPECULATE)) {
        struct page *root;

        root = vttbl_allocate_root();
        if (!root)
            exec_panic("Unable to allocate root table for "
                       "ARM64_WORKAROUND_AT_SPECULATE\n");

        empty_root_pfn = page_to_pfn(root);
    }

    setup_virt_paging_one(NULL);
    smp_call_function(setup_virt_paging_one, NULL, 1);
}

static int cpu_virt_paging_callback(struct notifier_block *nfb,
                                    unsigned long action,
                                    void *hcpu)
{
    switch (action) {
    case CPU_STARTING:
        ASSERT(system_state != SYS_STATE_boot);
        setup_virt_paging_one(NULL);
        break;
    default:
        break;
    }

    return NOTIFY_DONE;
}

static struct notifier_block cpu_virt_paging_nfb = {
    .notifier_call = cpu_virt_paging_callback,
};

static int __init cpu_virt_paging_init(void)
{
    register_cpu_notifier(&cpu_virt_paging_nfb);

    return 0;
}
__bootcall(cpu_virt_paging_init);

#endif
// --------------------------------------------------------------
