+--------------------------------------------------------------------------------------+
| Refer to Linux-6.1.63                                                                |
+--------------------------------------------------------------------------------------+

BASICS
----------------------------------------------------------------------------------------
ARMv8 MMU and Caches

A cache is a small, fast block of memory that sits between the core and main memory. A
cache line is the smallest loadable unit of a cache, a block of contiguous words from
main memory under the same tag.

          +---------+   +---------+
L1 Caches | D-cache | + | I-cache | -> For each core
          +---------+   +---------+
               |             |
          +-----------------------+
L2 Caches |          ...          | -> Share between cores in a cluster
          +-----------------------+
                      |
          +-----------------------+
L3 Caches |          ...          | -> Share between clustlers
          +-----------------------+
                      |
          +-----------+----------------+--------------> BUS
                                       |
                            +----------------------+
                            |      Main Memory     |
                            +----------------------+

The MMU translates the virtual addresses of code and data to the physical addresses in
the actual hardware system. The translation is carried out automatically in hardware and
is transparent to the application. In addition to address translation, the MMU controls
memory access permissions, memory ordering, and cache policies for each region of memory.

              +------------------------------+
+----------+  | MMU (Memory Management Unit) |  +--------+
| ARM Core |->| a) TLBs                      |->| Caches |
+----------+  | b) Table Walk Unit           |  +--------+
              +------------------------------+            \
                                                           \
                                                +--------------------+
                                                | MEMORY             |
                                                | Translation Tables |
                                                +--------------------+

The Translation Lookaside Buffer (TLB) is a cache of recently accessed page translations
in the MMU. For each memory access performed by the processor, the MMU checks whether
the translation is cached in the TLB. If the requested address translation causes a hit
within the TLB, the translation of the address is immediately available.

Each TLB entry typically contains not just physical and Virtual Addresses, but also
attributes such as memory type, cache policies, access permissions, the Address Space ID
(ASID), and the Virtual Machine ID (VMID).

If the TLB does not contain a valid translation for the Virtual Address issued by the
processor, known as a TLB miss, an external translation table walk or lookup is
performed. Dedicated hardware within the MMU enables it to read the translation tables
in memory. The newly loaded translation can then be cached in the TLB for possible reuse
if the translation table walk does not result in a page fault.

TLB Invalidate Instruction

TLBI <tlbi_op> {, <Xt>}

For a change of a single TLB entry:
tlbi vae1, <Xt>
which invalidates an entry associated with the address specified in the register Xt.

[VA]

63                                                                 0
+-------------+---------+---------------+---------------+----------+
| TTBR select |   ***   | Level 3 index | level 4 index | PA[15:0] |
+-------------+---------+---------------+---------------+----------+
       |                        |                   |            |
       |         Index in table |                   |            |
   +-------+           +--------+                   |            |
   | TTBRx |           |  63         0     +--------+            |
   +-------+           |  +----------+     |  63         0       |
       |               |  :          :     |  +----------+       |
       |               |  |          |     |  :          :       |
       |               |  +---+----+-+     |  |          |       |
       |               +->|   | BA |--->+  |  +---+----+-+       |
       |                  +---+----+-+  |  +->|   | BA |-----+   |
       |  Page Table BA   |          |  |     +---+----+-+   |   |
       +------ *** ------>+----------+  |     |          |   |   |
                          L2 page table +---->+----------+   |   |
                                              L3 page table  |   |
                                                             |   |
                                            +----------------+   |
[PA]                                        |                    |
                                            v                    v
+--------------------------------+----------------------+----------+
|                                |       PA[47:16]      | PA[15:0] |
+--------------------------------+----------------------+----------+

Context Switching

Context switch requires the kernel to save all execution state associated with the
process and to restore the state of the process to be run next. The kernel also switches
translation table entries to those of the next process to be run. The memory of the
tasks that are not currently running is completely protected from the task that is
running.

Save and restore elements:
    • general-purpose registers X0-X30.
    • Advanced SIMD and Floating-point registers V0-V31.
    • Some status registers.
    • TTBR0_EL1 and TTBR0.
    • Thread Process ID (TPIDxxx) Registers.
    • Address Space ID (ASID).

For EL0 and EL1, there are two translation tables. TTBR0_EL1 provides translations for
the bottom of Virtual Address space, which is typically application space and TTBR1_EL1
covers the top of Virtual Address space, typically kernel space.

TTBR0_EL1 -> Userspace
TTBR1_EL1 -> Kernelspace

----------------------------------------------------------------------------------------

Memory Layout on AArch64 Linux

    +----------------------------------------------------------------------------------+
    | Start               End                      Size      Use                       |
    | ---------------------------------------------------------------------------------+
    | 0000000000000000    0000ffffffffffff         256TB     user                      |
    | ffff000000000000    ffff7fffffffffff         128TB     kernel logical memory map |
    |[ffff600000000000    ffff7fffffffffff]         32TB     [kasan shadow region]     |
    | ffff800000000000    ffff800007ffffff         128MB     modules                   |
    | ffff800008000000    fffffbffefffffff         124TB     vmalloc                   |
    | fffffbfff0000000    fffffbfffdffffff         224MB     fixed mappings (top down) |
    | fffffbfffe000000    fffffbfffe7fffff           8MB     [guard region]            |
    | fffffbfffe800000    fffffbffff7fffff          16MB     PCI I/O space             |
    | fffffbffff800000    fffffbffffffffff           8MB     [guard region]            |
    | fffffc0000000000    fffffdffffffffff           2TB     vmemmap                   |
    | fffffe0000000000    ffffffffffffffff           2TB     [guard region]            |
    +----------------------------------------------------------------------------------+

Translation table lookup with 4KB pages

    +--------+--------+--------+--------+--------+--------+--------+--------+
    |63    56|55    48|47    40|39    32|31    24|23    16|15     8|7      0|
    +--------+--------+--------+--------+--------+--------+--------+--------+
     |                 |         |         |         |         |
     |                 |         |         |         |         v
     |                 |         |         |         |   [11:0 ] in-page offset
     |                 |         |         |         +-> [20:12] L3 index
     |                 |         |         +-----------> [29:21] L2 index
     |                 |         +---------------------> [38:30] L1 index
     |                 +-------------------------------> [47:39] L0 index
     +-------------------------------------------------> [63]    TTBR0/1

