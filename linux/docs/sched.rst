+------------------------------------------------------------------------------+
| LINUX KERNEL SCHEDULER                                                       |
+------------------------------------------------------------------------------+
- USERSPACE PROCESS EXECUTION -

execve() @syscall
   |
   +- do_execve()
        |
        +- do_execveat_common()
            :
            +- alloc_bprm()
            |       |
           [x]      :
                    +- bprm_mm_init()
                           |
                           +- mm_alloc()
                                 |
                                 +- allocate_mm()
                                         |
                                         +- kmem_cache_alloc()
                                                |
                                                +-> SLAB cache for mm_struct
                                                          (mm_cachep)
                                 |
                                 +- mm_init()

                           :
                           +- __bprm_mm_init()
                                    |
                                    : [+] kernel/fork.c
                                    +- vm_area_alloc()
                                            |
                                            +- kmem_cache_alloc()
                                                |
                                                + SLAB cache for vm_area_struct
                                                        (vm_area_cachep)
                                            |
                                            +- vma_init()
                                                   |
                                                   ▼
                                    +--------------------------------+
                                    | Initialize the bprm->vma as    |
                                    | anonymous vma, also initialize |
                                    | [vm_start, vm_end] of vma, etc.|
                                    +--------------------------------+

                                    vm_end = STACK_TOP_MAX
                                                    |
                                                    +-> 1 << VA_BITS_MIN
                                                                  |
                                                               VA_BITS
                                                                  |
                                                        CONFIG_ARM64_VA_BITS
                                                            (48 as default)

                        vm_start = vma->vm_end - PAGE_SIZE
                                        :
                                        |
                                        v [+] mm/mmap.c
                                insert_vm_struct()
                                        |
                                        ▼
                +----------------------------------------+
                | Insert vm structure into process list  |
                | sorted by address and into the inode's |
                | i_mmap tree.  If vm_file is non-NULL   |
                | then i_mmap_rwsem is taken here.       |
                +----------------------------------------+
                                        |
                                        +- find_vma_intersection()
                                                |
                                                ▼
                                +-----------------------------+
                                | Look up the first VMA which |
                                | intersects the interval.    |
                                +-----------------------------+
                                                |
                                                +- mt_find()
                                                        |
                                                        +-> mm->mm_mt
                                        |
                                        +- vma_link()
                                                :
                                                +- if vma->vm_file
                                                     |not NULL
                                                     :
                                                     +- vma->vm_file->f_mapping
                                                           :
                                                           |not NULL
                                                           +- __vma_link_file()
                                                                      |
                             +<---------------------------------------+
                             :
                             ▼
        vma_interval_tree_insert(vma, &mapping->i_mmap)
        (Interval tree - rbtree implemented)
        @note: mm->map_count++ in vma_link()

[x]
 :
 +- bprm_execve()
         |
         :
         +- sched_exec()
                 :
                 +- dest_cpu = p->sched_class->select_task_rq()
         |
         +- exec_binprm()

@maple tree

--------------------------------------------------------------------------------
fork() @syscall
  |
  +- kernel_clone() => It copies the process, and if successful kick-starts it
         :             and waits for it to finish using the VM if required.
         |
         +- copy_process()
                  :
                  +- dup_task_struct()
                  :
                  +- sched_fork()
                          :
                          +- __sched_fork() => Basic Setup

                  :
                  +- copy_thread() [+] arch/arm64/kernel/process.c

                *----------------------------------------------------*
                | if (struct kernel_clone_args *)args->fn != NULL    |
                | p->thread.cpu_context.x19 => args->fn              |
                | p->thread.cpu_context.x20 => args->fn_arg          |
                | p->thread.cpu_context.pc  => ret_from_fork         |
                | p->thread.cpu_context.sp  => childregs             |
                | p->thread.cpu_context.fp  => childregs->stackframe |
                *----------------------------------------------------*

@kernel assembly:

SYM_CODE_START(ret_from_fork)
	bl	schedule_tail
	cbz	x19, 1f	    => not a kernel thread
	mov	x0, x20
	blr	x19         => kthread()
1:	get_current_task tsk
	mov	x0, sp
	bl	asm_exit_to_user_mode
	b	ret_to_user
