+------------------------------------------------------------------------------+
| LINUX KERNEL SYNCHRONIZATION                                                 |
+------------------------------------------------------------------------------+

Synchronization

1) local_irq_enable/disable()
              |
              +- raw_local_irq_enable/disable()
                            |
                            +- arch_local_irq_enable/disable()
                                           |
                                           :
                                           +- msr daifset, #<imm>
                                                              |
                                                        GIC_PRIO_IRQOFF

                                           +- msr daifclr, #<imm>
                                                              |
                                                        GIC_PRIO_IRQON

2) preempt_enable/disable()
              |
              +- barrier()

#define barrier() __asm__ __volatile__("": : :"memory")

--------------------------------------------------------------------------------
[+] arch/arm64/include/asm/barrier.h

WFE is often used inside lock implementations as it is an efficient way of
temporarily suspending the core while it waits for a lock to be released. In
legacy code, it is common to see a Send Event (SEV) instruction in unlock code
which serves to wake up any core which is waiting for that lock.

Send Event is a hint instruction. It causes an event to be signaled to all PEs
in the multiprocessor system.

#define sev()           asm volatile("sev" : : : "memory")

Wait For Event is a hint instruction that indicates that the PE can enter a
low-power state and remain there until a wakeup event occurs.

#define wfe()           asm volatile("wfe" : : : "memory")

Wait For Event with Timeout is a hint instruction that indicates that the PE
can enter a low-power state and remain there until either a local timeout event
or a wakeup event occurs. Wakeup events include the event signaled as a result
of executing the SEV instruction on any PE in the multiprocessor system.

#define wfet(val)       asm volatile("msr s0_3_c1_c0_0, %0"	\
				        : : "r" (val) : "memory")

--------------------------------------------------------------------------------
Wait For Interrupt is a hint instruction that indicates that the PE can enter a
low-power state and remain there until a wakeup event occurs.

#define wfi()      asm volatile("wfi" : : : "memory")

#define wfit(val)  asm volatile("msr s0_3_c1_c0_1, %0" : : "r" (val) : "memory")

Instruction Synchronization Barrier flushes the pipeline in the PE and is a
context synchronization event. i.e., it flushes the CPU pipeline and
prefetch buffers, causing instructions after the ISB to be fetched (or
re-fetched) from cache or memory.

#define isb()      asm volatile("isb" : : : "memory")

@Types of Observers

Instruction Fetch Unit (IFU)
The data interface, typically called the Load Store Unit (LSU)
The MMU table walk unit

Data Memory Barrier is a memory barrier that ensures the ordering of
observations of memory accesses.

#define dmb(opt)   asm volatile("dmb " #opt : : : "memory")

e.g.

STR #1,[X1]
DMB
STR #1,[X3] => Cannot observe this STR without first observing the previous STR.

Data Synchronization Barrier is a memory barrier that ensures the completion
of memory accesses. All pending loads and stores, cache maintenance
instructions, and all TLB maintenance instructions, are completed before
program execution continues.

#define dsb(opt)   asm volatile("dsb " #opt : : : "memory")

e.g.

STR X0,[X1]  => Must complete before the DSB can retire.
DSB
ADD X1,X2,X3 => Must NOT be executed before the first STR completes.
STR X4,[X5]  => Must NOT be executed until the first STR completes.

The DSB stalls execution until the STR to Device-nGnRnE memory receives a write
response from the end peripheral at the corresponding memory location

e.g.

STR X0,[Device-nGnRnE] => Must receive a write response from the end-peripheral
DSB SY

LDAR => Load-Acquire Instructions
STLR => Store-Release Instructions

        LDR and STR ------+
                          |
                          |
----------- LDAR ---------|--- Accesses can cross a barrier in one
     ▲                    |    direction but not the other.
     |  LDR and STR       ▼
     |       |
     +-------+


        LDR and STR ------+
     ▲                    |
     |                    ▼
-----|----- STLR ------------- Accesses can cross a barrier in one
     |                         direction but not the other.
     |  LDR and STR
     |       |
     +-------+


When the stage 1 MMU is disabled:

• All data accesses are Device-nGnRnE.
• All instruction fetches are treated as either non-cacheable or cacheable,
according to the value of the SCTLR_ELx.I (instruction cacheability control)
field.
• All addresses have read/write access and are executable.

@Normal Memory
@Device Memory