ARMv8.5 based processors introduce the Memory Tagging Extension (MTE) feature. MTE is
built on top of the ARMv8.0 virtual address tagging TBI (Top Byte Ignore) feature and
allows software to access a 4-bit allocation tag for each 16-byte granule in the
physical address space.

----------------------------------------------------------------------------------------

Memory attributes and properties are a way of defining how memory behaves.

                              +---------> Device Memory
                              |
                     +------------------+                        -+
Non-cached Ordered - |   Peripherals    |                         |
                     +------------------+                         |
                     |   Kernel Data    |                         +-- Privileged
         cacheable - +------------------+                         |   Access Only
                     |   Kernel Code    | - Read-only Executable  |
                     +------------------+                        -+
                     |                  |
                     :                  :
                     |                  |                        -+
                     +------------------+                         |
                     | Application Data |                         |
         cacheable - +------------------+                         +-- Unprivileged
                     | Application Data |                         |
                     +------------------+                        -+

Peripheral registers => Memory-Mapped I/O (MMIO)

----------------------------------------------------------------------------------------
- HUGE PAGES -

To avoid spending precious processor cycles on the address translation, CPUs maintain a
cache of such translations called Translation Lookaside Buffer (or TLB).

Huge Pages significantly reduces pressure on TLB, improves TLB hit-rate and thus improves
overall system performance.

1) HugeTLB filesystem (hugetlbfs), a pseudo filesystem that uses RAM as its backing store.
2) Transparent HugePages (THP)

----------------------------------------------------------------------------------------
- COMPOUND PAGES -

A compound page with order N consists of 2^N physically contiguous pages. A compound
page with order 2 takes the form of "HTTT", where H donates its head page and T donates
its tail page(s). The major consumers of compound pages are hugeTLB pages (HugeTLB Pages),
the SLUB etc. memory allocators and various device drivers.

PageHead   PageTails
  /          /
+-+- ~ ~ -+-+
|x|       |x|
+-+- ~ ~ -+-+
      |
      +-> 2^N Physically Contiguous Pages

with __GFP_COMP, alloc_pages() can allocate compound pages.

@include/linux/page-flags.h

1) PageComound()
        |
        +- page->flags or page->compound_head
                    |
                    +- PG_head (@enum pageflags)

2) PageHead()
3) PageTail()

Above functions can verify if pages are compound/page heads/tails.

----------------------------------------------------------------------------------------
- MMAP -

mmap() @arch/arm64/kernel/sys.c
  |
  +- ksys_mmap_pgoff() @mm/mmap.c
            |
            +- vm_mmap_pgoff() @mm/util.c
                     |
                     +- security_mmap_file()
                               |ret=0
                               +- do_map()
                                     |
                                     +- get_unmapped_area()
                                     |  Obtain the address to map to:
                                        a) From current->mm->get_unmapped_area
                                        b) From shmem_get_unmapped_area
                                     |
                                     +- flags & MAP_FIXED_NOREPLACE
                                                    |!=0x0
                                                    +- find_vma_intersection()
                                                       Look up the first VMA with
                                                       intersects the interval.

                                     |
                                     +- mmap_region()
                                        |     |     |
                         +--------------+     |     +---------------+
                         |                    |                     |
                         v                    v                     v
                 a) file mappings    b) anonymous mapping    c) shared mapping
                    |                   |                       |
                    +- call_mmap()      +- vma_set_anonymous()  +- shmem_zero_setup()
                         |                    |                     |
                         +--------------------+---------------------+
                                              |
                                              v
                                vma_interval_tree_insert() @mm/interval_tree.c

                                   ●
                                  / \
                                 ○   ○
                                / \ / \

                                Note:
                                struct vm_area_sruct {
                                    ...
                                    union {
                                        struct {
                                            struct rb_node rb; -------------->+
                                            unsigned long rb_subtree_last;    |
                                        } shared;                             |
                                        ...                                   |
                                    };                                        |
                                    ...                                       |
                                };                                            |
                                                                              |
                                struct address_space {                        |
                                    ...                                       |
                                    ...                                       v
                                    struct rb_root_cached i_mmap; <-----------+
                                    ...
                                };

----------------------------------------------------------------------------------------
- SHMEM -

shmem_zero_setup() @mm/shmem.c
        |
        +- shmem_kernel_file_setup()
           Get an unlinked file living in tmpfs which must be kernel internal.
                        |
                        +- __shmem_file_setup()
                                    |
                                    +- shmem_get_inode()


                                    |
                                    +- ramfs_nommu_expand_for_mapping()
                                       Add a contiguous set of pages into ramfs inode
                                       when it's truncated from size 0 on the
                                       assumption that it's going to be used for an
                                       mmap of shared memory.
                                                     |res=0
                                                     +- alloc_file_pseudo()

----------------------------------------------------------------------------------------
- VIRTUALLAY CONTIGUOUS MEMORY -

Tips:

@include/linux/mm.h

0) page_to_virt()
1) vmalloc_to_page()
2) vmalloc_to_pfn()

struct vm_struct - kernel vmalloc area descriptor

struct vmap_block -> va
                      |
                      v
             struct vmap_area
             [va_start:va_end]

vmap_area_list
purge_vmap_area_list
free_vmap_area_list

vmap_area_root       - 'busy' vmap area rb-tree root
purge_vmap_area_root -  purge vmap area rb-tree root
free_vmap_area_root  - 'free' vmap area rb-tree root

@mm/vmalloc.c

