/**
 * Hustler's Project
 *
 * File:  ttbl.c
 * Date:  2024/05/20
 * Usage: exception handler implementation in C
 */

#include <asm/map.h>
#include <asm/sysregs.h>
#include <asm/exit.h>
#include <asm/cache.h>
#include <asm/vpa.h>
#include <asm-generic/section.h>
#include <asm-generic/globl.h>
#include <common/exit.h>
#include <bsp/debug.h>
#include <bsp/check.h>
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
void __dump_ttbl_walk(paddr_t ttbr, paddr_t addr,
                      unsigned int root_level,
                      unsigned int nr_root_tables)
{
    const pfn_t root_pfn = pa_to_pfn(ttbr);
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

    mapping = map_table(pfn_add(root_pfn, root_table));

    for (level = root_level; ; level++) {
        if (offsets[level] > PGTBL_TTBL_ENTRIES)
            break;

        pte = mapping[offsets[level]];

        MSGH("<%s> [0x%03x]=0x%016lx\n", lvlstr[level],
                offsets[level], pte.bits);

        if (level == 3 || !pte.walk.valid || !pte.walk.table)
            break;

        unmap_table(mapping);
        mapping = map_table(pte_get_pfn(pte));
    }

    unmap_table(mapping);
}

void dump_ttbl_walk(vaddr_t va)
{
    paddr_t ttbr = READ_SYSREG(TTBR0_EL2);

    MSGH("Walk TTBL VA 0x%016lx on CPU%d via TTBR 0x%016lx\n",
            va, smp_processor_id(), ttbr);

    __dump_ttbl_walk(ttbr, va, 0, 1);
}
// --------------------------------------------------------------

int mmu_enabled(void)
{
    return READ_SYSREG(SCTLR_EL2) & SCTLR_Axx_ELx_M;
}

ttbl_t pfn_to_entry(pfn_t pfn, unsigned int attr)
{
    ttbl_t pte = {
        .ttbl = {
            .valid = 1,
            .table = 0,
            .ai = attr,
            .af = 1,
            .ns = 1,
            .ap = 0,  /* 0b00, 0b01 - Read & Write */
            .ng = 1,
            .contig = 0,
            .uxn = 1, /* No executable in user space */
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

    ASSERT(!(pfn_to_pa(pfn) & ~PADDR_MASK));

    pte_set_pfn(pte, pfn);

    return pte;
}

ttbl_t __bootfunc pte_of_va(vaddr_t va)
{
    paddr_t pa = va + get_globl()->phys_offset;

    return pfn_to_entry(pa_to_pfn(pa), MT_NORMAL);
}

/* Enable / Disable Identity Mapping
 */
void update_idmap(unsigned int ok)
{
    paddr_t __pa = va_to_pa(__hypos_start);
    int ret;

    if (ok)
        ret = map_pages(__pa, pa_to_pfn(__pa), 1,
                PAGE_HYPOS_RX);
    else
        ret = remove_maps(__pa, __pa + PAGE_SIZE);

    if (ret)
        BUG();
}
// --------------------------------------------------------------
static void __switch_ttbr(u64 ttbr) {
    u64 sctlr;

    dsb(ish);
    isb();

    sctlr = READ_SYSREG(SCTLR_EL2);
    sctlr |= SCTLR_Axx_ELx_M;
    WRITE_SYSREG(sctlr, SCTLR_EL2);

    flush_tlb_local();

    WRITE_SYSREG(ttbr, TTBR0_EL2);
    isb();

    icache_clear(iallu);

    sctlr = READ_SYSREG(SCTLR_EL2);
    sctlr |= SCTLR_Axx_ELx_M;
    WRITE_SYSREG(sctlr, SCTLR_EL2);

    isb();
}

static void __bootfunc switch_ttbr(u64 ttbr)
{
    /* Enable the Identity Mapping in the Boot Page Tables
     */
    update_idmap(1);

    /* Switch TTBR
     */
    __switch_ttbr(ttbr);

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
    paddr_t idmap_paddr = va_to_pa(__hypos_start);
    ttbl_t pte;
    TTBL_OFFSETS(idmap_offset, idmap_paddr);

    MSGH("<HEAD> [VA] 0x%016lx -> [PA] 0x%016lx\n",
            (vaddr_t)__hypos_start,
            (paddr_t)idmap_paddr);

    zero_page(boot_idmap1);
    zero_page(boot_idmap2);
    zero_page(boot_idmap3);

    pte = pfn_to_entry(va_to_pfn(boot_idmap1), MT_NORMAL);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&boot_pgtbl0[idmap_offset[0]], pte);

    pte = pfn_to_entry(va_to_pfn(boot_idmap2), MT_NORMAL);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&boot_idmap1[idmap_offset[1]], pte);

    pte = pfn_to_entry(va_to_pfn(boot_idmap3), MT_NORMAL);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&boot_idmap2[idmap_offset[2]], pte);
}

// --------------------------------------------------------------

static void __bootfunc hypos_idmap_setup(void)
{
    paddr_t idmap_paddr = va_to_pa(__hypos_start);
    ttbl_t pte;
    TTBL_OFFSETS(idmap_offset, idmap_paddr);

    pte = pte_of_va((vaddr_t)hypos_idmap1);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&hypos_pgtbl0[idmap_offset[0]], pte);

    pte = pte_of_va((vaddr_t)hypos_idmap2);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    write_pte(&hypos_idmap1[idmap_offset[1]], pte);

    pte = pte_of_va((vaddr_t)hypos_idmap3);
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

/* Translation table setup
 */
int __bootfunc ttbl_setup(void)
{
    u64 ttbr;
    ttbl_t pte, *ptes;
    int i;

    prepare_idmap();

    pte = pte_of_va((ap_t)hypos_pgtbl1);
    pte.ttbl.table = 1;
    pte.ttbl.uxn = 0;
    hypos_pgtbl0[PGTBL0_OFFSET(HYPOS_VIRT_START)] = pte;

    ptes = (void *)hypos_pgtbl1;

    ptes[0] = pte_of_va((ap_t)hypos_pgtbl2);
    ptes[0].ttbl.table = 1;
    ptes[0].ttbl.uxn = 0;

    for (i = 0; i < HYPOS_DATA_NR_ENTRIES(3); i++) {
        vaddr_t va = HYPOS_VIRT_START + (i << PAGE_SHIFT);

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
        vaddr_t va = HYPOS_VIRT_START + (i << PGTBL_LEVEL_SHIFT(2));

        pte = pte_of_va((vaddr_t)(hypos_pgtbl3
                    + (i * PGTBL_TTBL_ENTRIES)));
        pte.ttbl.table = 1;
        hypos_pgtbl2[PGTBL2_OFFSET(va)] = pte;
    }

    /* Set up Fixmap
     */
    pte = pte_of_va((vaddr_t)hypos_fixmap);
    pte.ttbl.table = 1;
    hypos_pgtbl2[PGTBL2_OFFSET(HYPOS_FIXMAP_ADDR(0))] = pte;

    ttbr = (ap_t)hypos_pgtbl0 + get_globl()->phys_offset;

    MSGH("<TTBR> Before Switch TTBR - 0x%016lx to 0x%016lx\n",
            READ_SYSREG(TTBR0_EL2),
            (unsigned long)ttbr);

    switch_ttbr(ttbr);

    MSGH("<TTBR> After Switch TTBR  - 0x%016lx\n",
            READ_SYSREG(TTBR0_EL2));

    hypos_enforce_wnx();

    DEBUG("<TTBR> <%s> Finished\n", __func__);

    return 0;
}

// --------------------------------------------------------------
