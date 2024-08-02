/**
 * Hustler's Project
 *
 * File:  hypos.h
 * Date:  2024/07/13
 * Usage:
 */

#ifndef _ORG_HYPOS_H
#define _ORG_HYPOS_H
// --------------------------------------------------------------
#include <asm/xaddr.h>
#include <bsp/compiler.h>

#define map_hypos_page(hfn)         __hfn_to_va(hfn_get(hfn))
#define __map_hypos_page(pg)        page_to_va(pg)
#define unmap_hypos_page(ptr)       ((void)(ptr))

// --------------------------------------------------------------

#define HYPOS_FIRST_RESERVED (0x7FF0UL)
#define HYPOS_SELF           (0x7FF0UL)
#define HYPOS_IO             (0x7FF1UL)
#define HYPOS_RUN            (0x7FF2UL)
#define HYPOS_COW            (0x7FF3UL)
#define HYPOS_INVALID        (0x7FF4UL)
#define HYPOS_IDLE           (0x7FFFUL)
#define HYPOS_MASK           (0x7FFFUL)

// --------------------------------------------------------------
#endif /* _ORG_HYPOS_H */
