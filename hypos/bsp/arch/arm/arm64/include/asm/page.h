/**
 * Hustler's Project
 *
 * File:  page.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_PAGE_H
#define _ARCH_PAGE_H
// --------------------------------------------------------------
#include <asm-generic/bitops.h>
#include <asm/sysregs.h>

/*
 * Granularity | PAGE_SHIFT | LEVELS OF LOOKUP
 * -------------------------------------------
 * 4K          | 12         | 4
 * 16K         | 14         | 4
 * 64K         | 16         | 3
 */
#define PAGE_SHIFT          12
#define PAGE_SIZE           (_AC(1, L) << PAGE_SHIFT) /* 4KB */
#define PAGE_MASK           (~(PAGE_SIZE-1)) /* 0xFFFFFFFF FFFFF000 */
#define PAGE_OFFSET(ptr)    ((unsigned long)(ptr) & ~PAGE_MASK)
#define PAGE_ALIGN(x)       (((x) + PAGE_SIZE - 1) & PAGE_MASK)

#define PADDR_BITS          (48)
#define VADDR_BITS          (48)
#define PADDR_MASK          ((_AC(1,ULL) << PADDR_BITS) - 1)
#define VADDR_MASK          (~_AC(0,UL) >> (BITS_PER_LONG - VADDR_BITS))

#define STACK_ORDER         (3)
#define STACK_SIZE          (PAGE_SIZE << STACK_ORDER)

/* --------------------------------------------------------------
 * bit[0-2]                 - Attribute Index
 * bit[3]                   - Not Executable
 * bit[4-5]                 - Access Permission
 * bit[6]
 * bit[7]                   - Block
 * bit[8]                   - Contiguous
 */
#define _PAGE_XN_BIT        3
#define _PAGE_RO_BIT        4
#define _PAGE_XN            (1U << _PAGE_XN_BIT)
#define _PAGE_RO            (3U << _PAGE_RO_BIT)
#define PAGE_AI_MASK(x)     ((x) & 0x7U)
#define PAGE_XN_MASK(x)     (((x) >> _PAGE_XN_BIT) & 0x1U)
#define PAGE_RO_MASK(x)     (((x) >> _PAGE_RO_BIT) & 0x3U)

#define _PAGE_PRESENT       (1U << 5)
#define _PAGE_POPULATE      (1U << 6)

#define _PAGE_BLOCK_BIT     7
#define _PAGE_BLOCK         (1U << _PAGE_BLOCK_BIT)

#define _PAGE_CONTIG_BIT    8
#define _PAGE_CONTIG        (1U << _PAGE_CONTIG_BIT)

#define _PAGE_DEVICE        (_PAGE_XN|_PAGE_PRESENT)
#define _PAGE_NORMAL        (MT_NORMAL|_PAGE_PRESENT)

#define PAGE_HYPOS_RO       (_PAGE_NORMAL|_PAGE_RO|_PAGE_XN)
#define PAGE_HYPOS_RX       (_PAGE_NORMAL|_PAGE_RO)
#define PAGE_HYPOS_RW       (_PAGE_NORMAL|_PAGE_XN)

#define PAGE_HYPOS          PAGE_HYPOS_RW
#define PAGE_HYPOS_NOCACHE  (_PAGE_DEVICE|MT_DEVICE_nGnRE)
#define PAGE_HYPOS_WC       (_PAGE_DEVICE|MT_NORMAL_NC)

#define ROUND_PGUP(p)       (((p) + (PAGE_SIZE - 1)) & PAGE_MASK)
#define ROUND_PGDOWN(p)     ((p) & PAGE_MASK)
// --------------------------------------------------------------
#endif /* _ARCH_PAGE_H */
