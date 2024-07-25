/**
 * Hustler's Project
 *
 * File:  virq.h
 * Date:  2024/07/24
 * Usage:
 */

#ifndef _ORG_VIRQ_H
#define _ORG_VIRQ_H
// --------------------------------------------------------------
#include <org/vcpu.h>

bool irq_type_set_by_hypos(const struct hypos *h);
int route_irq_to_guest(struct hypos *h, unsigned int virq,
                       unsigned int irq, const char * devname);
int release_guest_irq(struct hypos *h, unsigned int virq);

// --------------------------------------------------------------
#endif /* _ORG_VIRQ_H */
