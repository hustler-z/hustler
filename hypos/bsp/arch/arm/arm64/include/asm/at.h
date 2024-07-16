/**
 * Hustler's Project
 *
 * File:  at.h
 * Date:  2024/07/01
 * Usage: Address Translation
 */

#ifndef _ASM_AT_H
#define _ASM_AT_H
// --------------------------------------------------------------
#ifndef __ASSEMBLY__
// --------------------------------------------------------------
#include <asm/ttbl.h>
#include <asm/atomic.h>
#include <asm/sysregs.h>
#include <asm/barrier.h>
#include <bsp/compiler.h>
#include <bsp/type.h>
#include <bsp/traps.h>
#include <lib/math.h>
#include <bsp/panic.h>

#define TTBL_OFFSETS(var, va)     \
    const unsigned int var[4] = { \
        PGTBL0_OFFSET(va),        \
        PGTBL1_OFFSET(va),        \
        PGTBL2_OFFSET(va),        \
        PGTBL3_OFFSET(va),        \
    }

/* PTE Shareability
 */
#define PTE_NOT_SHAREABLE 0x0
#define PTE_SHARE_UNDEF   0x1
#define PTE_SHARE_OUTER   0x2
#define PTE_SHARE_INNER   0x3

/* Always use the ARMv8-A long descriptor format in AArch64
 * execution state.
 *
 * When MMU is enabled, software uses virtual address to access
 * the page table. When using load/store instructions to change
 * a page table entry, MMU hardware translate the virtual address
 * before CPU LSU can access the physical memory of the page table
 * entry. As a result, some page table entries needs to describe
 * the memory attribute of the page table itself. When software
 * uses load/store instructions to access the page table, this
 * memory attribute is applied for accesses from CPU LSU.
 *
 * @LSU - Load-Store Unit
 *
 * page table entry format:
 *
 * +------------------+--+----------------------+--+------------------+
 * | Upper attributes |  | Output block address |  | Lower attributes |
 * +------------------+--+----------------------+--+------------------+
 *
 * Large Physical Address Extension (TTBL) - ARMv7-A Long Descriptor
 * format.
 *
 * @ttbl_t - translation table entry
 */

typedef struct __packed {
    unsigned long valid:1;
    unsigned long table:1;

    /* Memory Attributes
     *
     * lower attributes (10 bits)
     * [11:2]
     * ai  - attribute index to the MAIR_ELn
     *       0b111   Normal Memory
     *       0b100   Device Memory
     * sh  - shareable attibute
     * ap  - access permission
     *            Unprivileged(EL0)  Privileged(EL1/2/3)
     *       -------------------------------------------
     *       00   No access          Read & Write
     *       01   Read & Write       Read & Write
     *       10   No access          Read-only
     *       11   Read-only          Read-only
     *       -------------------------------------------
     * af  - access flag
     *       -------------------------------------------
     *       0    the block entry has not yet been used
     *       1    the block entry has been used
     *       -------------------------------------------
     * ng  - non-global
     *       -------------------------------------------
     *       1    associated with a specific task or app
     *       0    apply to all tasks
     *       -------------------------------------------
     * ns  - non-secure memory
     *       -------------------------------------------
     *       1    non-secure
     *       0    secure
     *       -------------------------------------------
     */
    unsigned long ai:3;
    unsigned long ns:1;
    unsigned long ap:2;
    unsigned long sh:2;
    unsigned long af:1;
    unsigned long ng:1;

    /* Base address of block or next table
     * [47:12]
     */
    unsigned long base:36;

    /* [51:48] reserved set as zeros
     */
    unsigned long res1:4;

    /* Upper attributes (16 bits)
     * contig - contiguous
     *
     * XXX: Execute attributes
     * HCR_EL2.{E2H, TGE} Trap General Exception
     *
     * uxn - User (EL0) Execute Never (Not used at EL3,
     *       or EL2 when HCR_EL2.E2H == 0)
     * pxn - Privileged Execute Never (Called XN at EL3,
     *       and EL2 when HCR_EL2.E2H == 0)
     */
    unsigned long contig:1;
    unsigned long pxn:1;
    unsigned long uxn:1;
    unsigned long res2:9;
} ttbl_entry_t;

/* For the purpose of page table walking
 */
typedef struct __packed {
    unsigned long valid:1;
    unsigned long table:1;
    unsigned long pad2:10;
    unsigned long base:36;
    unsigned long pad1:16;
} ttbl_walk_t;

typedef union {
    unsigned long bits;
    ttbl_entry_t  ttbl;
    ttbl_walk_t   walk;
} ttbl_t;

