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
#include <asm-generic/section.h>
#include <generic/exit.h>
#include <bsp/debug.h>
#include <bsp/check.h>
#include <bsp/percpu.h>

// --------------------------------------------------------------
extern unsigned long paddr_offset;

/* Boot-time Page Table Setup
 * --------------------------------------------------------------
 * boot_pgtbl<n> - 4-level page table used during boot time.
 *
 * --------------------------------------------------------------
 */
SET_BOOT_TTBL(boot_pgtbl0, 1);
SET_BOOT_TTBL(boot_pgtbl1, 1);
SET_BOOT_TTBL(boot_pgtbl2, 1);
SET_BOOT_TTBL(boot_pgtbl3, HYPOS_DATA_NR_ENTRIES(2));
SET_BOOT_TTBL(hypos_fixmap, 1);

SET_BOOT_TTBL(boot_idmap1, 1);
SET_BOOT_TTBL(boot_idmap2, 1);
SET_BOOT_TTBL(boot_idmap3, 1);

/* HYPOS Page Table Settings
 */
SET_TTBL(hypos_idmap1, 1);
SET_TTBL(hypos_idmap2, 1);
SET_TTBL(hypos_idmap3, 1);

SET_TTBL(hypos_pgtbl0, 1);
DEFINE_PERCPU(ttbl_t *, hypos_pgtbl0);
static SET_TTBL(hypos_pgtbl1, 1);
static SET_TTBL(hypos_pgtbl2, 1);
static SET_TTBL(hypos_pgtbl3, HYPOS_DATA_NR_ENTRIES(2));

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
            .ns = 1,
            .ap = 1,
            .ng = 1,
            .contig = 0,
            .pxn = 1,
        }
    };

    switch (attr) {
    case MT_DEVICE_nGnRnE:
    case MT_DEVICE_nGnRE:
        pte.ttbl.sh = PTE_SHARE_OUTER;
        break;
    case MT_NORMAL_NC:
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
    paddr_t pa = va + paddr_offset;

    return pfn_to_entry(pa_to_pfn(pa), MT_NORMAL);
}

void update_idmap(unsigned int ok)
{
    paddr_t __pa = va_to_pa(__hypos_start);
    int ret;

    if (ok)
        ret = map_pages(__pa, pa_to_pfn(__pa), 1,
                PAGE_HYPOS_RX);
    else
        ret = remove_maps(__pa, __pa + PAGE_SIZE);

    if (ret) {
        MSGH("Now shit happened at %s()\n", __func__);
        hang();
    }
}

extern void __switch_ttbr(u64 ttbr);
typedef void (switch_ttbr_fn)(u64 ttbr);

void __bootfunc switch_ttbr(u64 ttbr)
{
    paddr_t __pa = va_to_pa(__switch_ttbr);
    switch_ttbr_fn *fn = (switch_ttbr_fn *)__pa;
    ttbl_t pte;

    DEBUG("%s() been Called.\n", __func__);

    update_idmap(1);

    pte = pte_of_va((vaddr_t)__switch_ttbr);
    pte.ttbl.table = 1;
    pte.ttbl.pxn = 0;
    pte.ttbl.ap = RO_ACCESS_B11;
    write_pte(&hypos_idmap3[PGTBL3_OFFSET(__pa)], pte);

    fn(ttbr);

    update_idmap(0);
}

static void __bootfunc boot_idmap_setup(void)
{
    paddr_t idmap_paddr = va_to_pa(__hypos_start);
    ttbl_t pte;
    TTBL_OFFSETS(idmap_offset, idmap_paddr);

    MSGH("Boot ID Mapping Setup\n");

    zero_page(boot_idmap1);
    zero_page(boot_idmap2);
    zero_page(boot_idmap3);

    pte = pfn_to_entry(va_to_pfn(boot_idmap1), MT_NORMAL);
    pte.ttbl.table = 1;
    pte.ttbl.pxn = 0;

    write_pte(&boot_pgtbl0[idmap_offset[0]], pte);

    pte = pfn_to_entry(va_to_pfn(boot_idmap2), MT_NORMAL);
    pte.ttbl.table = 1;
    pte.ttbl.pxn = 0;

    write_pte(&boot_idmap1[idmap_offset[1]], pte);

    pte = pfn_to_entry(va_to_pfn(boot_idmap3), MT_NORMAL);
    pte.ttbl.table = 1;
    pte.ttbl.pxn = 0;

    write_pte(&boot_idmap2[idmap_offset[2]], pte);
}

