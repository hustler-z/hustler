+--------------------------------------------------------------------------------------+
| LINUX KERNEL BASICS                                                                  |
+--------------------------------------------------------------------------------------+

Kernel Thread

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

When a new kernel thread is created, thread stack is allocated from virtually contiguous
memory pages from the page level allocator. These pages are mapped into contiguous
kernel virtual space with PAGE_KERNEL protections.

alloc_thread_stack_node() calls __vmalloc_node_range() to allocate stack with
PAGE_KERNEL protections.

----------------------------------------------------------------------------------------
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

----------------------------------------------------------------------------------------
[+] arch/arm64/include/asm/barrier.h

Send Event is a hint instruction. It causes an event to be signaled to all PEs in the
multiprocessor system.

#define sev()		asm volatile("sev" : : : "memory")

Wait For Event is a hint instruction that indicates that the PE can enter a low-power
state and remain there until a wakeup event occurs.

#define wfe()		asm volatile("wfe" : : : "memory")

Wait For Event with Timeout is a hint instruction that indicates that the PE can enter
a low-power state and remain there until either a local timeout event or a wakeup event
occurs. Wakeup events include the event signaled as a result of executing the SEV
instruction on any PE in the multiprocessor system.

#define wfet(val)	asm volatile("msr s0_3_c1_c0_0, %0"	\
				     : : "r" (val) : "memory")

#define wfi()		asm volatile("wfi" : : : "memory")

#define wfit(val)	asm volatile("msr s0_3_c1_c0_1, %0"	\
				     : : "r" (val) : "memory")

#define isb()		asm volatile("isb" : : : "memory")

#define dmb(opt)	asm volatile("dmb " #opt : : : "memory")

#define dsb(opt)	asm volatile("dsb " #opt : : : "memory")

----------------------------------------------------------------------------------------
spin_lock/unlock()


----------------------------------------------------------------------------------------
RCU (Read-Copy Update) is a synchronization mechanism.




----------------------------------------------------------------------------------------
