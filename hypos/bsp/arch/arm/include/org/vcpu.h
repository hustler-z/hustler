/**
 * Hustler's Project
 *
 * File:  vcpu.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _ORG_VCPU_H
#define _ORG_VCPU_H
// --------------------------------------------------------------
#include <org/bitops.h>
#include <org/hypos.h>
#include <asm/vcpu.h>
#include <bsp/rcu.h>
#include <bsp/timer.h>
#include <bsp/sched.h>
#include <bsp/percpu.h>
#include <bsp/spinlock.h>

#define MAX_VCPU       (32)

struct vcpu_stat {
    int  stat;
    bool is_running;

    /* XXX */
};

struct vm_area {
    struct page *pg;
    void   *map;
};

struct vcpu_time_info {
    u32 version;
    u32 pad0;
    u64 tsc_timestamp;
    u64 system_time;
    u32 tsc_to_system_mul;
    s8  tsc_shift;
    u8  flags;
    u8  pad1[2];
};

typedef struct vcpu_time_info vcpu_time_info_t;

struct vcpu_info {
    vcpu_time_info_t time;

    /* TODO */
};

struct hypos;

struct vcpu {
    /* Virtual CPU ID */
    int vcpuid;

    /* Physical CPU ID */
    int hcpuid;

    /* XXX: For isolation purpose, provide solid and stable
     *      execution environment. As the design purposes,
     *      certain resources should be equally allocated.
     */
    struct hypos      *hypos;

    struct vm_area    area;
    struct timer      periodic_timer;

    struct vcpu       *next;
    struct vcpu_sched *sched;
    struct vcpu_stat  stat;
    struct arch_vcpu  arch;
};

struct shared_info {
    struct vcpu_info vcpu_info[MAX_VCPU];
    u32 wc_version;
    u32 wc_sec;
    u32 wc_nsec;
    u32 wc_sec_hi;
};

typedef struct shared_info shared_info_t;
#define __shared_info(h, s, field) ((s)->field)
#define shared_info(h, field) \
    __shared_info(h, (h)->shared_info, field)

// --------------------------------------------------------------

struct hypos_arch {
    u8 sve_vl;

    struct {
        u64     offset;
        stime_t nanoseconds;
    } vtimer_base;

    struct vgic_dist vgic;

    void *tee;
} __cacheline_aligned;

struct hypos {
    u8            hypos_id;

    unsigned int  max_vcpus;
    struct vcpu   **vcpu;
    shared_info_t *shared_info;
    spinlock_t    hypos_lock;

    struct {
        u64 seconds;
        bool set;
    } time_offset;

    struct hypos      *next;

    /* XXX: Architectural <hypos> Settings */
    struct hypos_arch arch;
};

// --------------------------------------------------------------

extern struct vcpu *hypos_vcpus[NR_CPUS];

#define is_idle_hypos(h)     ((h)->hypos_id == HYPOS_IDLE)
#define is_idle_vcpu(v)      (is_idle_hypos((v)->hypos))

DECLARE_PERCPU(struct vcpu *, current_vcpu);
#define current              (this_cpu(current_vcpu))
#define set_current(vcpu)    do { current = (vcpu); } while(0)
#define get_cpu_current(cpu) (percpu(current_vcpu, cpu))

extern struct hypos *hypos_list;

// --------------------------------------------------------------

void vcpu_wake(struct vcpu *v);
long vcpu_yield(void);
void vcpu_sleep_nosync(struct vcpu *v);
void vcpu_sleep_sync(struct vcpu *v);

// --------------------------------------------------------------

#define for_each_hypos(_h)                        \
for ((_h) = rcu_dereference(hypos_list);          \
     (_h) != NULL;                                \
     (_h) = rcu_dereference((_h)->next))          \

#define for_each_vcpu(_h, _v)                     \
for ((_v) = (_h)->vcpu ? (_h)->vcpu[0] : NULL;    \
     (_v) != NULL;                                \
     (_v) = (_v)->next)

// --------------------------------------------------------------
#endif /* _ORG_VCPU_H */