vmalloc() - Allocate virtually contiguous memory with given size
   |
   |          +<<- [3]
   |          |
   +- __vmalloc_node()
              |
              +- __vmalloc_node_range()
                            |
                            +- __get_vm_area_node()
                           [2]          |
                                        |- ...
                                        | 
                                        +- alloc_vmap_area()
                                       [1]         |
                                                   v
                                           Allocate a region of KVA of the
                                           specified size and alignment,
                                           within [vstart, vend]
                                                   |
                        free_vmap_area_root/list ->+- __alloc_vmap_area()
                                  |                          |
                                  +------------------------>>+- find_vmap_lowest_match()
                                                             |             |
                                                            [5]            |
                                                                           v
                                                                Find the first free block
                                                                (lowest start address) in
                                                                the free_vmap_area_root
                                                                rb-tree.


                                                            [5]
                                                             |
                                                             +- adjust_va_to_fit_type()
                                                                         |
                                                                         v
                                                              Update the free vmap_area

                                       [1]
                                        |
                  vmap_area_root/list ->+- insert_vmap_area()
                          |                         |
                          |                         |- find_va_links()
                          |                         |
                          +----------------------->>+- link_va()

  [2]
   |
   +- __vmalloc_area_node() - Allocate physical pages and map them into vmalloc space.
                |
                +- (array_size > PAGE_SIZE) ->> [3] recursion
               [7]           |no
                             +- kmalloc_node() @include/linux/slab.h
                                      |
                                      +- __kmalloc_node()
                                                 |
                                                 +- __do_malloc_node()
                                                    @mm/slab_common.c
               [7]
                |
                +- vm_area_alloc_pages()
               [4]          |
                            +- When order=0, use bulk allocator
                           [6]                  |
                                                v
                               1) alloc_pages_bulk_array_mempolicy() @mm/mempolicy.c
                                                |
                                                +- consider mempolicy
                                                           |
                                                           +- __alloc_pages_bulk()
                                                              @mm/page_alloc.c
                                                                     |
                               2) alloc_pages_bulk_array_node()      |
                                               |                     |
                                               +- consider node id   |
                                                          |          |
                                                          +--------->+

                           [6]
                            |
                            +- When order>0, or bulk allocator fails.
                               1) alloc_pages()
                               2) alloc_pages_node()
                               Then split_page()
                                         |
                                         +- Takes a non-compound higher-order page,
                                            and splits it into n (1 << order) sub-
                                            pages: page[0..n].

               [4]
                |
                +- Map pages to a kernel virtual address
                   vmap_pages_range()
                           |
                           +- vmap_pages_range_noflush()
                                         |
                                         +- __vmap_pages_range_noflush()
                                                        |
                                                        +- vmap_range_noflush()
                                                                   |
                                                      To build page table [layout]
                                                                   |
                                                                   v
            +-------------+-------------+------------+-------------+-------------+ 
            |     PGD     |     P4D     |     PUD    |     PMD     |     PTE     |
            +-------------+-------------+------------+-------------+-------------+
                   |             |             |            |             |
                   |             |             |            |             |
                   +- - - - - - >|             |            |             |
             vmap_p4d_range      |             |            |             |
                                 +- - - - - - >|            |             |
                           vmap_pud_range      |            |             |
                                               +- - - - - ->|             |
                                         vmap_pmd_range     |             |
                                                            +- - - - - - >|
                                                      vmap_pte_range      |
                                                                          v
                                                                        [END]

VMALLOC_SPACE

+----------------+ - VMALLOC_START
|                |
|                |
|                |
:                :
|                |
|                |
|                |
+----------------+ - VMALLOC_END

Kernel space and User space have separate translation tables.

* Translation Control Register (TCR_EL1) - EL1&0 Translation Regime

+----------------+--------------+
|                |       |
|                |
| Kernel Space   | tcr_el1.t1sz
|                |
:                :       |
+----------------+--------------+
:                :       |
|                |
|                |
|                |
|                |
|                |
| User Space     | tcr_el1.t0sz
|                |
|                |
|                |
|                |
|                |
|                |       |
+----------------+-------------+

The size offset of the memory region addressed by TTBR1_EL1. The region size is
2^(64-T1SZ) bytes. The maximum and minimum possible values for T1SZ depend on the
level of translation table and the memory translation granule size.

Two Stage Translations

Stage 1 Translation                       Stage 2 Translation (Control by Hypervisor)
      |                                           |
      v                                           v
VA <--+--> IPA (Intermediate Physical Address) <--+--> PA

For Non-secure EL1/0 accesses, these must be explicitly enabled by writing to the
Hypervisor Configuration Register HCR_EL2.

+-------------------+    +--------------------+    +-------------+
|      OS (EL1)     |    | Guest OS           |    | Peripherals |    +-------------+
+-------------------+--->| Translation Tables |--->+-------------+    | Translation |
| Application (EL0) |    | TTBRn_EL1          |    |    Flash    |--->| tables      |
+-------------------+    +--------------------+    +-------------+    | VTTBR0_EL2  |
 Virtual memory map                                |     RAM     |    +-------------+
 Under control of                                  +-------------+           |
 guest OS                                                                    |
                                                                             | VTCR_EL2
                                                                             |
                         +--------------------+                              |
+-------------------+    | Hypervisor         |           TCR_EL2            v
| Hypervisor (EL2)  |--->| Translation Tables |----------------------------->+
+-------------------+    | TTBR0_EL2          |                              |
                         +--------------------+                              |
                                                                             |
                         +--------------------+                              |
+-------------------+    | Secure Monitor     |           TCR_EL3            v
|Secure Monitor(EL3)|--->| Translation Tables |----------------------------->+
+-------------------+    | TTBR0_EL3          |                              |
 Virtual memory space    +--------------------+                              |
 seen by Hypervisor                                                          |
 and Secure Monitor                                                          v
                                                              +--------------+
                                                              |
                                                              |       +-------------+
                                                              |       | Peripherals |
                                                              |       +-------------+
                                                              |       |     RAM     |
                                                              |       +-------------+
                                                              +------>| Peripherals |
                                                                      +-------------+
                                                                      |     RAM     |
                                                                      +-------------+
                                                                      |    Flash    |
                                                                      +-------------+
                                                             Real physical memory map

----------------------------------------------------------------------------------------
- REVERSE MAPPING (RMAP) -

 physical pages   PTEs
      |            |
 (struct page) --->+


  malloc()
   |                                                                          User Space
