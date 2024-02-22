+--------------------------------------------------------------------------------------+
| LINUX KERNEL SCHEDULER                                                               |
+--------------------------------------------------------------------------------------+

$ cat /proc/<pid>/schedstat

a) time spent on the cpu (in nanoseconds)
b) time spent waiting on a runqueue (in nanoseconds)
c) # of timeslices run on this cpu

----------------------------------------------------------------------------------------
- Deadline Task Scheduling -

The SCHED_DEADLINE policy contained inside the sched_dl scheduling class is basically an
implementation of the Earliest Deadline First (EDF) scheduling algorithm, augmented with
a mechanism (called Constant Bandwidth Server, CBS) that makes it possible to isolate the
behavior of tasks between each other.

SCHED_DEADLINE uses three parameters, named "runtime", "period", and "deadline", to
schedule tasks. A SCHED_DEADLINE task should receive "runtime" microseconds of execution
time every "period" microseconds, and these "runtime" microseconds are available within
"deadline" microseconds from the beginning of the period.

Summing up, the CBS algorithm assigns scheduling deadlines to tasks so that each
task runs for at most its runtime every period, avoiding any interference between
different tasks (bandwidth isolation), while the EDF algorithm selects the task with
the earliest scheduling deadline as the one to be executed next.

When a SCHED_DEADLINE task wakes up (becomes ready for execution), the scheduler checks
if:

	+------------------------------------------------+
	|	 remaining runtime              runtime  |
	| ---------------------------------- > --------- |
	| scheduling deadline - current time     period  |
	+------------------------------------------------+

if scheduling_deadline < current_time
     |yes
     +-> scheduling deadline = current time + deadline remaining runtime = runtime

When a SCHED_DEADLINE task executes for an amount of time t, its remaining runtime is
decreased as:

	+-------------------------------------------+
	| remaining runtime = remaining runtime - t |
	+-------------------------------------------+

When the remaining runtime becomes less or equal than 0, the task is said to be
"throttled" (also known as "depleted" in real-time literature) and cannot be scheduled
until its scheduling deadline.

The "replenishment time" for this task is set to be equal to the current
value of the scheduling deadline; When the current time is equal to the replenishment
time of a throttled task, the scheduling deadline and the remaining runtime are updated
as:

	+----------------------------------------------------+
	| scheduling deadline = scheduling deadline + period |
	| remaining runtime = remaining runtime + runtime    |
	+----------------------------------------------------+

----------------------------------------------------------------------------------------
- CFS (Completely Fair Scheduler) Scheduler -

struct task_struct {
	...

	int				on_rq;

	int				prio;
	int				static_prio;
	int				normal_prio;
	unsigned int			rt_priority;

	struct sched_entity		se;
	struct sched_rt_entity		rt;
	struct sched_dl_entity		dl;
	const struct sched_class	*sched_class;

#ifdef CONFIG_SCHED_CORE
	struct rb_node			core_node;
	unsigned long			core_cookie;
	unsigned int			core_occupation;
#endif
	...
}

CFS's task picking logic is based on this p->se.vruntime value and it is thus very simple:
it always tries to run the task with the smallest p->se.vruntime value (i.e., the task
which executed least so far). CFS always tries to split up CPU time between runnable tasks
as close to "ideal multitasking hardware" as possible.

Ideal Multitasking Hardware - at any time all tasks would have the same p->se.vruntime
value (i.e., tasks would execute simultaneously and no task would ever get "out of
balance" from the "ideal" share of CPU time.)

CFS also maintains the rq->cfs.min_vruntime value, which is a monotonic increasing value
tracking the smallest vruntime among all tasks in the runqueue. The total amount of work
done by the system is tracked using min_vruntime.

CFS maintains a time-ordered rbtree, where all runnable tasks are sorted by the
p->se.vruntime key.

                                   ●
                                  / \
                                 ○   ○
                                / \   \
                               ●   ●
                              /
                      (leftmost task)

Summing up, CFS works like this: it runs a task a bit, and when the task schedules
(or a scheduler tick happens) the task's CPU usage is "accounted for": the (small) time
it just spent using the physical CPU is added to p->se.vruntime. Once p->se.vruntime
gets high enough so that another task becomes the "leftmost task" of the time-ordered
rbtree it maintains (plus a small amount of "granularity" distance relative to the
leftmost task so that we do not over-schedule tasks and trash the cache), then the new
leftmost task is picked and the current task is preempted.

CFS implements three scheduling policies:
a) SCHED_NORMAL
b) SCHED_BATCH
c) SCHED_IDLE