#define SET_BOOT_TTBL(name, nr)                            \
ttbl_t __aligned(PAGE_SIZE) __section(".data.boot_pgtbl")  \
    name[PGTBL_TTBL_ENTRIES * (nr)]

#define SET_TTBL(name, nr)                                 \
ttbl_t __aligned(PAGE_SIZE) name[PGTBL_TTBL_ENTRIES * (nr)]

int mmu_enabled(void);
int ttbl_setup(void);
extern void zero_page(ttbl_t *pte);

static inline void write_pte(ttbl_t *p, ttbl_t pte)
{
    dsb(sy);
    write_atomic(p, pte);
    dsb(sy);
}

static inline bool pte_is_valid(ttbl_t pte)
{
    return pte.walk.valid;
}

static inline bool pte_is_table(ttbl_t pte, unsigned int level)
{
    return (level < 3) && pte.walk.table;
}

static inline bool pte_is_mapped(ttbl_t pte, unsigned int level)
{
    if (level == 3)
        return pte.walk.table;
    else
        return !pte.walk.table;
}

/*
 * VFN -> IFN -> PFN (VA -> IPA -> PA)
 * vfn_t   -  Virtual Page Frame Number
 * ifn_t   -  Immediate Physical Page Frame Number
 * pfn_t   -  Physical Page Frame Number
 */
TYPE_SAFE(unsigned long, pfn);

#define pte_get_pfn(pte)      (pfn_set((pte).walk.base))
#define pte_set_pfn(pte, pfn) \
    (pte).walk.base = pfn_get(pfn)

#define INVALID_PFN_RAW   (~0UL)
#define INVALID_PFN       pfn_set(INVALID_PFN_RAW)
#define INVALID_PFN_INIT  { INVALID_PFN_RAW }

static inline pfn_t pfn_max(pfn_t x, pfn_t y)
{
    return pfn_set(max(pfn_get(x), pfn_get(y)));
}

static inline pfn_t pfn_min(pfn_t x, pfn_t y)
{
    return pfn_set(min(pfn_get(x), pfn_get(y)));
}

static inline bool pfn_eq(pfn_t x, pfn_t y)
{
    return pfn_get(x) == pfn_get(y);
}

static inline pfn_t pfn_add(pfn_t x, long i)
{
    return pfn_set(pfn_get(x) + i);
}

ttbl_t pfn_to_entry(pfn_t pfn, unsigned int attr);
// --------------------------------------------------------------

/* AT => Ask mmu to do <address translation>
 * --------------------------------------------------------------
 * @ at <at_op>, <Xt>
 * --------------------------------------------------------------
 * S1E1R
 * S1E1W
 * S1E0R
 * S1E0W
 * S1E1RP
 * S1E1WP
 * S1E1A
 * S1E2R
 * S1E2W
 * S12E1R
 * S12E1W
 * S12E0R
 * S12E0W
 * S1E2A
 * S1E3R
 * S1E3W
 * S1E3A
 * --------------------------------------------------------------
 */
static inline u64 __va_to_pa(vaddr_t va)
{
    u64 par, tmp = read_sysreg_par();
    asm volatile ("at s1e2r, %0;" : : "r"(va));
    isb();
    par = read_sysreg_par();
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
}

#define PAR_INVALID         (_AC(1, UL) << 0)
void dump_ttbl_walk(vaddr_t va);

static inline paddr_t _va_to_pa(vaddr_t va) {
    u64 par = __va_to_pa(va);

    if (par & PAR_INVALID) {
        dump_ttbl_walk(va);
        panic_par(par);
    }

    return (paddr_t)((par & PADDR_MASK & PAGE_MASK) |
            (va & ~PAGE_MASK));
}

#define va_to_pa(va)     _va_to_pa((vaddr_t)(va))
#define va_to_pfn(va)    pfn_set((va_to_pa((vaddr_t)va) >> PAGE_SHIFT))

#define __pa_to_pfn(pa)  ((unsigned long)(pa) >> PAGE_SHIFT)
#define pa_to_pfn(pa)    pfn_set(__pa_to_pfn(pa))

#define pa_to_directmapoff(x) (x)
#define directmapoff_to_pa(x) (x)

#define __pfn_to_idx(x)  (x)
#define __idx_to_pfn(x)  (x)
#define pfn_to_idx(pfn)  __pfn_to_idx(pfn_get(pfn))
#define idx_to_pfn(idx)  pfn_set(__idx_to_pfn(idx))
// --------------------------------------------------------------
extern unsigned long page_frame_table_base_idx;
extern unsigned long page_frame_table_va_end;
// --------------------------------------------------------------
#define pfn_to_page(pfn) \
    (page_frame_table + (pfn_to_idx(pfn) - page_frame_table_base_idx))

