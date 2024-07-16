/**
 * Hustler's Project
 *
 * File:  sched.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _BSP_SCHED_H
#define _BSP_SCHED_H
// ------------------------------------------------------------------------
#include <bsp/type.h>
#include <org/smp.h>

struct vcpu_rq {
    struct vcpu_rq *rq;
    /* TBD */
};

struct vcpu_wq {
    struct vcpu_wq *wq;
    /* TBD */
};

/* vcpu scheduler information & algorithm
 * for a single vcpu schedule entity.
 */
struct vcpu_sched {
    int policy;

    struct vcpu    *vcpu_list;
    void           *priv;

    /* Implement a simple schedule algorithm
     * no more, no less.
     */
    struct vcpu_rq *rq;
    struct vcpu_wq *wq;

    void (*sleep)(struct vcpu *v, struct vcpu_sched *sched);
    void (*wake)(struct vcpu *v, struct vcpu_sched *sched);
    void (*yield)(struct vcpu *v, struct vcpu_sched *sched);
    void (*do_schedule)(struct vcpu *v, struct vcpu_sched *sched);
};

void vcpu_sched_setup(void);

// ------------------------------------------------------------------------
#endif /* _BSP_SCHED_H */
