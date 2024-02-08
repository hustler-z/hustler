+--------------------------------------------------------------------------------------+
| BOOTING PROCESS for Linux                                                            |
+--------------------------------------------------------------------------------------+

----------------------------------------------------------------------------------------
Bare-metal Boot Steps for ARMv8 AArch64 mode

• Initializing exceptions
• Initializing registers
• Configuring the MMU and caches
• Enabling NEON and floating point
• Changing exception levels

----------------------------------------------------------------------------------------
- START KERNEL -

asmlinkage __visible void __init __no_sanitize_address start_kernel(void)
{
	char *command_line;
	char *after_dashes;

	set_task_stack_end_magic(&init_task);
	smp_setup_processor_id();
	debug_objects_early_init();
	init_vmlinux_build_id();

	cgroup_init_early();

	local_irq_disable();
	early_boot_irqs_disabled = true;

	/*
	 * Interrupts are still disabled. Do necessary setups, then
	 * enable them.
	 */
	boot_cpu_init();
	page_address_init();
	pr_notice("%s", linux_banner);
	early_security_init();
	setup_arch(&command_line);
	setup_boot_config();
	setup_command_line(command_line);
	setup_nr_cpu_ids();
	setup_per_cpu_areas();
	smp_prepare_boot_cpu();	/* arch-specific boot-cpu hooks */
	boot_cpu_hotplug_init();

	build_all_zonelists(NULL);
	page_alloc_init();

	pr_notice("Kernel command line: %s\n", saved_command_line);
	/* parameters may set static keys */
	jump_label_init();
	parse_early_param();
	after_dashes = parse_args("Booting kernel",
				  static_command_line, __start___param,
				  __stop___param - __start___param,
				  -1, -1, NULL, &unknown_bootoption);
	print_unknown_bootoptions();
	if (!IS_ERR_OR_NULL(after_dashes))
		parse_args("Setting init args", after_dashes, NULL, 0, -1, -1,
			   NULL, set_init_arg);
	if (extra_init_args)
		parse_args("Setting extra init args", extra_init_args,
			   NULL, 0, -1, -1, NULL, set_init_arg);

	/* Architectural and non-timekeeping rng init, before allocator init */
	random_init_early(command_line);

	/*
	 * These use large bootmem allocations and must precede
	 * kmem_cache_init()
	 */
	setup_log_buf(0);
	vfs_caches_init_early();
	sort_main_extable();
	trap_init();
	mm_init();
	poking_init();
	ftrace_init();

	/* trace_printk can be enabled here */
	early_trace_init();

	/*
	 * Set up the scheduler prior starting any interrupts (such as the
	 * timer interrupt). Full topology setup happens at smp_init()
	 * time - but meanwhile we still have a functioning scheduler.
	 */
	sched_init();

	if (WARN(!irqs_disabled(),
		 "Interrupts were enabled *very* early, fixing it\n"))
		local_irq_disable();
	radix_tree_init();
	maple_tree_init();

	/*
	 * Set up housekeeping before setting up workqueues to allow the unbound
	 * workqueue to take non-housekeeping into account.
	 */
	housekeeping_init();

	/*
	 * Allow workqueue creation and work item queueing/cancelling
	 * early.  Work item execution depends on kthreads and starts after
	 * workqueue_init().
	 */
	workqueue_init_early();

	rcu_init();

	/* Trace events are available after this */
	trace_init();

	if (initcall_debug)
		initcall_debug_enable();

	context_tracking_init();
	/* init some links before init_ISA_irqs() */
	early_irq_init();
	init_IRQ();
	tick_init();
	rcu_init_nohz();
	init_timers();
	srcu_init();
	hrtimers_init();
	softirq_init();
	timekeeping_init();
	time_init();

	/* This must be after timekeeping is initialized */
	random_init();

	/* These make use of the fully initialized rng */
	kfence_init();
	boot_init_stack_canary();

	perf_event_init();
	profile_init();
	call_function_init();
	WARN(!irqs_disabled(), "Interrupts were enabled early\n");

	early_boot_irqs_disabled = false;
	local_irq_enable();

	kmem_cache_init_late();

	/*
	 * HACK ALERT! This is early. We're enabling the console before
	 * we've done PCI setups etc, and console_init() must be aware of
	 * this. But we do want output early, in case something goes wrong.
	 */
	console_init();
	if (panic_later)
		panic("Too many boot %s vars at `%s'", panic_later,
		      panic_param);

	lockdep_init();

	/*
	 * Need to run this when irqs are enabled, because it wants
	 * to self-test [hard/soft]-irqs on/off lock inversion bugs
	 * too:
	 */
	locking_selftest();

#ifdef CONFIG_BLK_DEV_INITRD
	if (initrd_start && !initrd_below_start_ok &&
	    page_to_pfn(virt_to_page((void *)initrd_start)) < min_low_pfn) {
		pr_crit("initrd overwritten (0x%08lx < 0x%08lx) - disabling it.\n",
		    page_to_pfn(virt_to_page((void *)initrd_start)),
		    min_low_pfn);
		initrd_start = 0;
	}
#endif
	setup_per_cpu_pageset();
	numa_policy_init();
	acpi_early_init();
	if (late_time_init)
		late_time_init();
	sched_clock_init();
	calibrate_delay();

	arch_cpu_finalize_init();

	pid_idr_init();
	anon_vma_init();
#ifdef CONFIG_X86
	if (efi_enabled(EFI_RUNTIME_SERVICES))
		efi_enter_virtual_mode();
#endif
	thread_stack_cache_init();
	cred_init();
	fork_init();
	proc_caches_init();
	uts_ns_init();
	key_init();
	security_init();
	dbg_late_init();
	net_ns_init();
	vfs_caches_init();
	pagecache_init();
	signals_init();
	seq_file_init();
	proc_root_init();
	nsfs_init();
	cpuset_init();
	cgroup_init();
	taskstats_init_early();
	delayacct_init();

	acpi_subsystem_init();
	arch_post_acpi_subsys_init();
	kcsan_init();

	/* Do the rest non-__init'ed, we're now alive */
	arch_call_rest_init();

	prevent_tail_call_optimization();
}