SYM_CODE_END(ret_from_fork)
NOKPROBE(ret_from_fork)

         :
         +- wake_up_new_task()
         :        :
                  +- activate_task()
                  :
                  +- check_preempt_curr()
                  :
                  +- p->sched_class->task_woken()
                  :

--------------------------------------------------------------------------------
- KERNEL THREAD -

kthread_create() => create a kthread on the current node
      |             and leave it in the stopped state.
      |
      +- kthread_create_on_node()
                   |
                   :
                   +- __kthread_create_on_node()
                      - Initialize *kthread_create_info* structure
                      - Insert create->list to the tail of *kthread_create_list*
                      - wake_up_process(kthreadd_task)
                                              :
                                              | created during rest_init()
                                              :
                                kthreadd() ◀--*
                                    |
                                    ▼
        create_kthread() <----------+---- [*] <= {global: kthread_create_list}
                |                          |
               [0]           (struct kthread_create_info)
                                           ▲
                                           |                       ➊
                                 newly created kthread ◀--- kthread_create()
                                  (struct task_struct)
               [0]
                :
                +- kernel_thread()
                        :
                        +- kernel_clone() [see above]

kthreadd() will constantly check the *kthread_create_list*. If it's not empty,
then create_kthread().

kthread()
    :
    +- sched_setscheduler_nocheck() {SCHED_NORMAL thread}
    :
    +- set_cpus_allowed_ptr()
    :
    +- create->threadfn() => The thread function created early

kthread_bind() => bind a just-created kthread to a cpu.
      |
      +- __kthread_bind(p, cpu, TASK_UNINTERRUPTIBLE)
                |
                +- __kthread_bind_mask()
                             |
                             :
                             +- do_set_cpus_allowed()
                                        |
                                        +- __do_set_cpus_allowed()

kthread_run()
     |
     +- kthread_create()
     |
     +- wake_up_process()

kthread_stop()

When a new kernel thread is created, thread stack is allocated from virtually
contiguous memory pages from the page level allocator. These pages are mapped
into contiguous kernel virtual space with PAGE_KERNEL protections.

alloc_thread_stack_node() calls __vmalloc_node_range() to allocate stack with
PAGE_KERNEL protections.

--------------------------------------------------------------------------------
SCHED INTRO

Processes scheduled under one of the real-time policies (SCHED_FIFO, SCHED_RR)
have a sched_priority value in the range 1 (low) to 99 (high). For threads
scheduled under one of the normal scheduling policies (SCHED_OTHER, SCHED_IDLE,
SCHED_BATCH), sched_priority is not used in scheduling decisions (it must be
specified as 0).

                                     -20          +19 (nice)
*------------------------------------*-------------*
| Realtime                           | Normal      |
*------------------------------------*-------------*
0                                  99 100         139

Priority: RT processes > Normal processes > Idle processes

(1) scheduler classes (struct sched_class)

*--------------------------------*
|       (*enqueue_task) ()       |
|       (*dequeue_task) ()       |
|       (*yield_task)   ()       |
|       (*yield_to_task)()       |
|       (*check_preempt_curr)()  |
|       (*pick_next_task)()      |
|       (*put_prev_task)()       |
|       (*set_next_task)()       |
|                                |
| #ifdef CONFIG_SMP              |
|       (*balance)()             |
|       (*select_task_rq)()      |
|       (*pick_task)()           |
|       (*migrate_task_rq)()     |
|       (*task_woken)()          |
|       (*set_cpus_allowed)()    |
|       (*rq_online)()           |
|       (*rq_offline)()          |
|       (*find_lock_rq)()        |
| #endif                         |
|                                |
|       (*task_tick)()           |
|       (*task_fork)()           |
|       (*task_dead)()           |
|       (*switched_from)()       |
|       (*switched_to)  ()       |
|       (*prio_changed) ()       |
|       (*get_rr_interval)()     |
|       (*update_curr)()         |
| #ifdef CONFIG_FAIR_GROUP_SCHED |
|       (*task_change_group)()   |
| #endif                         |
*--------------------------------*

--------------------------------------------------------------------------------
rt_sigprocmask() @syscall [+] kernel/signal.c
      :
      +- sigprocmask() => kernel threads that want to temporarily
                          (or permanently) block certain signals.
              :
              ▼
      *-------------------------------*
      | SIG_BLOCK       sigorsets()   |
      | SIG_UNBLOCK     sigandnsets() |
      | SIG_SETMASK                   |
      *-------------------------------*
              :
              +- __set_current_blocked()
                            :
                            +- __set_task_blocked()

