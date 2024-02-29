+------------------------------------------------------------------------------+
| KERNEL BUGS                                                                  |
+------------------------------------------------------------------------------+

--------------------------------------------------------------------------------

ESR (Exception Syndrome Register)

63   56 55                   32 31    25                                    0
+------+-----------------------+----+--+------------------------------------+
| RES0 |         ISS2          | EC |IL|                ISS                 |
+------+-----------------------+----+--+------------------------------------+

EC (Exception Class) [31:26] - Indicates reason for the exception
IL (Instruction Length) [25] - For synchronous exception
ISS (Instruction Specific Syndrome) [24:0]

--------------------------------------------------------------------------------
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

[+] arch/arm64/mm/fault.c

do_mem_abort()
     |
     +- (struct fault_info *)inf->fn(far, esr, regs)
                                   |
Registration on fault_info[] <-----+ callback
                                   |!=0                 yes
                                   +- user_mode(regs) ------> die_kernel_fault()
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

--------------------------------------------------------------------------------

arm64_force_sig_fault()
          |
          +- arm64_show_signal()
          |                       no
          +- signo == SIGKILL --------> force_sig_fault()
                    |yes
                    +- force_sig(SIGKILL)

--------------------------------------------------------------------------------

do_bad_area()
     |                    yes
     +- user_mode(regs) ------> set_thread_esr() ----> arm64_force_sig_fault()
              |no
              +- __do_kernel_fault()
                         :
                         |
          [Analysis on Exception reason]
                         |
                         v
     +---------------------------------------+
     | msg                                   |
     | 1) write to read-only memory          |
     | 2) execute from non-executable memory |
     | 3) read from unreadable memory        |
     | 4) NULL pointer dereference           |
     | 5) paging request                     |
     +---------------------------------------+

If can't fix the exception then die_kernel_fault() fires with the "msg".

die_kernel_fault() => log: Unable to handle kernel * at virtual address *
       |
       |
       +- mem_abort_decode() => display mem abort infos
       |
       +- show_pte()
       |
       +- die("Oops"", regs, esr) => check the workflow below
       :
       +- make_task_dead(SIGKILL)
       :

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

--------------------------------------------------------------------------------
- Kernel Panic -

As log shown below:

Kernel panic - not syncing: Attempted to kill init! exitcode=0x00000004

where exitcode defined in [+] include/uapi/asm-generic/signal.h

--------------------------
#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4 => Illegal Instruction
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGBUS		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGURG		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGIO		29
#define SIGPOLL		SIGIO
/*
#define SIGLOST		29
*/
#define SIGPWR		30
#define SIGSYS		31
#define	SIGUNUSED	31
#define SIGRTMIN	32
--------------------------

panic() => halt the system [+] kernel/panic.c
  |
  :
  +- a) disable local interrupt via local_irq_disable()
        also disable preemption via preempt_disable_notrace()
  :
  +- b) Ensure only one CPU is allowed to execute the panic
        code by panic_smp_self_stop() other CPUs
                          :
                          +- sdei_mask_local_cpu()
  :
  +- kgdb_panic()
  :                                  yes
  +- _crash_kexec_post_notifiers=0 ------> __crash_kexec() ------> crash kernel
  :
  +- panic_other_cpus_shutdown()
                 :
                 +- crash_smp_send_stop() => Stop other CPUs in panic
                             :
                             +- smp_send_stop()
                                       :
                                       +- smp_cross_call(&mask, IPI_CPU_STOP)
                                                :
                                                +- __ipi_send_mask()
                                                          |
                                                a) chip->ipi_send_mask()
                                                b) chip->ipi_send_single()
  :
*----------------------------------------------------*
| Run any panic handlers in an atomic notifier chain |
| panic_notifier_list by atomic_notifier_call_chain()|
*----------------------------------------------------*
  :
  +- kmsg_dump(KMSG_DUMP_PANIC) => dump kernel log to kernel message dumpers
  :
*----------------------------------------------------*
| _crash_kexec_post_notifiers offers a chance to run |
| panic_notifiers and dumping kmsg before kdump.     |
*----------------------------------------------------*
  :                                   yes
  +- _crash_kexec_post_notifiers!=0 ------> __crash_kexec() -----> crash kernel
  :