----------------------------------------------------------------------------------------
- EXECVE -

execve() @syscall
   |
   +- do_execve()
          |
          +- do_execveat_common()
            |
            :
            +- alloc_bprm()
            |   	|
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
                             +----------------------------------------+
                             | Insert vm structure into process list  |
                             | sorted by address and into the inode's |
                             | i_mmap tree.  If vm_file is non-NULL   |
                             | then i_mmap_rwsem is taken here.       |
                             +----------------------------------------+
                                                |
                                                +- find_vma_intersection()
                                                   Look up the first VMA which
                                                   intersects the interval
                                                          |
                                                          +- mt_find()
                                                                |
                                                                +-> mm->mm_mt
                                                |
                                                +- vma_link()
                                                      |
                                                      :
                                                      +- if vma->vm_file
                                                             |not NULL
                                                             :
                                                             +- vma->vm_file->f_mapping
                                                                     :
                                                                     |not NULL
                                                                     +- __vma_link_file()
                                                                               |
                             +<------------------------------------------------+
                             |
                             v
        vma_interval_tree_insert(vma, &mapping->i_mmap)
            (interval tree - rbtree implemented)

[x]
 :
 +- bprm_execve()
         |
         :
         +- sched_exec()
         |
         +- exec_binprm()

@maple tree

----------------------------------------------------------------------------------------

[+] kernel/fork.c

static struct mm_struct *mm_init(struct mm_struct *mm, struct task_struct *p,
	struct user_namespace *user_ns)
{
	mt_init_flags(&mm->mm_mt, MM_MT_FLAGS);
	mt_set_external_lock(&mm->mm_mt, &mm->mmap_lock);
	atomic_set(&mm->mm_users, 1);
	atomic_set(&mm->mm_count, 1);
	seqcount_init(&mm->write_protect_seq);
	mmap_init_lock(mm);
	INIT_LIST_HEAD(&mm->mmlist);
	mm_pgtables_bytes_init(mm);
	mm->map_count = 0;
	mm->locked_vm = 0;
	atomic64_set(&mm->pinned_vm, 0);
	memset(&mm->rss_stat, 0, sizeof(mm->rss_stat));
	spin_lock_init(&mm->page_table_lock);
	spin_lock_init(&mm->arg_lock);
	mm_init_cpumask(mm);
	mm_init_aio(mm);
	mm_init_owner(mm, p);
	mm_pasid_init(mm);
	RCU_INIT_POINTER(mm->exe_file, NULL);
	mmu_notifier_subscriptions_init(mm);
	init_tlb_flush_pending(mm);
#if defined(CONFIG_TRANSPARENT_HUGEPAGE) && !USE_SPLIT_PMD_PTLOCKS
	mm->pmd_huge_pte = NULL;
#endif
	mm_init_uprobes_state(mm);
	hugetlb_count_init(mm);

	if (current->mm) {
		mm->flags = current->mm->flags & MMF_INIT_MASK;
		mm->def_flags = current->mm->def_flags & VM_INIT_DEF_MASK;
	} else {
		mm->flags = default_dump_filter;
		mm->def_flags = 0;
	}

	if (mm_alloc_pgd(mm)) ----------------------> pgd_alloc() [+] arch/arm64/mm/pgd.c
		goto fail_nopgd;                              |
                                                      +-if PGD_SIZE == PAGE_SIZE
                                                                 |yes
                                                                 +- __get_free_page()
                                                                 *
                                                                 |no
                                                                 +- kmem_cache_alloc()
                                                                       (pgd_cache)
	if (init_new_context(p, mm))
		goto fail_nocontext;

	mm->user_ns = get_user_ns(user_ns);
	lru_gen_init_mm(mm);
	return mm;

fail_nocontext:
	mm_free_pgd(mm);
fail_nopgd:
	free_mm(mm);
	return NULL;
}


[+] include/linux/mm.h

static inline void vma_init(struct vm_area_struct *vma, struct mm_struct *mm)
{
	static const struct vm_operations_struct dummy_vm_ops = {};

	memset(vma, 0, sizeof(*vma));
	vma->vm_mm = mm;
	vma->vm_ops = &dummy_vm_ops;
	INIT_LIST_HEAD(&vma->anon_vma_chain);
}

----------------------------------------------------------------------------------------
Reference:
[1] Application Note Bare-metal Boot Code for ARMv8-A Processors
----------------------------------------------------------------------------------------