----------------------------------------------------------------------------------------
   |                                                                        Kernel Space
  brk() - syscall @mm/mmap.c
   |
   +- if brk <= mm->brk
             |
             +- do_brk_munmap() -> Unmap a partial vma.

   |
   +- check_brk_limits()
   |
   +- do_brk_flags() -> Extend the brk VMA from addr to addr + len. If the VMA is
           |            NULL or the flags do not match then create a new anonymous
           |            VMA.
           |
           +- (1) vma->anon_vma
                         |
                         +- anon_vma_interval_tree_pre_update_vma()
                         |
                         +- anon_vma_interval_tree_post_update_vma()
           |
           +- (2) anonymous mapping
                         |
                         +- vm_area_alloc()
                                  |
                                  +- kmem_cache_alloc()
                                            |
                                            +- vm_area_cachep
                                  |
                                  +- vma_init()
                         |
                         +- vma_set_anonymous() -> set vma->vm_ops to NULL.

   |
   +- userfaultfd_unmap_complete()
   |
   +- if populate
            |
            +- mm_populate()

----------------------------------------------------------------------------------------
- IOREMAP -


ioremap()
   |
   +- ioremap_prot()
        |
        +- ioremap_allowed()
                |yes
                +- get_vm_area_caller()
                |          |
                |          +- __get_vm_area_node()
                |
                +- ioremap_page_range()
                           |
                           +- vmap_range_noflush()
                           |
                           +- flush_cache_vmap()

----------------------------------------------------------------------------------------
- DMA -





----------------------------------------------------------------------------------------
- ADDRESS TRANSLATION -

a) Address Space Identifiers (ASIDs)
            |
            +- Tagging translations with the owning process

TLB entries for multiple processes are allowed to coexist in the cache, and the ASID
determines which entry to use.

b) Virtual Machine Identifiers (VMIDs)
            |
            +- Tagging translations with the owning VM

VMIDs allow translations from different VMs to coexist in the cache.

MMU - Memory Management Unit

* The table walk unit - contains logic that reads the translation tables from memory.
* Translation Lookaside Buffers (TLBs) - cache recently used translations.

Translation Table Base Registers

ttbr0_el1
ttbr1_el1
   |
   +- Holds the base address of the translation table for the initial lookup for stage 1
      of the translation of an address from the higher VA range in the EL1&0 stage 1
      translation regime, and other information for this translation regime.

----------------------------------------------------------------------------------------
- BOOTTIME MEMORY MANAGEMENT -

start_kernel() @init/main.c
      |
      +- page_address_init() @mm/highmem.c
      |
      +- setup_arch() @arch/arm64/kernel/setup.c
              |
              +- paging_init() @arch/arm64/mm/mmu.c
              |       |
             [0]      +- map_kernel
                      |
                      +- map_mem

             [0]
              |
              +- bootmem_init() @arch/arm64/mm/init.c
                      |
                      +- early_memtest()
                      |
                      +- arch_numa_init()
                      |
                      +- arm64_hugetlb_cma_reserve()
                      |  Reserve CMA areas for the largest supported gigantic huge page
                      |  when requested.
                      |
                      +- dma_pernuma_cma_reserve()
                      |
                      +- kvm_hyp_reserve()
                      |
                      +- sparse_init()
                      |
                      +- zone_sizes_init()
                      |         |
                     [1]        +- free_area_init() @mm/page_alloc.c
                                          |
                                          +- free_area_init_node()
                                                [To each node]
                                                      |
                                                      +- free_area_init_core()
                                                             |
                                                             +- pgdat_init_internals()
                                                             |
                                                             +- zone_init_internals()
                                                                   [To each zone]
                     [1]              
                      |
                      +- dma_contiguous_reserve()

----------------------------------------------------------------------------------------
- MEMBLOCK -

Memblock is a method of managing memory regions during the early boot period when the
usual kernel memory allocators are not up and running.

----------------------------------------------------------------------------------------
- BUDDY SYSTEM -

+--------------------------------------------------------------------------------------+
| @Configuration                                                                       |
| CONFIG_ARM64_PAGE_SHIFT=12                                                           |
|                                                                                      |
| @arch/arm64/include/asm/page-def.h                                                   |
| #define PAGE_SHIFT          CONFIG_ARM64_PAGE_SHIFT                                  |
| #define PAGE_SIZE           (_AC(1, UL) << PAGE_SHIFT)                               |
|                                                                                      |
| @include/linux/mm.h                                                                  |
| #define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)                                      |
|                                                                                      |
| @include/linux/align.h                                                               |
| #define ALIGN(x, a)      __ALIGN_KERNEL((x), (a))                                    |
|                                                                                      |
| @include/uapi/linux/const.h                                                          |
| #define __ALIGN_KERNEL(x, a)   __ALIGN_KERNEL_MASK(x, (__typeof__(x))(a) - 1)        |
| #define __ALIGN_KERNEL_MASK(x, mask)  (((x) + (mask)) & ~(mask))                     |
|                                                                                      |
| => (x + a - 1) & (~(a - 1))                                                          |
+--------------------------------------------------------------------------------------+

nr_free_buffer_pages() - count number of pages beyond high watermark
          |
          +- nr_free_zone_pages() -> sum of [managed_pages - high_pages] of all zones


si_mem_available()
         |
         +-> Userspace Allocated Memory + Page Cache + Reclaimable Kernel Memory

(1) CONFIG_NUMA=y

alloc_pages() 
     |
     +- 1) pol->mode == MPOL_INTERLEAVE
     |                |
     |     alloc_page_interleave()
     |
     +- 2) pol->mode == MPOL_PREFERRED_MANY
     |                |
     |     alloc_pages_preferred_many()
     |
     +- 3) __alloc_pages()

(2) CONFIG_NUMA=n

alloc_pages()
     |
     +- alloc_pages_node()
                |
                +- __alloc_pages_node()
                             |
                             +- __alloc_pages()

$ cat /proc/pagetypeinfo

struct free_area

+------------+    +------------------+    +---------------------+
| order = 0  | -> | struct free_list | -> | MIGRATE_UNMOVABLE   |
+------------+    +------------------+    +---------------------+
|            |                            | MIGRATE_MOVABLE     |
:            :                            +---------------------+
|            |                            | MIGRATE_RECLAIMABLE |
+------------+                            +---------------------+
| order = 11 |                            | MIGRATE_HIGHATOMIC  |
+------------+                            +---------------------+
                                          | MIGRATE_ISOLATE     |
                                          +---------------------+