The Device memory type is used for describing peripherals. Peripheral registers
are often referred to as Memory-Mapped I/O (MMIO).

• Device-GRE
• Device-nGRE
• Device-nGnRE

@Gathering (G, nG)
@Re-ordering (R, nR)
@Early Write Acknowledgement (E, nE)

--------------------------------------------------------------------------------

#define smp_mb()   do { kcsan_mb(); __smp_mb(); } while (0)
                                        |
                                        +- dmb(ish)
                                                |
                                                +- Inner Shareable

--------------------------------------------------------------------------------
- SPINLOCK -

spin_lock()
    |
    +- raw_spin_lock()
             |
             +- _raw_spin_lock()

--------------------------------------------------------------------------------
RCU (Read-Copy Update) is a synchronization mechanism.




--------------------------------------------------------------------------------
- FUTEX -

The futex() system call provides a method for waiting until a certain condition
becomes true.

futex() @syscall
  :
  +- do_futex() [+] kernel/futex/syscalls.c
         :cmd
*--------------------------------------------------------------------*
| FUTEX_WAIT_BITSET/FUTEX_WAIT               futex_wait()            |
| FUTEX_WAKE_BITSET/FUTEX_WAKE               futex_wake()            |
| FUTEX_REQUEUE                              futex_requeue()         |
| FUTEX_CMP_REQUEUE                          futex_requeue()         |
| FUTEX_WAKE_OP                              futex_wake_op()         |
| FUTEX_LOCK_PI/FUTEX_LOCK_PI2               futex_lock_pi()         |--------
| FUTEX_UNLOCK_PI                            futex_unlock_pi()       |PI futex
| FUTEX_TRYLOCK_PI                           futex_lock_pi()         |--------
| FUTEX_WAIT_REQUEUE_PI                      futex_wait_requeue_pi() |
| FUTEX_CMP_REQUEUE_PI                       futex_requeue()         |
*--------------------------------------------------------------------*

The waiter reads the futex value in user space and calls futex_wait(). This
function computes the hash bucket and acquires the hash bucket lock. After
that it reads the futex user space value again and verifies that the data
has not changed. If it has not changed it enqueues itself into the hash
bucket, releases the hash bucket lock and schedules.

The waker side modifies the user space value of the futex and calls
futex_wake(). This function computes the hash bucket and acquires the hash
bucket lock. Then it looks for waiters on that futex in the hash bucket and
wakes them.

futex_wake()
futex_wait()

Priority-Inheritance Futexes

Priority inversion is the problem that occurs when a high-priority task is
blocked waiting to acquire a lock held by a low-priority task, while tasks
at an intermediate priority continuously preempt the low-priority task from
the CPU.  Consequently, the low-priority task makes no progress toward
releasing the lock, and the high-priority task remains blocked.

futex_lock_pi()
      :
      +- futex_setup_timer()
      |
      :
      +- __futex_queue(&q, hb)
                |
                ▼
(struct futex_q) q              => The hashed futex queue entry,
                                   one per waiting task.

(struct futex_hash_bucket *) hb => Hash buckets are shared by all
                                   the futex_keys that hash to the
                                   same location.  Each key may have
                                   multiple futex_q structures, one
                                   for each task waiting on a futex.

futex_unlock_pi()

--------------------------------------------------------------------------------
- RT MUTEX -

@struct rt_mutex => To avoid priority inversion, temporarily boost the priority
                    of low-priority process held the mutex to the priority of
                    high-priority process blocks on it.

rt_mutex_lock()
      |
      +- __rt_mutex_lock_common()

rt_mutex_unlock()

--------------------------------------------------------------------------------
- ROBUST FUTEX -

get_robust_list(), set_robust_list() => get/set list of robust futexes

Per-thread robust futex lists are managed in user space: the kernel knows only
about the location of the head of the list. A thread can inform the kernel of
the location of its robust futex list using set_robust_list(). The address of
a thread's robust futex list can be obtained using get_robust_list().

In the common case, at do_exit() time, there is no list registered, so the cost
of robust futexes is just a simple current->robust_list != NULL comparison. If
the thread has registered a list, then normally the list is empty. If the thread
/process crashed or terminated in some incorrect way then the list might be
non-empty: in this case the kernel carefully walks the list [not trusting it],
and marks all locks that are owned by this thread with the FUTEX_OWNER_DIED bit,
and wakes up one waiter (if any).

