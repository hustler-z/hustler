+--------------------------------------------------------------------------------------+
| BOOTING PROCESS for Linux                                                            |
+--------------------------------------------------------------------------------------+



----------------------------------------------------------------------------------------
- START KERNEL -

[+] init/main.c
asmlinkage __visible void __init __no_sanitize_address start_kernel(void)

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
                                                          |
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
                             +<-----------------------------------------+
                             :
                             v
        vma_interval_tree_insert(vma, &mapping->i_mmap)
        (interval tree - rbtree implemented)
        @note: mm->map_count++ in vma_link()

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

	if (mm_alloc_pgd(mm)) --------------> pgd_alloc() [+] arch/arm64/mm/pgd.c
                goto fail_nopgd;                |
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
