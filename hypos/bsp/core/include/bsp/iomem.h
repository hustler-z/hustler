/**
 * hustler's project
 *
 * file:  iomem.h
 * date:  2024/07/16
 * usage:
 */

#ifndef _BSP_IOMEM_H
#define _BSP_IOMEM_H
// --------------------------------------------------------------
#include <org/vcpu.h>

static inline int iomem_permit_access(struct hypos *h, unsigned long s,
                                      unsigned long e)
{
    return 0;
}

static inline int iomem_deny_access(struct hypos *h, unsigned long s,
                                    unsigned long e)
{
    return 0;
}

// --------------------------------------------------------------
#endif /* _BSP_IOMEM_H */