--------------------------------------------------------------------------------
$ cat /proc/<pid>/schedstat

a) time spent on the cpu (in nanoseconds)
b) time spent waiting on a runqueue (in nanoseconds)
c) # of timeslices run on this cpu

@The main, per-CPU runqueue data structure

@struct rq

*---------------------------------------*
| ...                                   |
| unsigned int          nr_running;     |
| ...                                   |
| call_single_data_t    nohz_csd;       |
| ...                                   |
| struct cfs_rq         cfs;            |
| struct rt_rq          rt;             |
| struct dl_rq          dl;             |
| ...                                   |
| struct task_struct __rcu      *curr;  |
| struct task_struct    *idle;          |
| struct task_struct    *stop;          |
| unsigned long         next_balance;   |
| struct mm_struct      *prev_mm;       |
| ...                                   |
| struct rq             *core;          |
| struct task_struct    *core_pick;     |
| unsigned int          core_enabled;   |
| unsigned int          core_sched_seq; |
| struct rb_root        core_tree;      |
| ...                                   |
*---------------------------------------*

change the scheduling policy and/or RT priority of a thread
                        |
                        +- (struct sched_param *) param->sched_priority

sched_setscheduler()
        :
        +- _sched_setscheduler() with struct sched_attr initialization
                    :
                    +- __sched_setscheduler()
                                :

@struct sched_attr

*---------------------------------------*
| __u32 size;                           |
|                                       |
| __u32 sched_policy;                   |
| __u64 sched_flags;                    |
|                                       |
| __s32 sched_nice;                     |
|                                       |
| __u32 sched_priority;                 |
|                                       |
| __u64 sched_runtime;                  |
| __u64 sched_deadline;                 |
| __u64 sched_period;                   |
|                                       |
| __u32 sched_util_min;                 |
| __u32 sched_util_max;                 |
*---------------------------------------*

--------------------------------------------------------------------------------
- Preemption-RT -

(1) RT-Mutex

(2) Threaded IRQ handler

--------------------------------------------------------------------------------
- Deadline Task Scheduling -

@SCHED_DEADLINE: Sporadic task model deadline scheduling

A sporadic task is one that has a sequence of jobs, where each job is activated
at most once per period.  Each job also has a relative deadline, before which
it should finish execution, and a computation  time,  which  is the CPU time
necessary for executing the job. The moment when a task wakes up because a new
job has to be executed is called the arrival time (also referred to as the
request time or release time).  The start time is the time at which a task
starts its execution.  The absolute deadline is thus obtained by adding the
relative deadline to the arrival time.

           arrival/wakeup                    absolute deadline
                |    start time                    |
                |        |                         |
                v        v                         v
           -----x--------xooooooooooooooooo--------x--------x---
                         |<- comp. time ->|
                |<------- relative deadline ------>|
                |<-------------- period ------------------->|

The SCHED_DEADLINE policy contained inside the sched_dl scheduling class is
basically an implementation of the Earliest Deadline First (EDF) scheduling
algorithm, augmented with a mechanism (called Constant Bandwidth Server, CBS)
that makes it possible to isolate the behavior of tasks between each other.

SCHED_DEADLINE uses three parameters, named "runtime", "period", and "deadline",
to schedule tasks. A SCHED_DEADLINE task should receive "runtime" microseconds
of execution time every "period" microseconds, and these "runtime" microseconds
are available within "deadline" microseconds from the beginning of the period.

           arrival/wakeup                    absolute deadline
                |    start time                    |
                |        |                         |
                v        v                         v
           -----x--------xooooooooooooooooo--------x--------x---
                         |<-- Runtime ------->|
                |<----------- Deadline ----------->|
                |<-------------- Period ------------------->|

Summing up, the CBS algorithm assigns scheduling deadlines to tasks so that
each task runs for at most its runtime every period, avoiding any interference
between different tasks (bandwidth isolation), while the EDF algorithm selects
the task with the earliest scheduling deadline as the one to be executed next.

When a SCHED_DEADLINE task wakes up (becomes ready for execution), the scheduler
checks if:

             *------------------------------------------------*
             |        remaining runtime              runtime  |
             | ---------------------------------- > --------- |
             | scheduling deadline - current time     period  |
             *------------------------------------------------*