__alloc_pages() with struct alloc_context ac instantiated
       |
       +- prepare_alloc_pages()
       |           |
      [2]          +- Initialize the alloc_context structure

      [2]    
       |
       +- get_page_from_freelist()
       |           |
      [3]          +- 1) scan zonelist, looking for a zone with enough free pages.
                         a. Check the watermark via zone_watermark_fast()
                            if false, try to reclaim via node_reclaim() when needed;
                            if true, try to allocate from current zone;
                         b. Allocate pages from given zone via rmqueue()
                               |
                           [x]-+- When order=0, allocate via rmqueue_pcplist()
                                          |
                                          +-

                           [x]-+- When order>0, allocate via rmqueue_buddy()
                                          |
                                          +- Try __rmqueue_smallest() first
                                          |           |
                                         [4]          +- Go through the free lists for
                                                         the given migratetype and remove
                                                         the smallest available page from
                                                         the freelist
                                                         a) get_page_from_free_area()
                                                         b) del_page_from_free_list()
                                                         c) if current order doesn't meet,
                                                         Go to higher order, if available,
                                                         split them into half, put a half
                                                         back to lower order of free area,
                                                         another half as requested.
                                                         - expand()
                                         [4]
                                          |
                                          +- if fails, do __rmqueue()
                                                               |
                                                               +- CONFIG_CMA=y
                                                                  __rmqueue_cma_fallback()
                                                               
                                                               |
                                                               +- Try __rmqueue_smallest()
                                                                  again. If fails, then do
                                                                  1) __rmqueue_cma_fallback()
                                                                  2) __rmqueue_fallback()
                                                                             |
                                                                             v
                             Try finding a free buddy page on the fallback list and put it
                             on the free list of requested migratetype.
                             a) find_suitable_fallback() to find largest/smallest available
                                free page in other list.
                             Note: fallback lists can be (depends on current migratetype,
                             other two are fallbacks) two of below:
                                1) MIGRATE_UNMOVABLE
                                2) MIGRATE_MOVABLE
                                3) MIGRATE_RECLAIMABLE
                             -------------------------------------------------------------
                             b) get_page_from_free_area()
                             c) steal_suitable_fallback()
                                        |
                                        +- when whole_block=true, do move_freepages_block()
                                                                                 |
                                           Move the free pages in a range to the freelist
                                           tail of the requested type via move_freepages()
                                                                                 |
                                                               loop to move_to_free_list()

                           [x]-+- Test on zone->flags - ZONE_BOOSTED_WATERMARK
                                                                   |
                                                                   +- wakeup_kswapd()


      [3]
       |
       +- __alloc_pages_slowpath()
                   |
                   +- try get_page_from_freelist() <--------------+
                   |  Since alloc_flags changed                   |
                   |                                             [5]
                   +- __alloc_pages_direct_compact()
                   |  If costly order allocation
                   |
             +---->+- alloc_flags & ALLOC_KSWAPD
             |     |  Then wake_all_kswapds()
            [6]    |              |
                   |              +- wakeup_kswapd()
                   |                 on each zone
                   |
                   +- try get_page_from_freelist()
                   |  Since adjusted zonelist and alloc_flags
                   |
                   +- __alloc_pages_direct_reclaim()
                   |               |
                  [7]              +- __perform_reclaim()
                                   |           |
                                               +- try_to_free_pages() @mm/vmscan.c
                                   |
                                   +- get_page_from_freelist()
                                   |
                                   +- drain_all_pages()
                                      When above fails, try to spill all the per-cpu
                                      pages from all CPUs back into the buddy allocator
                                             |
                                             +- __drain_all_pages()
                                                Optimized to only execute on CPUs where
                                                pcplists are not empty, with these CPUs
                                                    |
                                                    +- drain_pages_zone()
                                                           |
                                                           +- free_pcppages_bulk()

                  [7]
                   |
                   +- __alloc_pages_direct_compact()
                   |                |
                  [8]               +- try_to_compact_pages() @mm/compaction.c
                                    |
                                    +- get_page_from_freelist()
                                    |
                                    +- compaction_defer_reset()
                                       Update defer tracking counters after successful
                                       compaction of given order.
            [6]   [8]
             |     |
             +<----+- should_reclaim_retry()
             |     |
             +<----+- should_compact_retry()
                   |
                   +- check_retry_cpuset()                       [5]
                      OR                                          |
                      check_retry_zonelist() -------------------->+
 

                   |
                   +- __alloc_pages_may_oom()

                   |
                   +- When hit nopage
                      __alloc_pages_cpuset_fallback()
                      
----------------------------------------------------------------------------------------
- MEMPOOL -

@include/linux/mempool.h

a) mempool_create_slab_pool()
b) mempool_create_kmalloc_pool()
c) mempool_create_page_pool()

mempool_create() @mm/mempool.c
       |
       +- mempool_create_node()
                    |
                    +- kzalloc_node()
                            | mempool_t *pool
                            +- mempool_init_node()
                                        |
                                        +- kmalloc_array_node()
                                                   | pool->elements
                                                   +- For each element, do
                                                      pool->alloc()
                                                              |
                                                              +- Decided by the type
                                                                 of mempool

                                                   |
                                                   +- Then add_element() to
                                                      the mempool

----------------------------------------------------------------------------------------
- PAGE RECLAIM -

try_to_free_pages() with struct scan_control sc initialized
        |
        +- do_try_to_free_pages() as the main entry point to direct page reclaim
                     |
                     +- sc->proactive --------+
                             |yes             |no
                        shrink_zones()        +- vmpressure_prio()
                             |
                             +- shrink_node()
                                      |
                                      +- prepare_scan_count()
                                      |
                                      +- shrink_node_memcgs()
                                                  |
                                                  +- mem_cgroup_iter() iterate the memcgs
                                                  |                                  |
                                                  +- mem_cgroup_lruvec()             |
                                                  |                                  |
                                                  +- shrink_lruvec()                 |
                                                  |                                  |
                                                  +- shrink_slab()                   |
                                                  |                                  |
                                                  +- sc->proactive                   |
                                                         |no                         |
                                                     vmpressure() ------------------>+

