/**
 * Hustler's Project
 *
 * File:  sched.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _HYP_SCHED_H
#define _HYP_SCHED_H
// ------------------------------------------------------------------------
#include <generic/type.h>
#include <asm/vcpu.h>

struct hyp_rq {
    struct hyp_rq *rq;
    /* TBD */
};

struct hyp_wq {
    struct hyp_wq *wq;
    /* TBD */
};

/* hypervisor scheduler information & algorithm
 */
struct vcpu_sched {
    int policy;
    /* Implement a simple schedule algorithm
     * no more, no less.
     */
    struct hyp_rq *rq;
    struct hyp_wq *wq;
};

struct vcpu_stat {
    int stat;
    /* TBD */
};

struct vcpu {
    /* virtual cpu ID */
    int vcpuid;
    /* Physical cpu ID */
    int hcpuid;

    struct vcpu       *vcpu_list;
    struct vcpu_sched *sched;
    struct vcpu_stat  stat;
    struct arch_vcpu  arch;
};

// ------------------------------------------------------------------------
#endif /* _HYP_SCHED_H */