#define page_to_pfn(pg) \
    (idx_to_pfn((unsigned long)((pg) - page_frame_table) \
                + page_frame_table_base_idx))

#define vmap_to_pfn(va)  pa_to_pfn(va_to_pa((vaddr_t)(va)))
#define vmap_to_page(va) pfn_to_page(vmap_to_pfn(va))
// --------------------------------------------------------------
extern unsigned long directmap_va_start;
extern unsigned long directmap_va_end;
extern pfn_t directmap_pfn_start;
extern pfn_t directmap_pfn_end;
extern unsigned long directmap_base_idx;
// --------------------------------------------------------------

#define is_heap_pfn(pfn) ({                  \
    unsigned long _pfn = pfn_get(pfn);       \
    (_pfn >= pfn_get(directmap_pfn_start) && \
     _pfn < pfn_get(directmap_pfn_end));     \
})

void *pa_to_va(paddr_t pa);

#define __va(x)          (pa_to_va(x))
#define pfn_to_va(pfn)   (pa_to_va(pfn_get(pfn) << PAGE_SHIFT))
#define __va_to_pfn(va)  (va_to_pa(va) >> PAGE_SHIFT)
#define __pfn_to_va(pfn) (pa_to_va((paddr_t)(pfn) << PAGE_SHIFT))

#define __pa(x)          (va_to_pa(x))
#define pfn_to_pa(pfn)   ((paddr_t)(pfn_get(pfn) << PAGE_SHIFT))
#define __pfn_to_pa(pfn) ((paddr_t)((pfn) << PAGE_SHIFT))
// --------------------------------------------------------------
TYPE_SAFE(unsigned long, ifn);

#define GV2P_READ        (0U << 0)
#define GV2P_WRITE       (1U << 0)
#define GV2P_EXEC        (1U << 1)

static inline u64 __gva_to_pa(vaddr_t va, unsigned int flags)
{
    u64 par, tmp = read_sysreg_par();
    if (!flags & GV2P_WRITE)
        asm volatile ("at s12e1r, %0;" : : "r"(va));
    else
        asm volatile ("at s12e1w, %0;" : : "r"(va));
    isb();
    par = read_sysreg_par();
    WRITE_SYSREG64(tmp, PAR_EL1);

    return par;
}

static inline u64 __gva_to_ipa(vaddr_t va, unsigned int flags)
{
    u64 par, tmp = read_sysreg_par();
    if (!flags & GV2P_WRITE)
        asm volatile ("at s1e1r, %0;" : : "r"(va));
    else
        asm volatile ("at s1e1w, %0;" : : "r"(va));
    isb();
    par = read_sysreg_par();
    WRITE_SYSREG64(tmp, PAR_EL1);

    return par;
}

// --------------------------------------------------------------
/* Page Implementation */

struct pglist {
    unsigned long next, prev;
};

struct page {
    struct pglist list;
    u64 count;
    u32 flags;
    u32 order;
    void *vaddr;

    struct {
        u32 owner;
        u32 dirty;
    } thread;

    struct {
        u32 used;
        u32 status;
    } tiny;
};

struct pglist_head {
    struct page *next, *tail;
};

extern struct page *page_frame_table;
void page_frame_table_init(void);

#define page_to_idx(pg)         ((pg) - page_frame_table)
#define idx_to_page(idx)        (page_frame_table + (idx))

#define pa_to_page(pa)          pfn_to_page(pa_to_pfn(pa))
#define page_to_pa(pg)          (pfn_to_pa(page_to_pfn(pg)))

#define NR_PAGEFRAME            (HYPOS_PAGE_FRAME_SIZE / sizeof(*page_frame_table))

static inline struct page *va_to_page(const void *v)
{
    unsigned long va = (unsigned long)v;
    unsigned long idx;

    ASSERT(va >= HYPOS_VIRT_START);
    ASSERT(va < directmap_va_end);

    idx = (va - HYPOS_HEAP_VIRT_START) >> PAGE_SHIFT;
    idx += pfn_to_idx(directmap_pfn_start);

    return page_frame_table + idx - directmap_base_idx;
}

static inline void *page_to_va(const struct page *pg)
{
    return pfn_to_va(page_to_pfn(pg));
}

#define PGLIST_HEAD_INIT(name)  { NULL, NULL }
#define PGLIST_HEAD(name) \
    struct pglist_head name = PGLIST_HEAD_INIT(name)
#define INIT_PGLIST_HEAD(head)  ((head)->tail = (head)->next = NULL)
#define INIT_PGLIST_ENTRY(ent)  ((ent)->prev = (ent)->next = 0)

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
// --------------------------------------------------------------
#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#endif /* _ASM_AT_H */
