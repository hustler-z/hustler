+--------------------------------------------------------------------------------------+
| CASE STUDY                                                                           |
+--------------------------------------------------------------------------------------+

kernel logs when it crashed:

[   54.667134] Unable to handle kernel paging request at virtual address ffffffc08138799e
[   54.667395] Mem abort info:
[   54.667451]   ESR = 0x96000045
[   54.667610]   EC = 0x25: DABT (current EL), IL = 32 bits
[   54.667704]   SET = 0, FnV = 0
[   54.667772]   EA = 0, S1PTW = 0
[   54.667870] Data abort info:
[   54.667938]   ISV = 0, ISS = 0x00000045
[   54.667996]   CM = 0, WnR = 1
[   54.670503] swapper pgtable: 4k pages, 39-bit VAs, pgdp=0000000040e1c000
[   54.670776] [ffffffc08138799e] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
[   54.674304] Internal error: Oops: 96000045 [#1] PREEMPT_RT SMP
[   54.679438] Modules linked in:
[   54.679668] CPU: 0 PID: 639 Comm: thread2 Tainted: G W 5.10.145-rt74-g8486af2177c9-dirty #36
[   54.679981] Hardware name: linux,dummy-virt (DT)
[   54.680254] pstate: 80000085 (Nzcv daIf -PAN -UAO -TCO BTYPE=--)
[   54.680409] pc : queued_spin_lock_slowpath+0x1b8/0x3a8
[   54.680636] lr : _raw_spin_lock_irqsave+0x84/0xb0
[   54.680792] sp : ffffffc011ecbe10
[   54.680851] x29: ffffffc011ecbe10 x28: 0000000000000000
[   54.681051] x27: 0000000000000000 x26: ffffffc010e74a10
[   54.681224] x25: ffffffc011eb3d80 x24: ffffffc0100a2440
[   54.681769] x23: ffffffc010b05228 x22: ffffffc011eb3c68
[   54.682597] x21: ffffff80c249a980 x20: ffffff80c2458ec0
[   54.682771] x19: 0000000000000000 x18: ffffffffffffffff
[   54.682889] x17: 0000000012e9a236 x16: 0000000031f39da0
[   54.682982] x15: 0000ad3d7b10e734 x14: 00000000000002d1
[   54.683058] x13: 00000000000002d1 x12: 0000000000000000
[   54.683132] x11: 0000000000000002 x10: 0000000000000000
[   54.683229] x9 : 0000000000040000 x8 : 0000000000000000
[   54.683307] x7 : ffffff80ff788a00 x6 : ffffffc010da98e0
[   54.683381] x5 : ffffffc010c90000 x4 : ffffff80ff788a00
[   54.683493] x3 : ffffffc011eb3d80 x2 : 00000000706f6f6e
[   54.683571] x1 : ffffffc010c90a30 x0 : ffffff80ff788a08
[   54.683651] Call trace:
[   54.683692]  queued_spin_lock_slowpath+0x1b8/0x3a8
[   54.683804]  swake_up_one+0x18/0x50
[   54.683879]  thread_wake2+0x10/0x20
[   54.683950]  kthread+0x18c/0x198
[   54.684023]  ret_from_fork+0x10/0x34
[   54.693206] Code: d37c0400 8b000021 910020e0 f862d8c2 (f8226827)
[   54.699734] ---[ end trace 0000000000000004 ]---

----------------------------------------------------------------------------------------

ESR (Exception Syndrome Register)

63   56 55                   32 31    25                                    0
+------+-----------------------+----+--+------------------------------------+
| RES0 |         ISS2          | EC |IL|                ISS                 |
+------+-----------------------+----+--+------------------------------------+

EC (Exception Class) [31:26] - Indicates reason for the exception
IL (Instruction Length) [25] - For synchronous exception
ISS (Instruction Specific Syndrome) [24:0]

----------------------------------------------------------------------------------------
<a> arch/arm64/kernel/entry-common.c

el1h_64_sync_handler() branch based on EC
          |
          | • el1_abort() ---> do_mem_abort(far, esr, regs)
          | • el1_pc()    ---> do_sp_pc_abort(far, esr, regs)
          | • el1_undef() ---> do_undefinstr(regs, esr)
          | • el1_bti()   ---> do_el1_bti(regs, esr)
          | • el1_dbg()   ---> do_debug_exception(far, esr, regs)
          | • el1_fpac()  ---> do_el1_spac(regs, esr)
          v

el0t_64_sync_handler() branch based on EC
          |
          | • el0_svc() ---> invoke_syscall()
          | • el0_da()  ---> do_mem_abort()
          | • el0_ia()  ---> do_mem_abort()
          | • el0_fpsimd_acc()
          | • el0_sve_acc()
          | • el0_sme_acc()
          | • el0_fpsimd_exc()
          | • el0_sys()
          | • el0_sp()
          | • el0_pc()
          | • el0_undef()
          | • el0_bti() ---> do_el0_bti()
          | • el0_dbg()
          | • el0_fpac()
          | • el0_inv()
          v

*) BTI (Branch Target Identification)

do_el1_bti()
     |
     +- die()

do_el0_bti()
     |
     +- force_signal_inject()

*) FPAC

