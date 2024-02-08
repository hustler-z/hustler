+--------------------------------------------------------------------------------------+
| LINUX KERNEL SCHEDULER                                                               |
+--------------------------------------------------------------------------------------+

----------------------------------------------------------------------------------------
- Deadline Task Scheduling -

The SCHED_DEADLINE policy contained inside the sched_dl scheduling class is basically an
implementation of the Earliest Deadline First (EDF) scheduling algorithm, augmented with
a mechanism (called Constant Bandwidth Server, CBS) that makes it possible to isolate the
behavior of tasks between each other.

----------------------------------------------------------------------------------------
- CFS Scheduler -


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


----------------------------------------------------------------------------------------
[+] kernel/sched/core.c

schedule()
    |
    :
    +- __schedule() if need_resched()
            |
            +- get the rq from current cpu with cpu_rq()
               note rq is the main, per-CPU runqueue
            :
            +- schedule_debug()
            :
            +- check the sched_mode and prev->__state
                                |ok
                                +- signal_pending_state()
                                             |no
                                             :
                                             +- deactivate_task()
                                                       |
                                                       +- dequeue_task()

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