--------------------------------------------------------------------------------
- Crash Kernel -

__crash_kexec()

--------------------------------------------------------------------------------
Bus Error

SIGBUS {si_signo=SIGBUS, si_code=BUS_ADRERR, si_addr=0x7fa884a000}
                                     |
                                     v
SIGBUS indicates an access to an invalid address. In particular, SIGBUS signals
often result from dereferencing a misaligned pointer.

--------------------------------------------------------------------------------
- TRAPS -

trap_init() been called in start_kernel()


--------------------------------------------------------------------------------
BUG() will generate a Breakpoint Instruction Exception (EC 0x3C) via assembly
code: brk BUG_BRK_IMM (0x800)

    BUG() [ BUG_ON(),WARN(),WARN_ON() ]
     |traps
     v
bug_handler() [+] arch/arm64/kernel/traps.c
     |
     +- report_bug()
             :                   warning
             +- __report_bug() -----------> __warn() ---- @BUG_TRAP_TYPE_WARN
                      |
              @BUG_TRAP_TYPE_BUG
                      |
     |________________|
     |
     +- die("Oops - BUG", regs, esr)
                :
                +- __die()
                       |
                       +- notify_die() => call functions in die_chain
                              :
                              +- nb->notifier_call() => kgdb_notify()
                       :
                       +- print_modules()
                       |
                       +- show_regs()
                :
                +- if in_interrupt()
                       |yes
                       +- panic("%s: Fatal exception in interrupt", str)
                |
                +- if panic_on_oops
                       |yes
                       +- panic("%s: Fatal exception", str)
                :
                +- if return value of __die() != NOTIFY_STOP
                       |yes
                       +- make_task_dead(SIGSEGV) => check the workflow above

As log below:

------------[ cut here ]------------
Kernel BUG at do_task_dead+0x4c/0x50 [verbose debug info unavailable ]
Internal error: Oops - BUG: 0 [#1] PREEMPT_RT SMP
Modules linked in:
CPU: 6 PID: 190 Comm: test_thread1 Tainted: G  W (init_utsname()->version)
Hardware name: linux,dummy-virt (DT)
pstate: 60000005 (nZCv daif -PAN -UAO -TCO BTYPE=--)
pc : do_task_dead+0x4c/0x50
lr : do_task_dead+0x4c/0x50
sp : ffffffc0113bbdb0
x29: ffffffc0113bbdb0 x28: ffffff8001498040
x27: ffffff80038963c8 x26: ffffffc010e54a08
x25: ffffff800389b040 x24: ffffff800389afc0
x23: ffffffc010d75080 x22: ffffffc0113bbe38
x21: 0000000000000000 x20: ffffff800389b328
x19: ffffff800389abc0 x18: 0000000000000020
x17: 00000000b4b80526 x16: 0000000000000001
x15: 00000563c95f14b4 x14: 001599bc21580ad4
x13: 00000000000001bd x12: 00000000fa83b2da
x11: 0000000000000000 x10: 0000000000000563
x9 : 0000000000000000 x8 : 00000000001599bc
x7 : 000000000000b6fb x6 : 0000000000000000
x5 : 000000000000001e x4 : 000000000000001e
x3 : ffffff803fdd3880 x2 : 0000000000000000
x1 : 0000000000000000 x0 : ffffff800389abc0
Call trace:
do_task_dead+0x4c/0x50
do_exit+0x5e8/0x950
kthread+0x144/0x1a0
ret_from_fork+0x10/0x34
Code: 52800000 32110042 b9003e62 94222456 (d4210000)
---[ end trace 0000000000000004 ]---
Kernel panic - not syncing:
Oops - BUG: Fatal exception

--------------------------------------------------------------------------------
To debug a kernel, use objdump and look for the hex offset from the crash output
to find the valid line of code/assembler.

$ objdump -r -S -l --disassemble *.o
			[-d]
@ -t => display static symbol table vs. -T dynamic symbol table

--------------------------------------------------------------------------------
