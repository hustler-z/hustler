/**
 * Hustler's Project
 *
 * File:  vcore_sched.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _VCORE_SCHED_H
#define _VCORE_SCHED_H
// ------------------------------------------------------------------------
struct vcpu;

struct hyp_rq {
    /* TBD */
};

struct hyp_wq {
    /* TBD */
};

/* hypervisor scheduler information & algorithm
 */
struct vsched {
    int policy;
    /* Implement a simple schedule algorithm
     * no more, no less.
     */
    struct hyp_rq;
    struct hyp_wq;
};

struct vstat {
    int stat;
    /* TBD */
};

struct vcpu {
    /* virtual cpu ID */
    int vcpu_id;
    /* Physical cpu ID */
    int pcpu_id;

    struct vcpu   *vcpu_list;
    struct vsched *sched;
    struct vstat  stat;
};

// ------------------------------------------------------------------------
#endif /* _VCORE_SCHED_H */