[+] kernel/sched/fair.c implements the CFS scheduler

----------------------------------------------------------------------------------------


[+] kernel/sched/rt.c implements SCHED_FIFO and SCHED_RR semantics

----------------------------------------------------------------------------------------
- Capacity Aware Scheduling -

Consider that homogeneous SMP (identical cpus) vs heterogeneous (different cpus) platforms.

CPU capacity is a measure of the performance a CPU can reach.

=> capacity(cpu) = work_per_hz(cpu) * max_freq(cpu)

original (maximum attainabl) capacity
                |
                +- arch_scale_cpu_capacity()

@capacity fitness

The main capacity scheduling criterion of CFS

=> task_util(p) < capacity(task_cpu(p))

----------------------------------------------------------------------------------------
- Energy Aware Scheduling -

Energy Aware Scheduling (or EAS) gives the scheduler the ability to predict the impact
of its decisions on the energy consumed by CPUs. EAS relies on an Energy Model (EM) of
the CPUs to select an energy efficient CPU for each task, with a minimal impact on
throughput.

EAS operates only on heterogeneous CPU topologies (such as Arm big.LITTLE) because this
is where the potential for saving energy through scheduling is the highest.

----------------------------------------------------------------------------------------
[+] kernel/sched/core.c

schedule()
    |                       +-----------> tif_need_resched() checks thread info flags.
    :                       |
    +- __schedule() if need_resched()
            |
            +- get the rq from current cpu with cpu_rq()
               note rq is the main, per-CPU runqueue
            :
            +- schedule_debug()
            :
            +- check the sched_mode and (struct task_struct *)prev->__state
                |ok
                +- signal_pending_state() -> WRITE_ONCE(prev->__state, TASK_RUNNING)
                            |no
                            :
                            +- deactivate_task()
                                        |
                                        +- dequeue_task()
                                                |
                                                v
                        (struct task_struct *)p->sched_class->dequeue_task()


                            pick_next_task() from the rq
                                  |
            :                     v    no
            +- check if prev != next -----> __balance_callbacks()
                        |ok
                        :
                        +- RCU_INIT_POINTER(rq->curr, next)
                        :
                        +- migrate_disable_switch()
                        :
                        +- psi_sched_switch()
                        :
                        +- context_switch()
                           switch to the new MM and the new thread's register state
                                  |
                                  +- prepare_task_switch() called when the rq lock
                                     held and interrupts off
                                  |
                                  +- arch_start_context_switch()
                                  |
                                  v
                        +---------------------------------------------+
                        | kernel -> kernel   lazy + transfer active   |
                        | user   -> kernel   lazy + mmgrab() active   |
                        | kernel -> user     switch + mmdrop() active |
                        | user   -> user     switch                   |
                        +---------+-----------------------------------+
                                  |
                                  v
                                  +- if !next->mm
                                          |yes => to kernel
                                          +- enter_lazy_tlb()
                                                   |
                                                   +- update_saved_ttbr0()

                                          :                no
                                          +- if prev->mm -----> prev->active_mm = NULL
                                                   |yes
                                                   +- mmgrab()
                                                      Pin the mm_struct, ensure it won't
                                                      get freed even after the owning
                                                      task exits. No guarantee that the
                                                      associated address space will still
                                                      exist later on and mmget_not_zero()
                                                      has to be used before accessing it.

                                          |no => to user
                                          +- membarrier_switch_mm()
                                          |
                                          +- switch_mm_irq_off()
                                                      |
                                                      +- switch_mm()
                                                              |
                                                              +- if prev != next
                                                                         |yes
                                                                         +- __switch_mm()
                                                                                   |
                                                                                   :
                                                              |
                                                              +- update_saved_ttbr0()
                                          |
                                          +- lru_gen_use_mm()
                                          |                 no
                                          +- if !prev->mm -----> nop
                                                 |yes
                                                 +- rq->prev_mm = prev->active_mm
                                                    prev->active_mm = NULL

                                  :
                                  +- prepare_lock_switch()
                                  |
                                  +- switch_to()
                                         |
                                         +- __switch_to()
                                                 :
                                                 |
                                                 +- cpu_switch_to()

                                  +- barrier()
                                  |
                        finish_task_switch()