@include/linux/mmzone.h

typedef struct pglist_data {
    ...
    struct lruvec __lruvec; ---------+
    ...                              |
} pg_data_t;                         |
                                     \
                                       struct lruvec {
                                            struct list_head lists[NR_LRU_LISTS];
                                            ...                         |
                                       };                               |
                                                                        |
                                                                        /
                                                            +-------------------+
                                                            | LRU_INACTIVE_ANON |
                                                            +-------------------+
                                                            | LRU_ACTIVE_ANON   |
                                                            +-------------------+
                                                            | LRU_INACTIVE_FILE |
                                                            +-------------------+
                                                            | LRU_ACTIVE_FILE   |
                                                            +-------------------+
                                                            | LRU_UNEVICTABLE   |
                                                            +-------------------+

mem_cgroup_lruvec() to get the LRU list vector for a memcg & node
        |
        +- a) (struct pglist_data) pgdat->__lruvec
           b) (struct mem_cgroup_per_node) mz->lruvec

shrink_lruvec()
      |
      +- shrink_list()
               |
               +- is_active_lru() ---------------------------------------------------+
                       |no                                                           |
                       +- shrink_inactive_list()                  shrink_active_list()
                             |
                             +- Isolate page from the lruvec
                             |  to fill in @dst list by
                                nr_to_scan times via
                                isolate_lru_folios()
                                         |
                                         +-

                             |
                             +- shrink_folio_list()
                             |           |
                                         +-

                             |
                             +- Move folios from private @list
                             |  to appropriate LRU list via
                                move_folios_to_lru()
                                         |
                                         +-
shrink_slab()
      |
      +- shrink_slab_memcg()
      |

      |
      +- go through shrinker in shrinker list
         do_shrink_slab()

----------------------------------------------------------------------------------------
- Control Group -

cgroup is a mechanism to organize processes hierarchically and distribute system resources
along the hierarchy in a controlled and configurable manner.

cgroups form a tree structure and every process in the system belongs to one and only one
cgroup.


----------------------------------------------------------------------------------------
- PAGE SWAP -

wakeup_kswapd() @mm/vmscan.c
      |
      +- Check if pgdat->kswapd_wait is active ---> [END]
                         |
                         |yes
                         |
                         +- a) When kswapd fails over MAX_RECLAIM_RETRIES  ---->+
                            b) When have enough free memory available, but      |
                               too fragmented for high-order allocations.  ---->+
                                |                                               |
                                +- pgdat_balanced()                             |
                                                                                |
                                                            Not __GFP_DIRECT_RECLAIM
                                                                                |
                                                                                v
                                                                  wakeup_kcompactd()

                         |
                         +- wake_up_interruptible(&pgdat->kswapd_wait)
                                                               |
             +<------------------------------------------------+
             |
The Background Pageout Daemon
             |
             +- kswapd()
                   |
                   +- [Infinite Loop]
                             |
                             +- kswapd_try_to_sleep()
                             |
                             +- For kswapd, it will reclaim pages across a node
                                from zones that are eligible for use by the caller
                                until at least one zone is balanced.
                                      |
                                      v
                                balance_pgdat() with struct scan_control sc initialized
                                      |
                    +---------------->+
                    |                 |
                    |                 +- pgdat_balanced() ------------------+
                    |                 |                                     |
                    |                 +- kswapd_shrink_node()               |
                    |  sc.priority>=1 |                                     |
                    +<----------------+                                     |
                                                                            v
                                                                    Check on watermark

$ sar -B 1
----------------------------------------------------------------------------------------
- MEMORY COMPACTION -

As the system runs, tasks allocate and free the memory and it becomes fragmented. memory
compaction addresses the fragmentation issues. This mechanism moves occupied pages from
the lower part of a memory to free pages in the upper part of the zone.

Before Compaction:

+---------------------+
|x| | |x|x| | | |x|x| |
+---------------------+

After Compaction:

+---------------------+
| | | | | | |x|x|x|x|x|
+---------------------+

try_to_compact_pages() for high-order allocation
        |
        +- compact each zone in the zonelist
                      |
                      +- compact_zone_order() with struct compact_control cc initialized
                                      |
                                      +- compact_zone()
                                               |
                                               +- initialize two lists:
                                                  a) cc->freepages
                                                  b) cc->migratepages
                                               +- compaction_suitable() check if should
                                                  do compaction
                                                           |
                                                           +- __compaction_suitable()
                                                           |    |
                                                          [0]   +- Check if watermarks
                                                                   for high-order
                                                                   allocation are met
                                                                            |
                                                                   zone_watermark_ok()
                                                                            |
                                                                   Possible outcomes
                                                                   a) COMPACT_CONTINUE
                                                                   b) COMPACT_SKIPPED
                                                                   c) COMPACT_SUCCESS
                                                          [0]
                                                           |
                                                           +- fragmentation_index()
                                                              | @mm/vmstat.c
                                                              +- __fragmentation_index()
                                                                           |
        +<----------------------------------------------------+- Possible outcomes
        |                                                        a) 0 => lack of memory
        |                                                        b) 1 => fragmentation
        |
        +- setup for where to start the scanners
        |
        +- loop to do compaction
                          |
                          +- isolate_migratepages()
                          |           |
                         [1]          +- Briefly search the free lists for a migration
                                         source that already has some free pages to
                                         reduce the number of pages that need migration
                                         before a pageblock is free.
                                                   |
                                                   v
                                         fast_find_migrateblock()

                                      |
                         [1]<---------+- a) ISOLATE_ABORT
                          |
                          +- putback_movable_pages() [cc->migratepages = 0]

                                         b) ISOLATE_NONE
                                         c) ISOLATE_SUCCESS
                          |
                          +- migrate_pages() @mm/migrate.c