do_el1_fpac()
     |
     +- die()

do_el0_fpac()
     |
     +- force_signal_inject()
                  |
                  :
                  +- @desc [SIGILL/SIGSEGV]
                     arm64_notify_die()

<b> arch/arm64/mm/fault.c

do_mem_abort()
     |
     +- (struct fault_info *)inf->fn(far, esr, regs)
                                   |
Registration on fault_info[] <-----+ callback
                                   |!=0                  yes
                                   +- user_mode(regs) --------> die_kernel_fault()
                                            |no
                                            +- arm64_notify_die()
                                               For an unrecognized fault type.

Functions includes:

- do_bad()
     |
     +- Unimpletmented

- do_translation_fault()
           |                        yes
           +- is_ttbr0_addr(far) --------> do_page_fault()
                     |no
                     +- do_bad_area()

- do_page_fault()

[Synchronous External Abort]
- do_sea()
     |                                               yes
     +-> In user state, apei_claim_sea(regs) == 0 --------> <return>
				|no
				: <--- siaddr
				|
		arm64_notify_die()
                |                     yes
                +- user_mode(regs) --------> arm64_force_sig_fault()
                         |no
                         +- die()

- do_tag_check_fault()
           |
           +- do_bad_area()

- do_alignment_fault()
           |
           +- compact_user_mode(regs)                       yes
              && CONFIG_COMPACT_ALIGNMENT_FIXUPS enabled --------+
                        |no                                      |
                        +- do_bad_area()            do_compact_alignment_fixup()

----------------------------------------------------------------------------------------

arm64_force_sig_fault()
          |
          +- arm64_show_signal()
          |                       no
          +- signo == SIGKILL --------> force_sig_fault()
                    |yes
                    +- force_sig(SIGKILL)

----------------------------------------------------------------------------------------

do_bad_area()
     |                     yes
     +- user_mode(regs) --------> set_thread_esr() ----> arm64_force_sig_fault()
              |no
              +- __do_kernel_fault()
                         :
                         |
            [Analysis on Exception reason]
            If can't fix the exception then die_kernel_fault() fires with the "msg".

die_kernel_fault()
       |
       +<--------------------------+ bust_spinlock(1)
       |
       +- mem_abort_decode()
       |
       +- show_pte()
       |
  +<---+- die("Oops"", regs, esr)
  |    |
  |    +<--------------------------+ bust_spinlock(0)
  |    :
  |    +- make_task_dead(SIGKILL)
  |                |
  |                :
  :
  +- oops_enter()              +-> arch/arm64/kernel/traps.c
  |                            |
  +- console_verbose() ---> __die() ---> crach_kexec() ---> add_taint()
  |                            |
  :                            +- notify_die()
  |
  +- oops_exit()
  :
  +- panic() <--- in_interrupt()
  :
  +- The return value of __die() != NOTIFY_STOP
                        |yes
                        +- make_task_dead(SIGSEGV)

make_task_dead()
       :
       |
       +- panic() to halt the system
       :
       |
       +- do_exit(signr)
              :
              +- if tsk->mm (current task)
                          |
                          +- sync_mm_rss(tsk->mm)
              |
              +- if group_dead <- tsk->signal->live
                         |
                         +- if is_global_init(tsk)
                                    |yes
                                    +- panic()
              :
              +- taskstats_exit()
              |
              +- exit_mm()
              |
              +- trace_sched_process_exit()
              |
              +- exit_sem()
              |
              +- exit_shm()
              |
              +- exit_files()
              |
              +- exit_fs()
              |
              +- exit_task_namespaces()
              |
              +- exit_task_work()
              |
              +- exit_thread()
              |
              +- perf_event_exit_task()
              |
              +- sched_autogroup_exit_task()
              |
              +- cgroup_exit()
              |
              +- flush_ptrace_hw_breakpoint()
              |
              +- exit_tasks_rcu_start()
              |
              +- exit_notify()
              |
              +- proc_exit_connector()
              |
              +- mpol_put_task_policy()
              :
              +- validate_creds_for_do_exit()
              |
              +- exit_task_stack_account()
              |
              +- exit_rcu()
              |
              +- exit_tasks_rcu_finish()
              :
              +- do_task_dead()
                      |
                      :
                      +- __schedule(SM_NONE)

panic()
  |
  :
  +- Disable local interrupts.
     local_irq_disable()
     preempt_disable_notrace()

  +- Only one CPU is allowed to execute the panic
     if old_cpu != PANIC_CPU_INVALID && old_cpu != this_cpu
             |                          |yes
             |                          +- panic_smp_self_stop()
             +- 1st CPU fired panic()
                        |
                        +- console_verbose()
                        :
                        +- if !test_taint(TAINT_DIE) && oops_in_progress <= 1
                                      |yes
                                      +- dump_stack()

                        +- kgdb_panic()
                        :
                        +-

----------------------------------------------------------------------------------------

Bus Error

SIGBUS {si_signo=SIGBUS, si_code=BUS_ADRERR, si_addr=0x7fa884a000}
                                     |
                                     v
SIGBUS indicates an access to an invalid address. In particular, SIGBUS signals often
result from dereferencing a misaligned pointer.

----------------------------------------------------------------------------------------
