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
#include <asm/atomic.h>
#include <org/bitops.h>
#include <org/hypos.h>
#include <asm/vcpu.h>
#include <bsp/rcu.h>
#include <bsp/timer.h>
#include <bsp/sched.h>
#include <bsp/percpu.h>
#include <bsp/spinlock.h>

#define MAX_VCPUS        (32)
#define INVALID_VCPU_ID  MAX_VCPUS

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

    unsigned long     pause_flags;
    atomic_t          pause_count;

    struct vm_area    area;
    struct timer      periodic_timer;

    struct vcpu       *next;
    struct vcpu_sched *sched;
    struct vcpu_stat  stat;
    struct arch_vcpu  arch;
};

// --------------------------------------------------------------

/*
 * Per-VCPU pause flags.
 */
#define _VPF_BLOCKED         0
#define VPF_BLOCKED          (1UL<<_VPF_BLOCKED)
 /* VCPU IS OFFLINE. */
#define _VPF_DOWN            1
#define VPF_DOWN             (1UL<<_VPF_DOWN)
 /* VCPU IS BLOCKED AWAITING AN EVENT TO BE CONSUMED BY XEN. */
#define _VPF_BLOCKED_IN_XEN  2
#define VPF_BLOCKED_IN_XEN   (1UL<<_VPF_BLOCKED_IN_XEN)
 /* VCPU AFFINITY HAS CHANGED: MIGRATING TO A NEW CPU. */
#define _VPF_MIGRATING       3
#define VPF_MIGRATING        (1UL<<_VPF_MIGRATING)
 /* VCPU IS BLOCKED DUE TO MISSING MEM_PAGING RING. */
#define _VPF_MEM_PAGING      4
#define VPF_MEM_PAGING       (1UL<<_VPF_MEM_PAGING)
 /* VCPU IS BLOCKED DUE TO MISSING MEM_ACCESS RING. */
#define _VPF_MEM_ACCESS      5
#define VPF_MEM_ACCESS       (1UL<<_VPF_MEM_ACCESS)
 /* VCPU IS BLOCKED DUE TO MISSING MEM_SHARING RING. */
#define _VPF_MEM_SHARING     6
#define VPF_MEM_SHARING      (1UL<<_VPF_MEM_SHARING)
 /* VCPU IS BEING RESET. */
#define _VPF_IN_RESET        7
#define VPF_IN_RESET         (1UL<<_VPF_IN_RESET)
/* VCPU IS PARKED. */
#define _VPF_PARKED          8
#define VPF_PARKED           (1UL<<_VPF_PARKED)

// --------------------------------------------------------------

struct shared_info {
    struct vcpu_info vcpu_info[MAX_VCPUS];
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

    unsigned int evtchn_irq;

    struct vgic_dist vgic;

    void *tee;
} __cacheline_aligned;

typedef u16     hid_t;

struct hypos {
    hid_t           hid;
    unsigned int    max_vcpus;
    struct vcpu     **vcpu;
    shared_info_t   *shared_info;
    rspinlock_t     hypos_lock;
    rspinlock_t     alloc_lock;
    rcu_read_lock_t rcu_lock;

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

#define is_idle_hypos(h)     ((h)->hid == HYPOS_IDLE)
#define is_idle_vcpu(v)      (is_idle_hypos((v)->hypos))

DECLARE_PERCPU(struct vcpu *, current_vcpu);
#define current              (this_cpu(current_vcpu))
#define set_current(vcpu)    do { current = (vcpu); } while(0)
#define get_cpu_current(cpu) (percpu(current_vcpu, cpu))

extern struct hypos *hypos_list;

extern struct hypos *hardware_hypos;

static inline bool is_hardware_hypos(const struct hypos *h)
{
    return !!(h == hardware_hypos);
}
// --------------------------------------------------------------

struct hypos *rcu_lock_hypos_by_id(hid_t hid);

static inline void rcu_unlock_hypos(struct hypos *h)
{
    if (h != current->hypos)
        rcu_read_unlock(&h->rcu_lock);
}

static inline struct hypos *rcu_lock_hypos(struct hypos *h)
{
    if (h != current->hypos)
        rcu_read_lock(&h->rcu_lock);
    return h;
}

// --------------------------------------------------------------

void vcpu_kick(struct vcpu *v);
void vcpu_wake(struct vcpu *v);
long vcpu_yield(void);
void vcpu_sleep_nosync(struct vcpu *v);
void vcpu_sleep_sync(struct vcpu *v);

static inline bool is_vcpu_online(const struct vcpu *v)
{
    return !test_bit(_VPF_DOWN, &v->pause_flags);
}

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