if scheduling_deadline < current_time
   |yes
   +-> scheduling deadline = current time + deadline remaining runtime = runtime

When a SCHED_DEADLINE task executes for an amount of time t, its remaining
runtime is decreased as:

                *-------------------------------------------*
                | remaining runtime = remaining runtime - t |
                *-------------------------------------------*

When the remaining runtime becomes less or equal than 0, the task is said to be
"throttled" (also known as "depleted" in real-time literature) and cannot be
scheduled until its scheduling deadline. The "replenishment time" for this task
is set to be equal to the current value of the scheduling deadline; When the
current time is equal to the replenishment time of a throttled task, the
scheduling deadline and the remaining runtime are updated as:

              *----------------------------------------------------*
              | scheduling deadline = scheduling deadline + period |
              | remaining runtime = remaining runtime + runtime    |
              *----------------------------------------------------*

--------------------------------------------------------------------------------
- CFS (Completely Fair Scheduler) Scheduler -

@struct task_struct

*---------------------------------------*
| ...                                   |
| int                      on_rq;       |
| int                      prio;        |
| int                      static_prio; |
| int                      normal_prio; |
| unsigned int             rt_priority; |
|                                       |
| struct sched_entity      se;          |
| struct sched_rt_entity   rt;          |
| struct sched_dl_entity   dl;          |
| const struct sched_class *sched_class;|
| ...                                   |
| struct rb_node           core_node;   |
|                                       |
*---------------------------------------*

CFS's task picking logic is based on this p->se.vruntime value and it is thus
very simple: it always tries to run the task with the smallest p->se.vruntime
value (i.e., the task which executed least so far). CFS always tries to split
up CPU time between runnable tasks as close to "ideal multitasking hardware"
as possible.

Ideal Multitasking Hardware - at any time all tasks would have the same
p->se.vruntime value (i.e., tasks would execute simultaneously and no task
would ever get "out of balance" from the "ideal" share of CPU time.)

CFS also maintains the rq->cfs.min_vruntime value, which is a monotonic
increasing value tracking the smallest vruntime among all tasks in the runqueue.
The total amount of work done by the system is tracked using min_vruntime.

CFS maintains a time-ordered rbtree, where all runnable tasks are sorted by the
p->se.vruntime key.

                                   ●
                                  / \
                                 ○   ○
                                / \   \
                               ●   ●
                              /
                      (leftmost task)

Summing up, CFS works like this: it runs a task a bit, and when the task
schedules (or a scheduler tick happens) the task's CPU usage is "accounted
for": the (small) time it just spent using the physical CPU is added to
p->se.vruntime. Once p->se.vruntime gets high enough so that another task
becomes the "leftmost task" of the time-ordered rbtree it maintains (plus a
small amount of "granularity" distance relative to the leftmost task so that
we do not over-schedule tasks and trash the cache), then the new leftmost task
is picked and the current task is preempted.

CFS implements three scheduling policies:

@SCHED_NORMAL: Default Linux time-sharing scheduling

The thread to run is chosen from the static priority 0 list based on a dynamic
priority that is determined only inside this list.  The dynamic priority is
based on the nice value and is increased for each time quantum the thread is
ready to run, but denied to run by the scheduler.

@SCHED_BATCH: Scheduling batch processes

SCHED_BATCH can be used only at static priority 0. This policy is similar to
SCHED_OTHER in that it schedules the thread according to its dynamic priority
(based on the nice value). The difference is that this policy will cause the
scheduler to always assume that the thread is CPU-intensive. Consequently, the
scheduler will apply a small scheduling penalty with respect to wakeup behavior,
so that this thread is mildly disfavored in scheduling decisions.

This policy is useful for workloads that are noninteractive, but do not want to
lower their nice value, and for workloads that want a deterministic scheduling
policy without interactivity causing extra preemptions (between the workload's
tasks).

@SCHED_IDLE: Scheduling very low priority jobs

--------------------------------------------------------------------------------

The nice value is an attribute that can be used to influence the CPU scheduler
to favor or disfavor a process in scheduling decisions. It affects the
scheduling of SCHED_NORMAL and SCHED_BATCH (see below) processes.

[+] kernel/sched/fair.c implements the CFS scheduler

