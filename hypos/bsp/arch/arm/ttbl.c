/**
 * Hustler's Project
 *
 * File:  ttbl.c
 * Date:  2024/05/20
 * Usage: exception handler implementation in C
 */

#include <asm/map.h>
#include <asm/sysregs.h>
#include <asm/cache.h>
#include <asm/xaddr.h>
#include <org/section.h>
#include <org/hypos.h>
#include <org/globl.h>
#include <org/cache.h>
#include <org/smp.h>
#include <bsp/debug.h>
#include <bsp/panic.h>
#include <bsp/percpu.h>

// --------------------------------------------------------------

/* Boot-time Page Table Setup
 * --------------------------------------------------------------
 * boot_pgtbl<n> - 4-level page table used during boot time.
 * boot_idmap<m> - Identity Mapping used during boot time.
 * --------------------------------------------------------------
 */
SET_BOOT_TTBL(boot_pgtbl0, 1);
SET_BOOT_TTBL(boot_pgtbl1, 1);
SET_BOOT_TTBL(boot_pgtbl2, 1);
SET_BOOT_TTBL(boot_pgtbl3, HYPOS_DATA_NR_ENTRIES(2));

/* hypos fixed mapping for console, peripherals, etc.
 */
SET_BOOT_TTBL(hypos_fixmap, 1);

/* Boot-time ID Mapping Page Table
 */
SET_BOOT_TTBL(boot_idmap1, 1);
SET_BOOT_TTBL(boot_idmap2, 1);
SET_BOOT_TTBL(boot_idmap3, 1);

/* hypos ID Mapping Page Table in C World
 */
SET_TTBL(hypos_idmap1, 1);
SET_TTBL(hypos_idmap2, 1);
SET_TTBL(hypos_idmap3, 1);

/* hypos Run-time Page Table in C World
 */
SET_TTBL(hypos_pgtbl0, 1);
DEFINE_PERCPU(ttbl_t *, hypos_pgtbl0);
static SET_TTBL(hypos_pgtbl1, 1);
static SET_TTBL(hypos_pgtbl2, 1);
static SET_TTBL(hypos_pgtbl3, HYPOS_DATA_NR_ENTRIES(2));

// --------------------------------------------------------------
/* for other cpu to locate the address of Translation Table */
u64 __section(".data.ttbr") ttbr_base;

static void clear_table(void *table)
{
    zero_page(table);
    clean_and_invalidate_dcache_va_range(table, PAGE_SIZE);
}

static void clear_boot_ttbl(void)
{
    clear_table(boot_pgtbl0);
    clear_table(boot_pgtbl1);
    clear_table(boot_idmap1);
    clear_table(boot_pgtbl2);
    clear_table(boot_pgtbl3);
}

static void set_ttbr_base(ttbl_t *root)
{
    void *ptr = map_hypos_page(va_to_hfn(&ttbr_base));

    ptr += PAGE_OFFSET(&ttbr_base);

    *(u64 *)ptr = va_to_pa(root);

    clean_dcache_va_range(ptr, sizeof(u64));

    unmap_hypos_page(ptr);
}

int prep_smp_mm(int cpu)
{
    clear_boot_ttbl();

    set_ttbr_base(hypos_pgtbl0);

    return 0;
}

// --------------------------------------------------------------

void __dump_ttbl_walk(hpa_t ttbr, hpa_t addr,
                      unsigned int root_level,
                      unsigned int nr_root_tables)
{
    const hfn_t root_hfn = pa_to_hfn(ttbr);
    ttbl_t pte, *mapping;
    unsigned int level, root_table;
    TTBL_OFFSETS(offsets, addr);
    const char *lvlstr[4] = {"0th", "1st", "2nd", "3rd"};

    if (nr_root_tables > 1) {
        root_table = offsets[root_level - 1];
        if (root_table >= nr_root_tables)
            return;
    } else
        root_table = 0;

    mapping = map_table(hfn_add(root_hfn, root_table));

    for (level = root_level; ; level++) {
        if (offsets[level] > PGTBL_TTBL_ENTRIES)
            break;

        pte = mapping[offsets[level]];

        MSGH("<%s> [0x%03x]=0x%016lx\n", lvlstr[level],
                offsets[level], pte.bits);

        if (level == 3 || !pte.walk.valid || !pte.walk.table)
            break;

        unmap_table(mapping);
        mapping = map_table(pte_get_hfn(pte));
    }

    unmap_table(mapping);
}

