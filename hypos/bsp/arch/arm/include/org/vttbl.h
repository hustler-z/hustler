/**
 * Hustler's Project
 *
 * File:  vttbl.h
 * Date:  2024/07/24
 * Usage:
 */

#ifndef _ORG_VTTBL_H
#define _ORG_VTTBL_H
// ------------------------------------------------------------------------

#include <org/vcpu.h>

// ------------------------------------------------------------------------

/* XXX: Guest Memory Access Interfaces
 */
int access_guest_memory_by_gpa(struct hypos *h, gpa_t gpa,
                               void *buf, u32 size,
                               bool is_write);

// ------------------------------------------------------------------------
#endif /* _ORG_VTTBL_H */
