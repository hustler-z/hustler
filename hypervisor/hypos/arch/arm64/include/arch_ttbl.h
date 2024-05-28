/**
 * Hustler's Project
 *
 * File:  arch_ttbl.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_TTBL_H
#define _ARCH_TTBL_H
// ---------------------------------------------------------

#include <common_ccattr.h>
#include <arch_pgtbl.h>

#ifndef __ASSEMBLY__

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
 * @ttbl_t - translation table entry
 */

typedef struct __packed {
    unsigned long valid:1;
    unsigned long table:1;

    /* lower attributes (10 bits)
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
     */
    unsigned long ai: 3;
    unsigned long ns: 1;
    unsigned long ap: 2;
    unsigned long sh: 2;
    unsigned long af: 1;
    unsigned long ng: 1;

    /* Base address of block or next table
     * [47:12]
     */
    unsigned long long base: 36;

    /* [51:48] reserved set as zeros
     */
    unsigned long res1: 4;

    /* Upper attributes (16 bits)
     * contig - contiguous
     * Execute attributes
     * uxn    - Unprivileged eXecute Never
     * pxn    - Privileged eXecute Never
     */
    unsigned long contig: 1;
    unsigned long pxn: 1;
    unsigned long uxn: 1;
    unsigned long res2: 9;
} ttbl_t;

#define SET_TTBL_ENTRIES(name, nr)                        \
ttbl_t __aligned(PAGE_SIZE) name[PGTBL_TTBL_ENTRIES * (nr)];

#endif /* __ASSEMBLY__ */

#define TTBL_ENTRY_DEF  0xF7F /* nG=1 AF=1 SH=11 AP=01 NS=1 AI=111 T=1 V=1 */
#define TTBL_ENTRY_MEM  0xF7D /* nG=1 AF=1 SH=11 AP=01 NS=1 AI=111 T=0 V=1 */

#define TTBL_ENTRY_DEV  0xE71 /* nG=1 AF=1 SH=10 AP=01 NS=1 AI=100 T=0 V=1 */
#define TTBL_ENTRY_DEM  0xE73 /* nG=1 AF=1 SH=10 AP=01 NS=1 AI=100 T=1 V=1 */

// ---------------------------------------------------------
#endif /* _ARCH_TTBL_H */
