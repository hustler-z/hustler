/**
 * Hustler's Project
 *
 * File:  tlb.c
 * Date:  2024/06/19
 * Usage: translation buffer cache related implementation
 */

#include <asm/tlb.h>
#include <asm/page.h>
#include <asm/barrier.h>

void flush_tlb_range_va_local(vaddr_t va, unsigned long size)
{
    vaddr_t end = va + size;

    dsb(nshst);
    while (va < end) {
        __flush_tlb_one_local(va);
        va += PAGE_SIZE;
    }
    dsb(nsh);
    isb();
}

void flush_tlb_range_va(vaddr_t va, unsigned long size)
{
    vaddr_t end = va + size;

    dsb(ishst);
    while (va < end) {
        __flush_tlb_one(va);
        va += PAGE_SIZE;
    }
    dsb(ish);
    isb();
}