--------------------------------------------------------------------------------
- LOAD BALANCE -

sched_init() => primary task is to initialize per cpu runqueues.
     :
     +- init_sched_fair_class()
                :
                +- open_softirq(SCHED_SOFTIRQ, run_rebalance_domains)
                                                       |
                                        +--------------+
                                        :
                                        +- nohz_idle_balance(this_rq, idle)
                                        :
                                        +- rebalance_domains(this_rq, idle)

run_rebalance_domains() can be triggered when needed from the scheduler tick.
Also triggered for nohz idle balancing (with nohz_balancing_kick set).

Normal load balance rebalance_domains() checks each scheduling domain to see
if it is due to be balanced, and initiates a balancing operation if so.

rebalance_domains()
        :
        +- load_balance()
                :
                +- find_busiest_group()
                |
                +- find_busiest_queue()
                :

      *
      |
*-----+-----*
| process N |
|           |
|           | detach_tasks(&env)
| ...       +-----*
|           |      \
|           |       \ attach_tasks(&env)
| process 0 |        \
*-----+-----*         *-----> pull processes from busiest runqueue to this_rq.
      |
      ▼

--------------------------------------------------------------------------------
- PERIODIC SCHEDULER -

scheduler_tick() => gets called by the timer code, with HZ frequency.
      :
      +- curr->sched_class->task_tick()
      :
      +- trigger_load_balance() => Trigger the SCHED_SOFTIRQ if it is
                    :              time to do periodic load balancing.
                    |
                    +- if time_after_eq(jiffies, rq->next_balance)
                                        |yes
                                        +- raise_softirq(SCHED_SOFTIRQ)
                    |
                    +- nohz_balancer_kick()
                        :
                        +- kick_ilb() => Kick a CPU to do the nohz balancing.
                                :
        smp_call_function_single_async(ilb_cpu, &cpu_rq(ilb_cpu)->nohz_csd)
                                       |
                                       ▼
        *----------------------------------------------------------------*
        | Run an asynchronous function on a specific CPU, in this case,  |
        | idle cpu picked by find_new_ilb().                             |
        *----------------------------------------------------------------*

--------------------------------------------------------------------------------

[+] kernel/sched/rt.c implements SCHED_FIFO and SCHED_RR semantics

@SCHED_FIFO: First in-first out scheduling

SCHED_FIFO can be used only with static priorities higher than 0, which means
that when a SCHED_FIFO thread becomes runnable, it will always immediately
preempt any currently running SCHED_NORMAL, SCHED_BATCH, or SCHED_IDLE thread.
SCHED_FIFO is a simple scheduling algorithm without time slicing.

@SCHED_RR: Round-robin scheduling

Similar to SCHED_FIFO, except that each thread is allowed to run only for a
maximum time quantum. If a SCHED_RR thread has been running for  a  time  period
equal to or longer than the time quantum, it will be put at the end of the list
for its priority.

               |<--- maximum time quantum --->|
               |                              |
        ◀------|-------------[x]--------------|---[*]--------
               |                              |             ▲
               @SCHED_RR                                    |
               |                                            :
               *- - - - - - - - - - - - - - - - - - - - - - *

--------------------------------------------------------------------------------
$ cat /proc/<pid>/schedstat

(a) time spent on the cpu (in nanoseconds)
(b) time spent waiting on a runqueue (in nanoseconds)
(c) # of timeslices run on this cpu

--------------------------------------------------------------------------------
- STOP TASKS -

The stop task is the highest priority task in the system, it preempts everything
and will be preempted by nothing.


--------------------------------------------------------------------------------
- Capacity Aware Scheduling -

Consider that homogeneous SMP (identical cpus) vs heterogeneous (different cpus)
platforms.

CPU capacity is a measure of the performance a CPU can reach.

=> capacity(cpu) = work_per_hz(cpu) * max_freq(cpu)

original (maximum attainabl) capacity
                |
                +- arch_scale_cpu_capacity()

@capacity fitness

The main capacity scheduling criterion of CFS

=> task_util(p) < capacity(task_cpu(p))

--------------------------------------------------------------------------------
- Energy Aware Scheduling -

Energy Aware Scheduling (or EAS) gives the scheduler the ability to predict the
impact of its decisions on the energy consumed by CPUs. EAS relies on an Energy
Model (EM) of the CPUs to select an energy efficient CPU for each task, with a
minimal impact on throughput.