wakeup_kcompactd()
        |
        +- check if kcompactd_node_suitable() - [END]
                                |yes
                                +- wake_up_interrupt(&pgdat->kcompactd_wait)
                                                                   |
                                                                   v
        +<---------------------------------------------------------+
        |
The background compaction daemon
        |
   kcompactd()
        |
        +- loop wait for event
        |           |
       [2]          +- kcompactd_do_work()
                                |
                                +- Find a suitable zone to do compaction
                                                       |
                                                       v
                                                       +-> compact_zone()
                                                       |
                                                       +<-------+
       [2]                                                      |
        |                                                       |
        +- should_proactive_compact_node()                      |
                         |yes                                   |
                         +- proactive_compact_node()            |
                                       |                        |
                                       +- compact all zones within a node till each
                                          zone's fragmentation score reaches within
                                          proactive compaction thresholds (as
                                          determined by the proactiveness tunable).

(Fragmentation Score)
    |
    |                _____ proactive compaction invokes
    |               /     \
    |----------------------------------- (threshold)
    |         ____/         \
    |   _____/               \__________ proactive compaction suspends
    |  /
    | /
    +------------------------------------> (time)

fragmentation_score_node()
            |
            +- compute per-node fragmentation score
                              |
                              +- fragmentation_score_zone_weighted()
                                                 |
                                                 +- fragmentation_score_zone()
                                                        |        |
                                                        +        +- extfrag_for_order()
                                                       /                [0, 100]
                                                      /
                                  zone->present_pages
                 sum of [ ------------------------------------ ] all zones
                          zone->zone_pgdat->node_present_pages

----------------------------------------------------------------------------------------
- PAGE FAULT -

@include/linux/sched.h

struct task_struct {
    struct thread_info thread_info;
    ...
    void *stack;
    ...
    struct mm_struct *mm;
    struct mm_struct *active_mm;
    ...
    pid_t pid;
    ...
    struct signal_struct *signal;
    struct signal_struct __rcu *sighand;
    ...
    struct thread_struct thread; - CPU-specific state of the task
};
                 |
                 | @arch/arm64/include/asm/processor.h
                 \
                   struct thread_struct {
                        struct cpu_context cpu_context; - CPU registers
                        ...
                        unsigned long fault_address;
                        unsigned long fault_code;
                        struct debug_info debug;
                        ...
                   };


@include/linux/mm.h

vm_fault is filled by the pagefault handler and passed to the vma's fault callback function.

struct vm_fault {
    const struct {
        struct vm_area_struct *vma;
        ...
        unsigned long address;
        unsigned long real_address;
    };
    ...
    struct page *cow_page;
    struct page *page;
    ...
};

A VM area is any part of the process virtual memory space that has a special rule for page-
fault handlers.

@include/linux/mm_types.h

struct vm_area_struct {
    ...
    const struct vm_operations_struct *vm_ops;
    ...                 |
                        +----> ...
                        |
};                      +----> vm_fault_t (*fault)(struct vm_fault *vmf);
                        |
                        +----> vm_fault_t (*huge_fault)(struct vm_fault *vmf,
                                               enum page_entry_size pe_size);


@arch/arm64/kernel/entry.S

Exceptions are conditions or system events that require some action by privileged software
(an exception handler) to ensure smooth functioning of the system.

       Program Flow +
                    |       +
                    |     / |
                    |    /  |
                    |   /   |
                    |  /    |
                    | /     v
   Exception occurs +       + ASM Exception Handler -> C Subroutine
                    | \     |
                    |  \    |
                    |   \   |
                    |    \  |
                    |     \ v spsr_el[n]
                    v       + elr_el[n] - Exception Link Register
                                                     |
                                                     v
                                          When taking an exception to EL1, holds the
                                          address to return to.

Causes to Exception:
1) Aborts - a) Failed instruction fetches (Instruction Aborts)
     |      b) Failed data accesses (Data Aborts)
     |                  |
     |                  +- Error response on a memory access (indicating perhaps that the
     |                     specified address does not correspond to real memory in the
     |                     system)
     |
     +- far_el[n] - Holds the faulting virtual address for all synchronous instruction
                    abort exceptions, data abort exceptions, PC alignment fault
                    exceptions and watchpoint exceptions that are taken to EL1.

The virtual address of each table base is set by the Vector Base Address Registers.

a) vbar_el[n]

The Exception Syndrome Registers contains information that allows the exception handler
to determine the reason for the exception. It's updated only for synchronous exceptions
and SError.

a) esr_el[n]

If an exception is taken, the PSTATE information is saved in the Saved Program Status
Registers (spsr_el3, spsr_el2, spsr_el1).

PSTATE as below:

|31                                                         0|
+-+-+-+-+---------+--+--+-----------------+-+-+-+-+-+-+------+
|N|Z|C|V|         |SS|IL|                 |D|A|I|F| |M|M[3:0]|
+-+-+-+-+---------+--+--+-----------------+-+-+-+-+-+-+------+
                                           | | | |
                                           | | | +- FIQ interrupt process state mask
                                           | | +--- IRQ interrupt process state mask
                                           | +----- SError interrupt process state mask
                                           +------- Debug exception mask


exception vectors
   |
vectors => Assembly Code
   |
   +- ...
   |
   +- kernel_ventry 0, t, 64, sync --->+
   |                                   |
   +- ...                              |
   |                                   |
   +- kernel_ventry 1, h, 64, sync     |
   |                                   |
   +- ...                              +- entry_handler 0, t, 64, sync
                                       |    (Early Exception Handler)
@arch/arm64/kernel/entry-common.c      |
                                       |
   +------------------------+          |
   |                        |          v
   | el0t_64_sync_handler() <----------+
   |    |                   |
   | el0_da() ------------------------>+- Data Abort
   |    |                   |          |
   | el0_ia() ------------------------>+- Instruction Abort
U  |                        |          |
---+------------------------+          |
K  |                        |          |
   | el1h_64_sync_handler() |          |
   |     |                  |          |
   | el1_abort() --------------------->+
   |                        |          |
   +------------------------+          |
                                       |
@arch/arm64/mm/fault.c                 |
                                       v