init_mm.pgd does not contain any user mappings and it is always active for kernel
addresses in TTBR1.

__switch_mm()
      |
      +- if next == &init_mm
                  |yes
                  +- cpu_set_reserved_ttbr0()
      |
      +- check_and_switch_context() <--- [struct mm_struct *next]
                    :
                    |
                    +- cpu_switch_mm()
                            |
                            :
                            +- cpu_do_switch_mm() [+] arch/arm64/mm/context.c
                               reset ttbr1_el1, ttbr0_el1 with some masks

----------------------------------------------------------------------------------------
signal_pending_state()

----------------------------------------------------------------------------------------

[+] arch/arm64/kernel/entry.S

/*
 * Register switch for AArch64. The callee-saved registers need to be saved
 * and restored. On entry:
 *   x0 = previous task_struct (must be preserved across the switch)
 *   x1 = next task_struct
 * Previous and next are guaranteed not to be the same.
 *
 */
SYM_FUNC_START(cpu_switch_to)
	mov	x10, #THREAD_CPU_CONTEXT
	add	x8, x0, x10
	mov	x9, sp
	stp	x19, x20, [x8], #16		// store callee-saved registers
	stp	x21, x22, [x8], #16
	stp	x23, x24, [x8], #16
	stp	x25, x26, [x8], #16
	stp	x27, x28, [x8], #16
	stp	x29, x9, [x8], #16
	str	lr, [x8]
	add	x8, x1, x10
	ldp	x19, x20, [x8], #16		// restore callee-saved registers
	ldp	x21, x22, [x8], #16
	ldp	x23, x24, [x8], #16
	ldp	x25, x26, [x8], #16
	ldp	x27, x28, [x8], #16
	ldp	x29, x9, [x8], #16
	ldr	lr, [x8]
	mov	sp, x9
	msr	sp_el0, x1
	ptrauth_keys_install_kernel x1, x8, x9, x10
	scs_save x0
	scs_load_current
	ret
SYM_FUNC_END(cpu_switch_to)
NOKPROBE(cpu_switch_to)

----------------------------------------------------------------------------------------
- Completion -

Completions are a code synchronization mechanism which is preferable to any misuse of
locks/semaphores and busy-loops. Completions are built on top of the waitqueue and
wakeup infrastructure of the Linux scheduler. The event the threads on the waitqueue
are waiting for is reduced to a simple flag in 'struct completion', appropriately
called "done".

Completions currently use a FIFO to queue threads that have to wait for the "completion"
event.

[+] kernel/sched/completion.c

wait_for_completion()
         |
         +- wait_for_common(x, MAX_SCHEDULE_TIMEOUT, TASK_UNINTERRUPTIBLE)
                  |                           |
                  +- __wait_for_common()      +-------------------------->+
                              |                                           |
                              :                                           |
                              +- do_wait_for_common()                     |
                                         |                                |
                                         +- if !x->done                   |
                                                  |yes                    |
                                                  :                       |
                                                  +- action() callback    |
                                                           |              |
                                                           v              |
                                                 schedule_timeout() <-----+
                                                           |
                                                           v
                                           +-----------------------------------+
                                           | Make the current task sleep       |
                                           | until @timeout jiffies have       |
                                           | elapsed. The function behavior    |
                                           | depends on the current task state |
                                           +-----------------------------------+