EAS operates only on heterogeneous CPU topologies (such as Arm big.LITTLE)
because this is where the potential for saving energy through scheduling is the
highest.

--------------------------------------------------------------------------------
- CORE SCHEDULER -

[+] kernel/sched/core.c

schedule()
    |                   +-----------> tif_need_resched()
    :                   |             TIF_NEED_RESCHED => rescheduling necessary
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
                           held and interrupts off.
                        |
                        +- arch_start_context_switch()
                                |
                                ▼
                +---------------------------------------------+
                | kernel -> kernel   lazy + transfer active   |
                | user   -> kernel   lazy + mmgrab() active   |
                | kernel -> user     switch + mmdrop() active |
                | user   -> user     switch                   |
                +---------------------------------------------+
                                |
                                ▼
                                +- if !next->mm
                                        |yes => to kernel
                                        +- enter_lazy_tlb() => Lazy TLB
                                                |
                                                +- update_saved_ttbr0()

                                        :                no
                                        +- if prev->mm -----> prev->active_mm
                                                |yes          => NULL
                                                +- mmgrab()
                                                      |
                                                      ▼
                                +-------------------------------------+
                                | Pin the mm_struct, ensure it won't  |
                                | get freed even after the owning     |
                                | task exits. No guarantee that the   |
                                | associated address space will still |
                                | exist later on and mmget_not_zero() |
                                | has to be used before accessing it. |
                                +-------------------------------------+

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
                                        :
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
                                :
                                +- barrier()
                                |
                        finish_task_switch()

--------------------------------------------------------------------------------
init_mm.pgd does not contain any user mappings and it is always active for
kernel addresses in TTBR1.

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
                           reset ttbr1_el1, ttbr0_el1 with some masks.

preempt_schedule()
        |
        +- !preemptible() => check non-zero preempt_count or
                |no          interrupts are disabled.
                |
                +- preempt_schedule_common()
                        |
                        +- if need_resched() <-----*
                                  :yes             |
                        __schedule(SM_PREEMPT)     |
                                  :                |
                                  +--------------->*

do_sched_yield()
        :
        +- current->sched_class->yield_task() => A process wants to relinquish
        :                                        control of the processor
                                                 voluntarily.
        +- schedule()

--------------------------------------------------------------------------------
signal_pending_state()

--------------------------------------------------------------------------------

[+] arch/arm64/kernel/entry.S

*--------------------------------------------------------------------------*
| Register switch for AArch64. The callee-saved registers need to be saved |
| and restored. On entry:                                                  |
| x0 = previous task_struct (must be preserved across the switch)          |
| x1 = next task_struct                                                    |
| Previous and next are guaranteed not to be the same.                     |
*--------------------------------------------------------------------------*

SYM_FUNC_START(cpu_switch_to)
	mov	x10, #THREAD_CPU_CONTEXT
	add	x8, x0, x10
	mov	x9, sp
	stp	x19, x20, [x8], #16	=> store callee-saved registers
	stp	x21, x22, [x8], #16
	stp	x23, x24, [x8], #16
	stp	x25, x26, [x8], #16
	stp	x27, x28, [x8], #16
	stp	x29, x9, [x8], #16
	str	lr, [x8]
	add	x8, x1, x10
	ldp	x19, x20, [x8], #16	=> restore callee-saved registers
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

--------------------------------------------------------------------------------
- Completion -

Completions are a code synchronization mechanism which is preferable to any
misuse of locks/semaphores and busy-loops. Completions are built on top of
the waitqueue and wakeup infrastructure of the Linux scheduler. The event
the threads on the waitqueue are waiting for is reduced to a simple flag in
*struct completion*, appropriately called *done*.

Completions currently use a FIFO to queue threads that have to wait for the
*completion* event.

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
                                                           ▼
                                           +-----------------------------------+
                                           | Make the current task sleep       |
                                           | until @timeout jiffies have       |
                                           | elapsed. The function behavior    |
                                           | depends on the current task state |
                                           +-----------------------------------+

