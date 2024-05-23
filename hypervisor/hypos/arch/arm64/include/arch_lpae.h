/**
 * Hustler's Project
 *
 * File:  arch_lpae.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_LPAE_H
#define _ARCH_LPAE_H
// ---------------------------------------------------------
#include <arch_page.h>
#include <arch_bitops.h>

typedef struct __packed {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;      /* Valid mapping */
    unsigned long table:1;      /* == 1 in 4k map entries too */

    /*
     * These ten bits are only used in Block entries and are ignored
     * in Table entries.
     */
    unsigned long ai:3;         /* Attribute Index */
    unsigned long ns:1;         /* Not-Secure */
    unsigned long up:1;         /* Unpriviledged access */
    unsigned long ro:1;         /* Read-Only */
    unsigned long sh:2;         /* Shareability */
    unsigned long af:1;         /* Access Flag */
    unsigned long ng:1;         /* Not-Global */

    /* The base address must be appropriately aligned for Block entries */
    unsigned long long base:36; /* Base address of block or next table */
    unsigned long sbz:4;        /* Must be zero */

    /*
     * These seven bits are only used in Block entries and are ignored
     * in Table entries.
     */
    unsigned long contig:1;     /* In a block of 16 contiguous entries */
    unsigned long pxn:1;        /* Privileged-XN */
    unsigned long xn:1;         /* eXecute-Never */
    unsigned long avail:4;      /* Ignored by hardware */

    /*
     * These 5 bits are only used in Table entries and are ignored in
     * Block entries.
     */
    unsigned long pxnt:1;       /* Privileged-XN */
    unsigned long xnt:1;        /* eXecute-Never */
    unsigned long apt:2;        /* Access Permissions */
    unsigned long nst:1;        /* Not-Secure */
} lpae_pt_t;

typedef struct __packed {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;      /* Valid mapping */
    unsigned long table:1;      /* == 1 in 4k map entries too */

    unsigned long pad2:10;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long long base:36; /* Base address of block or next table */

    unsigned long pad1:16;
} lpae_walk_t;

typedef union {
    unsigned long bits;
    lpae_pt_t pt;
    lpae_walk_t walk;
} lpae_t;

/*
 * Granularity | PAGE_SHIFT | LPAE_SHIFT
 * -------------------------------------
 * 4K          | 12         | 9
 * 16K         | 14         | 11
 * 64K         | 16         | 13
 */
#define LPAE_SHIFT(n)               ((n) - 3)
#define LPAE_ENTRIES(n)             (_AC(1, U) << LPAE_SHIFT(n))
#define ARCH_BOOT_LPAE_ENTRIES     LPAE_ENTRIES(PAGE_SHIFT)

/* Use in C after BSS is zeroed.
 */
#define SET_PGTBL(name, nr)                                 \
lpae_t __aligned(PAGE_SIZE) name[ARCH_BOOT_LPAE_ENTRIES * (nr)]

// ---------------------------------------------------------
#endif /* _ARCH_LPAE_H */