// --------------------------------------------------------------

static void __bootfunc hypos_idmap_setup(void)
{
    paddr_t idmap_paddr = va_to_pa(__hypos_start);
    ttbl_t pte;
    TTBL_OFFSETS(idmap_offset, idmap_paddr);

    MSGH("Hypos ID Mapping Setup\n");

    pte = pte_of_va((vaddr_t)hypos_idmap1);
    pte.ttbl.table = 1;
    pte.ttbl.pxn = 0;

    write_pte(&hypos_pgtbl0[idmap_offset[0]], pte);

    pte = pte_of_va((vaddr_t)hypos_idmap2);
    pte.ttbl.table = 1;
    pte.ttbl.pxn = 0;

    write_pte(&hypos_idmap1[idmap_offset[1]], pte);

    pte = pte_of_va((vaddr_t)hypos_idmap3);
    pte.ttbl.table = 1;
    pte.ttbl.pxn = 0;

    write_pte(&hypos_idmap2[idmap_offset[2]], pte);
}

// --------------------------------------------------------------
int __bootfunc ttbl_setup(void)
{
    u64 ttbr;
    ttbl_t pte, *ptes;
    int i;

    MSGH("Translation Table Setup\n");

    boot_idmap_setup();

    hypos_idmap_setup();

    pte = pte_of_va((uintptr_t)hypos_pgtbl1);
    pte.ttbl.table = 1;
    pte.ttbl.pxn = 0;
    hypos_pgtbl0[PGTBL0_OFFSET(HYPOS_VIRT_START)] = pte;

    ptes = (void *)hypos_pgtbl1;

    ptes[0] = pte_of_va((uintptr_t)hypos_idmap2);
    ptes[0].ttbl.table = 1;
    ptes[0].ttbl.pxn = 0;

    for (i = 0; i < HYPOS_DATA_NR_ENTRIES(3); i++) {
        vaddr_t va = HYPOS_VIRT_START + (i << PAGE_SHIFT);

        if (!is_core_section(va))
            break;

        pte = pte_of_va(va);
        pte.ttbl.table = 1;

        if (is_text_section(va) || is_boot_section(va)) {
            pte.ttbl.pxn = 1;
            pte.ttbl.ap = RO_ACCESS_B11;
        }

        if (is_rodata_section(va))
            pte.ttbl.ap = RO_ACCESS_B11;

        hypos_pgtbl3[i] = pte;
    }

    /* Initialize hypos 2nd-level page table entries
     */
    for (i = 0; i < HYPOS_DATA_NR_ENTRIES(2); i++) {
        vaddr_t va = HYPOS_VIRT_START + (i << PGTBL_LEVEL_SHIFT(2));

        pte = pte_of_va((vaddr_t)(hypos_pgtbl3 + (i * PGTBL_TTBL_ENTRIES)));
        pte.ttbl.table = 1;
        hypos_pgtbl2[PGTBL2_OFFSET(va)] = pte;
    }

    pte = pte_of_va((vaddr_t)hypos_fixmap);
    pte.ttbl.table = 1;
    hypos_pgtbl2[PGTBL2_OFFSET(HYPOS_FIXMAP_ADDR(0))] = pte;

    ttbr = (uintptr_t)hypos_pgtbl0 + paddr_offset;

    switch_ttbr(ttbr);

    /* Enforce WXN policy, meaning that page tables should not
     * contain mapping that both writable and executable.
     */
    WRITE_SYSREG(READ_SYSREG(SCTLR_EL2) | SCTLR_Axx_ELx_WXN, SCTLR_EL2);
    isb();
    flush_tlb_local();

    return 0;
}

// --------------------------------------------------------------