complete() will wake up a single thread waiting on this completion. Threads will
    |      be awakened in the same order in which they were queued.
    :
    +- x->done = UINT_MAX
    :
    +- swake_up_all_locked(&x->wait) finds the task on waitqueue and wakes it up.
               |
               :
               +- wake_up_process(curr->task)
                              |
                              ▼
        +------------------------------------------+
        | Attempt to wake up the nominated process |
        | and move it to the set of runnable       |
        | processes.                               |
        +------------------------------------------+
                              |
                              +- try_to_wake_up()
                                        |
                                        ▼
    +------------------------------------------------------------------------+
    | Conceptually does: If (@state & @p->state) @p->state = TASK_RUNNING.   |
    | If the task was not queued/runnable, also place it back on a runqueue. |
    +------------------------------------------------------------------------+
                                        :
                                        +- select_task_rq()
                                        :
                                        +- ttwu_queue() {ttwu => try_to_wake_up}
                                        :       :
                                                +- ttwu_queue_wakelist()
                                                :
                                                +- ttwu_do_activate()
                                                           :
                                                           +- activate_task()
                                                           :
                                                           +- ttwu_do_wakeup()

ttwu_queue_wakelist()
         :
         +- __ttwu_queue_wakelist()
                      :
*-----------------------------------------------------------------------*
| Queue a task on the target CPUs wake_list and wake the CPU via IPI if |
| necessary. The wakee CPU on receipt of the IPI will queue the task    |
| via sched_ttwu_wakeup() for activation so the wakee incurs the cost   |
| of the wakeup instead of the waker.                                   |
*-----------------------------------------------------------------------*

--------------------------------------------------------------------------------
- workqueue -

While there are work items on the workqueue the worker executes the functions
associated with the work items one after the other. When there is no work item
left on the workqueue the worker becomes idle. When a new work item gets queued,
the worker begins executing again.

                     *--------*
           (*) <---- | worker | <---- (*) <---- (*) ---- workqueue
                     *--------*        ▲
                                       |
                                 (work item N)
                                       |
                                       +-> holds a pointer to the function that
                                           is to be executed asynchronously.

--------------------------------------------------------------------------------
- CPUHP (CPU hotplug) -

The kernel option CONFIG_HOTPLUG_CPU needs to be enabled. It is currently
available on multiple architectures including ARM, MIPS, PowerPC and X86.

$ echo 0 > /sys/devices/system/cpu/cpu0/online

Once the CPU is shutdown, it will be removed from /proc/interrupts,
/proc/cpuinfo and should also not be shown visible by the top command.

When a CPU is onlined, the startup callbacks are invoked sequentially until
the state CPUHP_ONLINE is reached. They can also be invoked when the callbacks
of a state are set up or an instance is added to a multi-instance state.

When a CPU is offlined the teardown callbacks are invoked in the reverse
order sequentially until the state CPUHP_OFFLINE is reached. They can also
be invoked when the callbacks of a state are removed or an instance is removed
from a multi-instance state.

--------------------------------------------------------------------------------
(struct task_struct *)p

- sched_setaffinity()
- set_cpus_allowed_ptr() p->cpus_ptr, p->nr_cpus_allowed
- set_user_nice()        p->se.load, p->*prio
- __sched_setscheduler() p->sched_class, p->policy, p->*prio,
                         p->se.load, p->rt_priority,
                         p->dl.dl_{runtime/deadline/period/flags/bw/density}
- sched_setnuma()        p->numa_preferred_nid
- sched_move_task()      p->sched_task_group
- uclamp_update_active() p->uclamp*

p->state <- TASK_* is changed locklessly using set_current_state(),
__set_current_state() or set_special_state(), see their respective comments, or
by try_to_wake_up(). This latter uses p->pi_lock to serialize against concurrent
self.

p->on_rq <- { 0, 1 = TASK_ON_RQ_QUEUED, 2 = TASK_ON_RQ_MIGRATING } is set by
activate_task() and cleared by deactivate_task(), under rq->lock. Non-zero
indicates the task is runnable, the special ON_RQ_MIGRATING state is used
for migration without holding both rq->locks. It indicates task_cpu() is not
stable, see task_rq_lock().

p->on_cpu <- { 0, 1 } is set by prepare_task() and cleared by finish_task()
such that it will be set before p is scheduled-in and cleared after p is
scheduled-out, both under rq->lock. Non-zero indicates the task is running
on its CPU.

--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
udelay()

--------------------------------------------------------------------------------
