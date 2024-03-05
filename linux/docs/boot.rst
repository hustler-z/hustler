+------------------------------------------------------------------------------+
| BOOTING PROCESS for Linux                                                    |
+------------------------------------------------------------------------------+



--------------------------------------------------------------------------------
- START KERNEL -

[+] init/main.c
asmlinkage __visible void __init __no_sanitize_address start_kernel(void)

start_kernel()
     |
     :
     +- cgroup_init_early()
     :
     +- boot_cpu_init()
     :
     +- setup_arch()
     :
     +- setup_per_cpu_areas()
     :
     +- build_all_zonelists(NULL)
     :
     +- page_alloc_init()
     :
     +- trap_init()
     :
     +- mm_init()
     :
     +- sched_init()
             :
             +- initialize per-cpu runqueue data structures
             :
             +- init_sched_fair_class()
             :            :
     :
     +- radix_tree_init()
     :
     +- maple_tree_init()
     :
     +- workqueue_init_early()
     :
     +- rcu_init()
     :
     +- early_irq_init()
     :
     +- init_IRQ()
     :
     +- tick_init()
     :
     +- init_timers()
     :
     +- hrtimers_init()
     :
     +- softirq_init()
     :
     +- time_init()

     :
     +- kmem_cache_init_late()
     :
     +- setup_per_cpu_pageset()
     :
     +- sched_clock_init()
     :
     +- anon_vma_init()
     :
     +- thread_stack_cache_init()
     :
     +- pagecache_init()
              :
              +- In order to wait for pages to become available
                 there must be waitqueues associated with pages.
                 - folio_wait_table[i]
     :
     +- signals_init()
     :
     +- cpuset_init()
     :
     +- cgroup_init()
     :
     +- arch_call_rest_init()
     :           |
     ▼           +- rest_init()
                        :
                        +- create a user mode thread via user_mode_thread()
                                |
                                ▼
                           kernel_init()
                                :
                                +- wait until kthreadd is all set-up
                                   wait_for_completion(&kthreadd_done) <---*
                                :                                          ▲
                                +- kernel_init_freeable()                  |
                                            :                              |
                                :                                          |
                                +- run_init_process()                      |
                        :                                                  |
                        +- create a kernel thread via kernel_thread()      |
                                |                                          |
                                ▼                                          |
                           kthreadd()                                      |
                        :                                                  |
                        +- complete(&kthreadd_done) ---------------------->*

kernel_init_freeable()
          :
          +- workqueue_init()
          :
          +- smp_init()
          :
          +- sched_init_smp()
                    :
                    +- sched_init_domains()
                    :
                    +- init_sched_rt_class()
                    :
                    +- init_sched_dl_class()
                    :
          :
          +- do_basic_setup()
          :         :
                    +- driver_init()

--------------------------------------------------------------------------------