The list is guaranteed to be private and per-thread at do_exit() time, so it can
be accessed by the kernel in a lockless way.

--------------------------------------------------------------------------------
- MUTEX -

struct mutex

mutex_lock() => Lock the mutex exclusively for this task. If the mutex is not
    :           available right now, it will sleep until it can get it.
    |
    +- (a) __mutex_trylock_fast()
       (b) __mutex_lock_slowpath()
             |
             +- __mutex_lock()
                  |
                  +- __mutex_lock_common()
                        :
                        +- mutex_acquire_nest()
                        :
                        +- if __mutex_trylock() or mutex_optimistic_spin()
                                |
                                : (failed)
                                |
                                +- raw_spin_lock(&lock->wait_lock)
                                   then __mutex_trylock() again.
                                     |
                                     : (failed)
                                     |               no
                                     +- use_ww_ctx ------▶ __mutex_add_waiter()
                                           |yes
                                           +- __ww_mutex_add_waiter()

                    *---▶| [infinite loop]
                    ▲    :
                    |    +- __mutex_waiter_is_first()
                    |       => Check if current is the first waiter
                    |    :
                    |    +- __mutex_trylock_or_handoff() ------------------*
                    |    :                                                 |
                    |    +- if current is the first waiter then            |
                    |       mutex_optimistic_spin()      ------------------*
                    |    :                                                 |
                    *◀---|                                              (break)

mutex_optimistic_spin() => Optimistic spinning

We try to spin for acquisition when we find that the lock owner is currently
running on a (different) CPU and while we don't need to reschedule. The
rationale is that if the lock owner is running, it is likely to release the
lock soon.

The mutex spinners are queued up using MCS lock so that only one spinner can
compete for the mutex. However, if mutex spinning isn't going to happen, there
is no point in going through the lock/unlock overhead.

Returns true when the lock was taken, otherwise false, indicating that we need
to jump to the slowpath and sleep. The waiter flag is set to true if the spinner
is a waiter in the wait queue. The waiter-spinner will spin on the lock directly
and concurrently with the spinner at the head of the OSQ, if present, until the
owner is changed to itself.

mutex_unlock()

--------------------------------------------------------------------------------
- RCU -

(a) Remove pointers to a data structure, so that subsequent readers cannot gain
    a reference to it.

(b) Wait for all previous readers to complete their RCU read-side critical
    sections.

(c) At this point, there cannot be any readers who hold references  to the data
    structure, so it now may safely be reclaimed (e.g., kfree()d).

        rcu_assign_pointer()
                                +--------+
            *------------------▶| reader |---------+
            |                   +--------+         |
            |                       |              |
            |                       |              | Protect:
            |                       |              | rcu_read_lock()
            |                       |              | rcu_read_unlock()
            |    rcu_dereference()  |              |
        +---------+                 |              |
        | updater |◀----------------+              |
        +---------+                                ▼
            |                                +-----------+
            *-------------------------------▶| reclaimer |
                                             +-----------+
        Defer:
        synchronize_rcu() & call_rcu()

-   Use rcu_read_lock() and rcu_read_unlock() to guard RCU read-side critical
    sections.

-   Within an RCU read-side critical section, use rcu_dereference() to
    dereference RCU-protected pointers.

-   Use some solid scheme (such as locks or semaphores) to keep concurrent
    updates from interfering with each other.

-   Use rcu_assign_pointer() to update an RCU-protected pointer. This primitive
    protects concurrent readers from the updater, **not** concurrent updates
    from each other!  You therefore still need to use locking (or something
    similar) to keep concurrent rcu_assign_pointer() primitives from interfering
    with each other.

-   Use synchronize_rcu() **after** removing a data element from an
    RCU-protected data structure, but **before** reclaiming/freeing the data
    element, in order to wait for the completion of all RCU read-side critical
    sections that might be referencing that data item.

--------------------------------------------------------------------------------

membarrier() => issue memory barriers on a set of threads.

All memory accesses performed in program order from each targeted thread
is guaranteed to be ordered with respect to sys_membarrier(). If we use
the semantic "barrier()" to represent a compiler barrier forcing memory
accesses to be performed in program order across the barrier, and
smp_mb() to represent explicit memory barriers forcing full memory
ordering across the barrier.

--------------------------------------------------------------------------------