void dump_ttbl_walk(hva_t va)
{
    hpa_t ttbr = READ_SYSREG(TTBR0_EL2);

    MSGH("Walk TTBL VA 0x%016lx on CPU%d via TTBR 0x%016lx\n",
            va, smp_processor_id(), ttbr);

    __dump_ttbl_walk(ttbr, va, 0, 1);
}
// --------------------------------------------------------------

int mmu_enabled(void)
{
    return READ_SYSREG(SCTLR_EL2) & SCTLR_Axx_ELx_M;
}

ttbl_t hfn_to_entry(hfn_t hfn, unsigned int attr)
{
    ttbl_t pte = {
        .ttbl = {
            .valid = 1,
            .table = 0,
            .ai   = attr,
            .af   = 1,
            .ns   = 1,
            .ap   = 0,  /* 0b00, 0b01 - Read & Write */
            .ng   = 1,
            .contig = 0,
            .uxn  = 1, /* No executable in user space */
            .pxn  = 0, /* Privileged executable */
            .res1 = 0,
            .res2 = 0,
        }
    };

    switch (attr) {
    case MT_DEVICE_nGnRnE:
    case MT_DEVICE_nGnRE:
        pte.ttbl.sh = PTE_SHARE_OUTER;
        break;
    case MT_NORMAL_NC: /* Non-Cacheable Normal Memory */
        pte.ttbl.sh = PTE_SHARE_OUTER;
        break;
    default:
        pte.ttbl.sh = PTE_SHARE_INNER;
        break;
    }

    ASSERT(!(hfn_to_pa(hfn) & ~PADDR_MASK));

    pte_set_hfn(pte, hfn);

    return pte;
}

ttbl_t __bootfunc pte_of_va(hva_t va)
{
    hpa_t pa = va + get_globl()->phys_offset;

    return hfn_to_entry(pa_to_hfn(pa), MT_NORMAL);
}

/* Enable / Disable Identity Mapping
 */
void update_idmap(unsigned int ok)
{
    hpa_t __pa = va_to_pa(__hypos_start);
    int ret;

    if (ok)
        ret = map_pages(__pa, pa_to_hfn(__pa), 1,
                PAGE_HYPOS_RX);
    else
        ret = remove_maps(__pa, __pa + PAGE_SIZE);

    if (ret)
        BUG();
}
// --------------------------------------------------------------
extern void __switch_ttbr(unsigned long ttbr);
typedef void (switch_ttbr_cb)(unsigned long ttbr);

static void __bootfunc switch_ttbr(u64 ttbr)
{
    hva_t addr = va_to_pa(__switch_ttbr);
    switch_ttbr_cb *cb = (switch_ttbr_cb *)addr;
    ttbl_t pte;

    /* Enable the Identity Mapping in the Boot Page Tables
     */
    update_idmap(1);

    pte = pte_of_va((hva_t)__switch_ttbr);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    pte.ttbl.ap = 3;
    write_pte(&hypos_idmap3[PGTBL3_OFFSET(addr)], pte);

    /* Switch TTBR
     */
    cb(ttbr);

    /* Disable the Identity Mapping in the Runtime Page
     * Tables.
     */
    update_idmap(0);
}
// --------------------------------------------------------------
static void hypos_enforce_wnx(void)
{
    /* Enforce WXN policy, meaning that page tables should not
     * contain mapping that both writable and executable.
     */
    WRITE_SYSREG(READ_SYSREG(SCTLR_EL2) | SCTLR_Axx_ELx_WXN, SCTLR_EL2);
    isb();
    flush_tlb_local();
}

static void __bootfunc boot_idmap_setup(void)
{
    hpa_t idmap_spaddr = va_to_pa(__hypos_start);
    ttbl_t pte;
    TTBL_OFFSETS(idmap_offset, idmap_spaddr);

    get_globl()->boot_spaddr = idmap_spaddr;
    get_globl()->boot_epaddr = idmap_spaddr + code_size();

    zero_page(boot_idmap1);
    zero_page(boot_idmap2);
    zero_page(boot_idmap3);

    pte = hfn_to_entry(va_to_hfn(boot_idmap1), MT_NORMAL);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&boot_pgtbl0[idmap_offset[0]], pte);

    pte = hfn_to_entry(va_to_hfn(boot_idmap2), MT_NORMAL);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&boot_idmap1[idmap_offset[1]], pte);

    pte = hfn_to_entry(va_to_hfn(boot_idmap3), MT_NORMAL);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&boot_idmap2[idmap_offset[2]], pte);
}