complete() will wake up a single thread waiting on this completion. Threads will be
    |      awakened in the same order in which they were queued.
    :
    +- x->done = UINT_MAX
    :
    +- swake_up_all_locked(&x->wait) finds the task on waitqueue and wakes it up.
               |
               :
               +- wake_up_process(curr->task)
                              |
                              v
        +------------------------------------------+
        | Attempt to wake up the nominated process |
        | and move it to the set of runnable       |
        | processes.                               |
        +------------------------------------------+
                              |
                              +- try_to_wake_up()
                                        |
                                        v
    +------------------------------------------------------------------------+
    | Conceptually does: If (@state & @p->state) @p->state = TASK_RUNNING.   |
    | If the task was not queued/runnable, also place it back on a runqueue. |
    +------------------------------------------------------------------------+

----------------------------------------------------------------------------------------
- workqueue -

While there are work items on the workqueue the worker executes the functions associated
with the work items one after the other. When there is no work item left on the workqueue
the worker becomes idle. When a new work item gets queued, the worker begins executing
again.

                     +--------+
           (*) <---- | worker | <---- (*) <---- (*) ---- workqueue
                     +--------+        |
                                       |
                                 (work item N)
                                       |
                                       +-> holds a pointer to the function that
                                           is to be executed asynchronously.

----------------------------------------------------------------------------------------
- CPUHP (CPU hotplug) -

The kernel option CONFIG_HOTPLUG_CPU needs to be enabled. It is currently available on
multiple architectures including ARM, MIPS, PowerPC and X86.

$ echo 0 > /sys/devices/system/cpu/cpu0/online

Once the CPU is shutdown, it will be removed from /proc/interrupts, /proc/cpuinfo and 
should also not be shown visible by the top command.


When a CPU is onlined, the startup callbacks are invoked sequentially until the state
CPUHP_ONLINE is reached. They can also be invoked when the callbacks of a state are
set up or an instance is added to a multi-instance state.

When a CPU is offlined the teardown callbacks are invoked in the reverse order
sequentially until the state CPUHP_OFFLINE is reached. They can also be invoked when
the callbacks of a state are removed or an instance is removed from a multi-instance
state.

----------------------------------------------------------------------------------------
(struct task_struct *)p

- sched_setaffinity()
- set_cpus_allowed_ptr()        p->cpus_ptr, p->nr_cpus_allowed
- set_user_nice()               p->se.load, p->*prio
- __sched_setscheduler()        p->sched_class, p->policy, p->*prio,
                                p->se.load, p->rt_priority,
                                p->dl.dl_{runtime, deadline, period, flags, bw, density}
- sched_setnuma()               p->numa_preferred_nid
- sched_move_task()             p->sched_task_group
- uclamp_update_active()        p->uclamp*

p->state <- TASK_* is changed locklessly using set_current_state(), __set_current_state()
or set_special_state(), see their respective comments, or by try_to_wake_up(). This
latter uses p->pi_lock to serialize against concurrent self.

p->on_rq <- { 0, 1 = TASK_ON_RQ_QUEUED, 2 = TASK_ON_RQ_MIGRATING } is set by
activate_task() and cleared by deactivate_task(), under rq->lock. Non-zero indicates the
task is runnable, the special ON_RQ_MIGRATING state is used for migration without holding
both rq->locks. It indicates task_cpu() is not stable, see task_rq_lock().

p->on_cpu <- { 0, 1 } is set by prepare_task() and cleared by finish_task() such that it
will be set before p is scheduled-in and cleared after p is scheduled-out, both under
rq->lock. Non-zero indicates the task is running on its CPU.

----------------------------------------------------------------------------------------
