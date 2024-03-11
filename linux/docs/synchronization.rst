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

Data Memory Barrier is a memory barrier that ensures the ordering of
observations of memory accesses.

#define dmb(opt)   asm volatile("dmb " #opt : : : "memory")

Data Synchronization Barrier is a memory barrier that ensures the completion
of memory accesses. All pending loads and stores, cache maintenance
instructions, and all TLB maintenance instructions, are completed before
program execution continues.

#define dsb(opt)   asm volatile("dsb " #opt : : : "memory")

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