// --------------------------------------------------------------

static void __bootfunc hypos_idmap_setup(void)
{
    hpa_t idmap_paddr = va_to_pa(__hypos_start);
    ttbl_t pte;
    TTBL_OFFSETS(idmap_offset, idmap_paddr);

    pte = pte_of_va((hva_t)hypos_idmap1);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&hypos_pgtbl0[idmap_offset[0]], pte);

    pte = pte_of_va((hva_t)hypos_idmap2);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&hypos_idmap1[idmap_offset[1]], pte);

    pte = pte_of_va((hva_t)hypos_idmap3);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&hypos_idmap2[idmap_offset[2]], pte);
}

static void prepare_idmap(void)
{
    boot_idmap_setup();
    hypos_idmap_setup();
}
// --------------------------------------------------------------
static void __bootfunc ttbl_consts(void)
{
    MSGI("[globl] Translation Table Constants   @_@\n"
         "        ---------------------------------\n"
         "        Level            Size       Shift\n"
         "            0     %8u GB        %4u\n"
         "            1     %8u GB        %4u\n"
         "            2     %8u MB        %4u\n"
         "            3     %8u KB        %4u\n"
         "        ---------------------------------\n",
         (unsigned int)(PGTBL_LEVEL_SIZE(0) / GB(1)),
         (unsigned int)PGTBL_LEVEL_SHIFT(0),
         (unsigned int)(PGTBL_LEVEL_SIZE(1) / GB(1)),
         (unsigned int)PGTBL_LEVEL_SHIFT(1),
         (unsigned int)(PGTBL_LEVEL_SIZE(2) / MB(1)),
         (unsigned int)PGTBL_LEVEL_SHIFT(2),
         (unsigned int)(PGTBL_LEVEL_SIZE(3) / KB(1)),
         (unsigned int)PGTBL_LEVEL_SHIFT(3));
}

/* Translation table setup
 */
int __bootfunc ttbl_setup(void)
{
    u64 ttbr;
    ttbl_t pte, *ptes;
    int i;

    prepare_idmap();

    pte = pte_of_va((addr_ptr)hypos_pgtbl1);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    hypos_pgtbl0[PGTBL0_OFFSET(HYPOS_VIRT_START)] = pte;

    ptes = (void *)hypos_pgtbl1;

    ptes[0] = pte_of_va((addr_ptr)hypos_pgtbl2);
    ptes[0].ttbl.table = 1;
    ptes[0].ttbl.uxn = 0;

    for (i = 0; i < HYPOS_DATA_NR_ENTRIES(3); i++) {
        hva_t va = HYPOS_VIRT_START + (i << PAGE_SHIFT);

        if (!is_core_section(va))
            break;

        pte = pte_of_va(va);
        pte.ttbl.table = 1;

        if (is_text_section(va) || is_boot_section(va)) {
            pte.ttbl.uxn = 0;
            pte.ttbl.ap = 3;
        }

        if (is_rodata_section(va))
            pte.ttbl.ap = 3;

        hypos_pgtbl3[i] = pte;
    }

    /* Initialize hypos 2nd-level page table entries
     * Reserved data memory region
     */
    for (i = 0; i < HYPOS_DATA_NR_ENTRIES(2); i++) {
        hva_t va = HYPOS_VIRT_START + (i << PGTBL_LEVEL_SHIFT(2));

        pte = pte_of_va((hva_t)(hypos_pgtbl3
                    + (i * PGTBL_TTBL_ENTRIES)));
        pte.ttbl.table = 1;
        hypos_pgtbl2[PGTBL2_OFFSET(va)] = pte;
    }

    /* Set up Fixmap
     */
    pte = pte_of_va((hva_t)hypos_fixmap);
    pte.ttbl.table = 1;
    hypos_pgtbl2[PGTBL2_OFFSET(HYPOS_FIXMAP_ADDR(0))] = pte;

    ttbl_consts();

    ttbr = (addr_ptr)hypos_pgtbl0 + hypos_get(phys_offset);

    hypos_get(arch.boot_ttbr) = READ_SYSREG(TTBR0_EL2);

    switch_ttbr(ttbr);

    hypos_get(arch.exec_ttbr) = READ_SYSREG(TTBR0_EL2);

    MSGH("Switch TTBR from %016lx to %016lx\n",
            hypos_get(arch.boot_ttbr),
            hypos_get(arch.exec_ttbr));

    hypos_enforce_wnx();

    return 0;
}
// --------------------------------------------------------------