do_mem_abort() <-----------------------+
      |
      +- (struct fault_info) inf->fn() callback
                                   |
                                   /
                                  /
                                 /
                                /
                               /

static const struct fault_info fault_info[] = {
    ...
    { do_translation_fault, SIGSEGV, SEGV_MAPERR, "level 0 translation fault" },
    ...                                               [0, 3]
    { do_page_fault, SIGSEGV, SEGV_ACCERR, "level 1 access flag fault" },
    ...                                        [1, 3]
    { do_page_fault, SIGSEGV, SEGV_ACCERR, "level 1 permission fault" },
    ...                                        [1, 3]
};         |
           |                                        \
           v                                        |
do_translation_fault() @arch/arm64/mm/fault.c       |
       |                                            |
       +- do_page_fault()                           |
                |                                   |
                +- __do_page_fault()                |
                |          |                        |
                |         [0]                       |
                |                                   |
                +- esr_to_fault_info() ------------>+
                |
                +- set_thread_esr()

                          [0]
                           |
                           +- handle_mm_fault()
                                      |
                                      +- is_vm_hugetlb_page() ------->+
                                                  |                   |
                                           hugetlb_fault()            |
                                                                      v
                                                          +- __handle_mm_fault()
                                                          |
                                                          :
                                                          |
                                                          +- handle_pte_fault()
                               vmf->pte                       |       |
    +<--------------------------------------------------------+       |!vmf->pte
    |                                                                [1]
do_fault() @mm/memory.c
    |
    +- When vma->vm_ops->fault
                  |
       vmf->flags & FAULT_FLAG_WRITE --+
                  |no                  |
            do_read_fault()            |yes
                  |                    |
                 [a]        vma->vm_flags & VM_SHARED --+
                                       |no              |
                                 do_cow_fault()         |yes
                                       |                |
                                      [b]     do_shared_fault()
                                                        |
                                                       [c]

                                                                     [1]
                                                                      |
                                                               vma_is_anonymous()
                                                               |yes        |
                                                    do_anonymous_page()    |no
                                                                           |
                                                                    pte_present()
                                                                   /no     |
                                                         do_swap_page()    |yes
                                                                           |
                                                                   pte_protnone()
                                                                  /yes     |
                                                         do_numa_page()    |no
                                                                           |
                                                                           :
                                                                           |
                                                               update_mmu_cache()


Above all [a, b, c] follows the logic below:

__do_fault()
     |
     +- When no vmf->prealloc_pte
     |            |
     |            +- pte_alloc_one()
     |
     +- vma->vm_ops->fault() callback

----------------------------------------------------------------------------------------
- PAGE MIGRATION -

Migrate the pages specified in a list, to the free pages supplied as the target for the
page migration.

migrate_pages() @mm/migrate.c
      |
      +- 10 attempts or if no pages are movable any more
                              |
                              +- PageHuge() --------------> unmap_and_move_huge_page()
                                     |no
                                     +- unmap_and_move()
                                               |
                                               +- Obtain the lock on page, remove all
                                                  ptes and migrate the page to the
                                                  newly allocated page in newpage.
                                                                 |
                                                                 +- __unmap_and_move()


----------------------------------------------------------------------------------------
- SLAB -

SLOB - Simple List of Blocks
SLAB - Simple List of Allocated Blocks
SLUB - Simple List of Unqueued Blocks



@include/linux/slub_def.h

                                                    +- struct kmem_cache_cpu {
                                                    |       void **freelist;
                                                    |       ...
                                                    |       struct slab *slab;
                                                    |       struct slab *partial;
                                                    |       ...
                                                    |  };
struct kmem_cache {                                 |
    struct kmem_cache_cpu __percpu *cpu_slab; ------+
    ...
    struct kmem_cache_node *node[MAX_NUMNODES];
};               |
                 |
                 +- struct kmem_cache_node { @mm/slab.h
                        ...
                        struct list_head slabs_partial; -+
                        struct list_head slabs_full;     |-> CONFIG_SLAB
                        struct list_head slabs_free;    -+
                        ...
                        struct list_head partial;        ---> CONFIG_SLUB
                        ...
                    };



kzalloc() @include/linux/slab.h
   |
   +- kmalloc()
         |
         +- Size is constant - __kmalloc()
                  |                 |
                 [0]                +- __do_kmalloc_node()
                                          |
                                          +- size > KMALLOC_MAX_CACHE_SIZE ---+
                                              |no                             |
                                              +- kmalloc_slab()               |
                                              |                               |
                 [0]                          +- __kmem_cache_alloc_node()    |
                  |yes                                                        |
                  +- size > KMALLOC_MAX_CACHE_SIZE - kmalloc_large()          |
                       |no                                |                   v
                      [1]                                 +- __kmalloc_large_node()
                                                                |
                                                                +- alloc_pages_node()


                      [1]    kmalloc_caches[NR_KMALLOC_TYPES][KMALLOC_SHIFT_HIGH + 1]
                       |        /
                       +- kmalloc_trace()
                                |
                                +- __kmem_cache_alloc_node()
                                      |
                                      +- slab_alloc_node()
                                                |
                                                +-


----------------------------------------------------------------------------------------
- KERNEL SAMEPAGE MERGING (KSM) -

KSM is a memory-saving de-duplication feature, enabled by CONFIG_KSM=y. KSM maintains
reverse mapping information for KSM pages in the stable tree.

----------------------------------------------------------------------------------------
- MEMFD -

memfd_creat() - @syscall @mm/memfd.c
     |
     +-> create an anonymous file that can be shared in a shmem tmpfs or hugetlbfs.

memfd_fcntl()
     |
     +- 1) F_ADD_SEALS
                |
                +- memfd_add_seals()
                           |
                           +-> Sealing allows multiple parties to share a tmpfs or
                               hugetlbfs file but restrict access to a specific subset
                               of file operations.

        2) F_GET_SEALS
                |
                +- memfd_get_seals()

----------------------------------------------------------------------------------------
Reference

[a] ARM®Cortex®-A Series Version: 1.0 Programmer’s Guide for ARMv8-A

+--------------------------------------------------------------------------------------+
| Memory Management                                                                    |
+--------------------------------------------------------------------------------------+
