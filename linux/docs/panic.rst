+--------------------------------------------------------------------------------------------------+
| KERNEL PANIC ANALYSIS                                                                            |
+--------------------------------------------------------------------------------------------------+

Panic when running lkvm on radxa-zero-2pro with linux-5.10.201 Image, buildroot-made rootfs.cpio
$ lkvm run -k Image -i rootfs.cpio

----------------------------------------------------------------------------------------------------
[    1.859276] Freeing unused kernel memory: 3712K
[    1.907583] Run /init as init process
[    1.909518] Kernel panic - not syncing: Attempted to kill init! exitcode=0x00000004
[    1.909907] CPU: 4 PID: 1 Comm: init Not tainted 5.10.201 #34                 |
[    1.910173] Hardware name: linux,dummy-virt (DT)                              v
[    1.910372] Call trace:                                                     SIGILL
[    1.910458]  dump_backtrace+0x0/0x1f0
[    1.910618]  show_stack+0x18/0x24
[    1.910749]  dump_stack+0xe8/0x124
[    1.910957]  panic+0x198/0x380
[    1.911169]  do_exit+0xa1c/0xa60
[    1.911318]  do_group_exit+0x38/0xa0
[    1.912376]  get_signal+0x18c/0x8b4
[    1.912642]  do_notify_resume+0x2ac/0x980
[    1.912894]  work_pending+0xc/0x618
[    1.913031] SMP: stopping secondary CPUs
[    1.913267] Kernel Offset: disabled
[    1.913396] CPU features: 0x08240002,61082004
[    1.913543] Memory Limit: none
[    1.913649] ---[ end Kernel panic - not syncing: Attempted to kill init! exitcode=0x00000004 ]---
----------------------------------------------------------------------------------------------------

@include/uapi/asm-generic/signal.h

#define _NSIG		64
#define _NSIG_BPW	__BITS_PER_LONG
#define _NSIG_WORDS	(_NSIG / _NSIG_BPW)

#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
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

/* These should not be considered constants from userland.  */
#define SIGRTMIN	32
#ifndef SIGRTMAX
#define SIGRTMAX	_NSIG
#endif

----------------------------------------------------------------------------------------------------

SIGILL - Illegal Instruction

Possible Causes:
1) data section overwritten
2) compiler issueas
3) ...
Whichever generates incorrect instructions that cpu can't recognize.

----------------------------------------------------------------------------------------------------

Panic has been solved. It might be the problem that compilation issues or configuration issues with
rootfs images, where might corrupt the executable.

Currently use busybox-1.36.1 made rootfs.ext4:
$ lkvm run -k Image -d rootfs.ext4 -c 6 -m 512 -n debug --debug --console serial

----------------------------------------------------------------------------------------------------

[+] arch/arm64/kernel/entry.S

/*
 * EL0 mode handlers.
 */
	.align	6
SYM_CODE_START_LOCAL_NOALIGN(el0_sync)
	kernel_entry 0
	mov	x0, sp
	bl	el0_sync_handler
	b	ret_to_user
SYM_CODE_END(el0_sync)

/*
 * "slow" syscall return path.
 */
SYM_CODE_START_LOCAL(ret_to_user)
	disable_daif
	gic_prio_kentry_setup tmp=x3
#ifdef CONFIG_TRACE_IRQFLAGS
	bl	trace_hardirqs_off
#endif
	ldr	x19, [tsk, #TSK_TI_FLAGS]
	and	x2, x19, #_TIF_WORK_MASK
	cbnz	x2, work_pending
finish_ret_to_user:
	user_enter_irqoff
	/* Ignore asynchronous tag check faults in the uaccess routines */
	clear_mte_async_tcf
	enable_step_tsk x19, x2
#ifdef CONFIG_GCC_PLUGIN_STACKLEAK
	bl	stackleak_erase
#endif
	kernel_exit 0

/*
 * Ok, we need to do extra processing, enter the slow path.
 */
work_pending:
	mov	x0, sp			        	// 'regs'
	mov	x1, x19
	bl	do_notify_resume
	ldr	x19, [tsk, #TSK_TI_FLAGS]	// re-check for single-step
	b	finish_ret_to_user
SYM_CODE_END(ret_to_user)

   [branch]
       |
do_notify_resume()
       |
       +- thread_flags & (_TIF_SIGPENDING | _TIF_NOTIFY_SIGNAL)
               |yes
               +- do_signal() [+] arch/arm64/kernel/signal.c
                      |
                      |  (struct ksignal) ksig
                      |      |
                      +- get_signal() [+] kernel/signal.c
                             |
                             +- sig_kernel_coredump()
                                         |yes
                                         +- do_coredump() [+] fs/coredump.c
                                                 |
                                                 ▽
                                ksig->info.si_signo
                             |        |
                             +- do_group_exit() [+] kernel/exit.c
                                      |
                                      +- do_exit()
                                             |
                                             +- Check atomic value of tsk->signal->live
                                                        |group_dead
                                                        +- is_group_init(tsk)
                                                                   |yes
                                                                   +- panic()  [+] kernel/panic.c

+--------------------------------------------------------------------------------------------------+
| Refer to Linux-5.10.201                                                                          |
+--------------------------------------------------------------------------------------------------+
