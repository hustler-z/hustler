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

      @mm->pgd
          |
pgd_offset()
     |
     +-> p4d_offset()
              |
              +-> pud_offset()
                       |
                       +-> pmd_offset()
                                |
                                +-> pte_offset_map_lock()

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

----------------------------------------------------------------------------------------
- MTE -

ARMv8.5 based processors introduce the Memory Tagging Extension (MTE) feature. MTE is
built on top of the ARMv8.0 virtual address tagging TBI (Top Byte Ignore) feature and
allows software to access a 4-bit allocation tag for each 16-byte granule in the
physical address space.

----------------------------------------------------------------------------------------
- KERNEL PAGE TABLE DUMP -

ptdump is a debugfs interface that provides a detailed dump of the kernel page tables.
CONFIG_GENERIC_PTDUMP=y
CONFIG_PTDUMP_CORE=y
CONFIG_PTDUMP_DEBUGFS=y

$ mount -t debugfs nodev /sys/kernel/debug
$ cat /sys/kernel/debug/kernel_page_tables
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

Huga Pages - Contiguous areas of physical memory. typically associated with Page table
             level.
             Pros: Fewer translation entries, Less time servicing TLB miss.
             Cons: Less granular page size, Fewer TLB entries.

The ARM64 port supports two flavors of hugepages:
a) Block mappings at the pud/pmd level;
b) Using the Contiguous bit;

1) HugeTLB filesystem (hugetlbfs), a pseudo filesystem that uses RAM as its backing store.
   Pools of hugetlb pages are created/preallocated.

-----------------------------------------------------
$ cat /proc/meminfo | egrep Huge
-----------------------------------------------------
AnonHugePages:         0 kB
ShmemHugePages:        0 kB
FileHugePages:         0 kB
HugePages_Total:       0
HugePages_Free:        0
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
Hugetlb:               0 kB
-----------------------------------------------------

[+] mm/hugetlb.c

    hugetlb_init()
       |
       :
       +- hugetlb_add_hstate() => struct hstate
       :
       +-

khugepaged()
     |
     +-

2) Transparent HugePages (THP)
             |
             +-> Primaryly used for anonymous memory.

                 [+] mm/madvise.c
                 * Related madvise behavior:
                 1) MADV_HUGEPAGE   - enable THP for give range
                 2) MADV_NOHUGEPAGE - disable THP for give range
                 3) MADV_COLLAPSE   - synchronously coalesce pages into new THP

                 madvise() @syscall
                    |
                    +- do_madvise()
                        |
                        :
                        +- madvise_walk_vmas() walk the vmas in range[start, end)
                                        |      and call the visit callback.
                                        :
                                        +- visit()
                                              |
                       +<--------- madvise_vma_behavior()
                       |                            |
                       :  [+] mm/khugepaged.c       :  [+] mm/khugepaged.c
                       +- madvise_collapse()        +- hugepage_madvise()
                                                         |
                                                         +- 1) MADV_HUGEPAGE
                                                               khugepaged_enter_vma()
                                                                          |
                                              +<--------------------------+
                                              |
                                              :
                                              +- __khugepaged_enter()
                                                           |
                                                           :
                                                           +- ..
HPAGE_PUD_SIZE => 1G
HPAGE_PMD_SIZE => 2M

madvise_collapse()
        |
        :                                               no
        +- if IS_ENABLED(CONFIG_SHMEM) && vma->vm_file ----> hpage_collapse_scan_pmd()
                  |yes
                  +- hpage_collapse_scan_file()

By default, transparent hugepage support is disabled in order to avoid risking an
increased memory footprint for applications that are not guaranteed to benefit from it.
When transparent hugepage support is enabled, it is for all mappings, and khugepaged
scans all mappings. Defrag is invoked by khugepaged hugepage allocations and by page
faults for all hugepage allocations.

----------------------------------------------------------------------------------------
- Heterogeneous Memory Management (HMM) -





----------------------------------------------------------------------------------------
- High Memory -

High memory (highmem) is used when the size of physical memory approaches or exceeds the
maximum size of virtual memory. The kernel needs to start using temporary mappings of
the pieces of physical memory that it wants to access.

Temporary Virtual Mappings

kmap_local_page() => Map a page for temporary usage

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

1) PageCompound() => true if compound page
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
                     +- ret = security_mmap_file()
                               |ret=0
                               +- do_mmap()
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
                flags & MAP_TYPE     |
                +----------------------------------------------+
                | MAP_SHARED  MAP_SHARED_VALIDATE  MAP_PRIVATE | with file!=NULL
                | MAP_SHARED                       MAP_PRIVATE | with file =NULL
                +----------------------------------------------+
                                     :
                                     +- mmap_region()
                                              |
                                              +- may_expand_vm() Check against
                                                 address space limit. Return true
                                                 if the calling process may expand
                                                 its vm space by the passed number
                                                 of pages.

when vma_expand() expand an existing VMA failed:

                                              :
                +----------------------------------------------+
                | a) file mappings         call_mmap()         |
                | b) anonymous mapping     vma_set_anonymous() |
                | c) shared mapping        shmem_zero_setup()  |
                +----------------------------------------------+
                                              :
                                              +- if vma->vm_file
                                                    |!=NULL
                                                    +- vma_interval_tree_insert()


[+] mm/interval_tree.c

struct vm_area_sruct {
    ...
    union {
        struct {
            struct rb_node rb; -------->+
            ...                         |
        } shared;                       |
        ...                             |                     ●
    };                                  |                    / \
    ...                                 |  RB-Tree          ○   ○
};                                      | Insertion        / \   \
                                        |                 ●   ●
struct address_space {                  |
    ...                                 |
    ...                                 v
    struct rb_root_cached i_mmap; <-----+
    ...
};


New (or expanded) vma always get soft dirty status. Otherwise user-space soft-dirty
page tracker won't be able to distinguish situation when vma area unmapped, then new
mapped in-place (which must be aimed as a completely new data area).

vma->vm_flags |= VM_SOFTDIRTY;

call_mmap()
    |
    +- file->f_op->mmap(file, vma)

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

__alloc_pages_bulk() - allocate a number of order-0 pages to a list or array.

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
            [Virtual/Linear Address]
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

----------------------------------------------------------------------------------------

VMALLOC_SPACE

+----------------+ - VMALLOC_START
|                |
|                |
:                :
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

Stage 1 Translation               Stage 2 Translation (Control by Hypervisor)
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

remap memory (physical memory) to userspace (user vma)
          |
          v
   vm_iomap_memory()
          |
          :
          +- io_remap_pfn_range()
                      |
                      +- remap_pfn_range()
                                 |
                                 +- track_pfn_remap()
                                 |
                                 +- remap_pfn_range_notrack()


----------------------------------------------------------------------------------------

- REVERSE MAPPING (RMAP) -

 physical pages   PTEs
      |            |
 (struct page) --->+


----------------------------------------------------------------------------------------

  malloc()
   |                                                                          User Space
----------------------------------------------------------------------------------------
   |                                                                        Kernel Space
  brk() - syscall @mm/mmap.c
   |
   +- if brk <= mm->brk
             |
             +- do_brk_munmap() => Unmap a partial vma.
                      |
                      :
                      +- do_mas_align_munmap()

   |
   +- check_brk_limits()
   |
   +- do_brk_flags() => Extend the brk VMA from addr to addr + len. If the VMA is
           |            NULL or the flags do not match then create a new anonymous
           |            VMA.
           |
           +- (1) Expand the existing vma if possible
                        |
                  vma->anon_vma

                +-------------------------------------------------------+-----------+
                | A file's MAP_PRIVATE vma can be in both i_mmap tree and anon_vma  |
                | list, after a COW of one of the file pages.  A MAP_SHARED vma     |
                | can only be in the i_mmap tree.  An anonymous MAP_PRIVATE, stack  |
                | or brk vma (with NULL file) can only be in an anon_vma list.      |
                +-------------------------------------------------------------------+

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
                         +- vma_set_anonymous() -> set vma->vm_ops = NULL.

   |
   +- userfaultfd_unmap_complete()
   |
   +- if populate => newbrk > oldbrk && (mm->def_flags & VM_LOCKED) != 0
            |true
            +- mm_populate()
                     |
                     +- __mm_populate() => populate and/or mlock pages within
                                           a range of address space.
                                           [+] mm/gup.c
                                :
                                +- populate_vma_page_range()
                                             :
                                             +- __get_user_pages()

__mm_populate() is used to implement mlock() and the MAP_POPULATE / MAP_LOCKED mmap
flags. VMAs must be already marked with the desired vm_flags, and mmap_lock must not
be held.

----------------------------------------------------------------------------------------
- get_user_pages*() (gup) -

__get_user_pages()
	|
	v
	*----------------------------------------------------------*
	| __get_user_pages walks a process's page tables and takes |
	| a reference to each struct page that each user address   |
	| corresponds to at a given instant. That is, it takes the |
	| page that would be accessed if a user thread accesses    |
	| the given user virtual address at that instant.          |
	*----------------------------------------------------------*

----------------------------------------------------------------------------------------
- IOREMAP -

MMIO (Memory Mapped IO) => The data type for an MMIO address is an __iomem qualified
                           pointer.
Generic accessors

readq(),  readl(),  readw(),  readb()
writeq(), writel(), writew(), writeb()

Example Usage:

writel(0x11001100, ioremap(0xfdc60054, 4))
        |                       |
        +- value to write       +- (phys_addr_t) device physical address

ioremap(phys_addr_t addr, size_t size) => MMIO address
   |
   |CONFIG_GENERIC_IOREMAP
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

The kernel manages device resources (like registers) as physical addresses. These are
the addresses in /proc/iomem. The physical address is not directly useful to a driver;
it must use ioremap() to map the space and produce a virtual address.

I/O devices use a third kind of address: a "bus address". If a device has registers at
an MMIO address, or if it performs DMA to read or write system memory, the addresses
used by the device are bus addresses.

             CPU                  CPU                  Bus
           Virtual              Physical             Address
           Address              Address               Space
            Space                Space

          +-------+             +------+             +------+
          |       |             |MMIO  |   Offset    |      |
          |       |  Virtual    |Space |   applied   |      |
        C +-------+ --------> B +------+ ----------> +------+ A
          |       |  mapping    |      |   by host   |      |
+-----+   |       |             |      |   bridge    |      |   +--------+
|     |   |       |             +------+             |      |   |        |
| CPU |   |       |             | RAM  |             |      |   | Device |
|     |   |       |             |      |             |      |   |        |
+-----+   +-------+             +------+             +------+   +--------+
          |       |  Virtual    |Buffer|   Mapping   |      |
        X +-------+ --------> Y +------+ <---------- +------+ Z
          |       |  mapping    | RAM  |   by IOMMU           |
          |       |             |      |                      +-> DMA address
          |       |             |      |
          +-------+             +------+

The driver can give a virtual address X to an interface like dma_map_single(), which
sets up any required IOMMU mapping and returns the DMA address Z. The driver then tells
the device to do DMA to Z, and the IOMMU maps it to the buffer at address Y in system
RAM.

Types of DMA Mappings

Consistent DMA mappings which are usually mapped at driver initialization, unmapped at
the end and for which the hardware should guarantee that the device and the CPU can
access the data in parallel and will see updates made by each other without any explicit
software flushing.
Think of "consistent" as "synchronous" or "coherent"

dma_alloc_coherent()
dma_free_coherent()

Streaming DMA mappings which are usually mapped for one DMA transfer, unmapped right
after it (unless you use dma_sync_* below) and for which hardware can optimize for
sequential accesses.
Think of "streaming" as "asynchronous" or "outside the coherency domain"

dma_map_single()
dma_unmap_single()

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
             [0]      +- map_kernel()
                      |
                      +- map_mem()

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
                                          |                |
                                         [4]               v
                                             +--------------------------------------+
                                             | Go through the free lists for the    |
                                             | given migratetype and remove the     |
                                             | smallest available page from the     |
                                             | freelist.                            |
                                             | a) get_page_from_free_area()         |
                                             | b) del_page_from_free_list()         |
                                             | c) if current order doesn't meet, go |
                                             | to higher order, if available, split |
                                             | them into half, put a half back to   |
                                             | lower order of free area, another    |
                                             | half as requested.                   |
                                             +--------------------------------------+
                                                           |
                                                         expand()
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
                        +----------------------------------------------------------------+
                        | Try finding a free buddy page on the fallback list and put it  |
                        | on the free list of requested migratetype.                     |
                        | a) find_suitable_fallback() to find largest/smallest available |
                        |    free page in other list.                                    |
                        | Note: fallback lists can be (depends on current migratetype,   |
                        | other two are fallbacks) two of below:                         |
                        |       1) MIGRATE_UNMOVABLE                                     |
                        |       2) MIGRATE_MOVABLE                                       |
                        |       3) MIGRATE_RECLAIMABLE                                   |
                        | b) get_page_from_free_area()                                   |
                        | c) steal_suitable_fallback()                                   |
                        +----------------------------------------------------------------+
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
                                             |
                                +---------------------------------------------------+
                                | When above fails, try to spill all the per-cpu    |
                                | pages from all CPUs back into the buddy allocator |
                                +---------------------------------------------------+
                                             |
                                             +- __drain_all_pages()
                                                    |
                                          +-----------------------------------------+
                                          | Optimized to only execute on CPUs where |
                                          | pcplists are not empty.                 |
                                          +-----------------------------------------+
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
                                                  |
                                  +-------------------------------------------------+
                                  | Update defer tracking counters after successful |
                                  | compaction of given order.                      |
                                  +-------------------------------------------------+
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

try_to_free_pages() with (struct scan_control) sc initialized
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
                                                        |no                        |
                                                   vmpressure() ------------------>+
                                                        |
                                Account memory pressure through scanned/reclaimed ratio

[+] include/linux/mmzone.h

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
                             +--------+
                                      :
                                      v
        +---------------------------------------------------------------------------+
        | For kswapd, function below will reclaim pages across a node from zones    |
        | eligible for use by the caller until at least one zone is balanced.       |
        |                                                                           |
        | kswapd scans the zones in the highmem->normal->dma direction.  It skips   |
        | zones which have free_pages > high_wmark_pages(zone), but once a zone is  |
        | found to have free_pages <= high_wmark_pages(zone), any page in that zone |
        | or lower is eligible for reclaim until at least one usable zone is        |
        | balanced.                                                                 |
        +---------------------------------------------------------------------------+
                                      |
                                      v
                                balance_pgdat() with struct scan_control sc initialized
                                      |
                    +---------------->+
                    |                 |
                    :                 :
                    |                 +- pgdat_balanced() ------------------+
                    :                 :                                     |
                    |                 +- mem_cgroup_soft_limit_reclaim()    |
                    :                 :                                     |
                    |                 +- kswapd_shrink_node()               |
                    |  sc.priority>=1 |           |                         |
                    +<----------------+           |                         |
                                                  |                         v
                                                  :                 Check on watermark
                                                  |
                    +<----------------------------+
                    |
                    +- shrink_node()

do_swap_page()

----------------------------------------------------------------------------------------
sar - System Activity Report

$ sar -B 1
Above command used to report paging statistics, as below:

pgpgin/s  pgpgout/s fault/s  majflt/s  pgfree/s pgscank/s pgscand/s pgsteal/s  %vmeff
0.00      0.00      3.00      0.00     37.00      0.00      0.00      0.00      0.00
0.00      0.00      1.00      0.00      6.00      0.00      0.00      0.00      0.00

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
        +<------------------------------------------------ Possible outcomes
        |                                                  a) 0 => lack of memory
        |                                                  b) 1 => fragmentation
        |
        +- setup for where to start the scanners
        |
        +- loop to do compaction
                          |
                          +- isolate_migratepages()
                          |     |
                         [1]    +-->+------------------------------------------------+
                                    | Briefly search the free lists for a migration  |
                                    | source that already has some free pages to     |
                                    | reduce the number of pages that need migration |
                                    | before a pageblock is free.                    |
                                    +------------------------------------------------+
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
                                        |                       |
                                +-------------------------------------------+
                                | compact all zones within a node till each |
                                | zone's fragmentation score reaches within |
                                | proactive compaction thresholds (as       |
                                | determined by the proactiveness tunable). |
                                +-------------------------------------------+

----------------------------------------------------------------------------------------
- FRAGMENTATION SCORE -

(Fragmentation Score)
        |
        |                _____ proactive compaction invokes
        |               /     \
        |----------------------------------- (threshold)
        |         ____/         \
        |   _____/               \_________ proactive compaction suspends
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
                                                               |
                                                               +- extfrag_for_order()
                                                                      [0, 100]

@COMPACTION_HPAGE_ORDER => 9

               data_race(zone->free_area[order].nr_free)
                                  |
(free pages of zone) = sum of [ blocks << order ] of all orders

(suitable free blocks) = sum of [ blocks << (order - COMPACTION_HPAGE_ORDER) ] of all
                         order >= COMPACTION_HPAGE_ORDER

                             (free pages of zone) - ((suitable free blocks) << 9)
(fragmentation score zone) = ---------------------------------------------------- * 100
                                            (free pages of zone)

(fragmentation score node)

           zone->present_pages * (fragmentation score zone)
sum of [ ---------------------------------------------------- ] of all zones
               zone->zone_pgdat->node_present_pages + 1

----------------------------------------------------------------------------------------
- PAGE FAULT -

@include/linux/sched.h

struct task_struct {
    struct thread_info thread_info;
    ...
    void *stack;
    ...
    struct mm_struct *mm;

/**
 * tsk->mm => real address space (which cares about user-level page tales, whereas
 * anonymous address space does not.)
 *
 * The rule is that for a process with a real address space (ie tsk->mm is non-NULL)
 * the active_mm obviously always has to be the same as the real one.
 *
 * [ kthreads - anonymous processes ]
 * For a anonymous process, tsk->mm == NULL, and tsk->active_mm is the "borrowed" mm
 * while the anonymous process is running. When the anonymous process gets scheduled
 * away, the borrowed address space is returned and cleared.
 *
 * To support all that, the "struct mm_struct" now has two counters: a "mm_users"
 * counter that is how many "real address space users" there are, and a "mm_count"
 * counter that is the number of "lazy" users (ie anonymous users) plus one if there
 * are any real users.
 */

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
                                        +---------------------------------------+
                                        | Huge Page (PUD / PMD) related process |
                                        +---------------------------------------+
                                                          |not huge page
                                                          :
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

do_anonymous_page()
        :
        +- page_add_new_anon_rmap() => add mapping to a new anonymous page
                  :                    no
                  +- compound page ? -----> set page->_mapcount to 0 (atomic_set)
                            |yes
                            +-> set page[1].compound_mapcount to 0 (atomic_set)
                                set page[1].compound_pincount to 0 (atomic_set)

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
                                                                    [src -> dst folios]

The main intent of page migration is to reduce the latency of memory accesses by moving
pages near to the processor where the process accessing that memory is running.

$ cat /proc/<pid>/numa_maps
-------------------------------------------------------------------------------------
ffff867f4000 default file=* anon=1 dirty=1 mapmax=2 active=0 N0=1 kernelpagesize_kB=4

Allows an easy review of where the pages of a process are located, as shown above.

----------------------------------------------------------------------------------------
- SLAB -

SLAB - Simple List of Allocated Blocks

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
                        struct list_head partial;       ---> CONFIG_SLUB
                        ...
                    };


kmem_cache_create()
        |
        +- kmem_cache_create_usercopy()
           Create a cache with a region suitable for copying to userspace
                       |
                       :
                       +- create_cache()
                                |
                                :
                                +- kmem_cache_zalloc()
                                           |
                                           +- kmem_cache_alloc()
                                               [slab/slub/slob]
                                :
                                +- __kmem_cache_create()
                                     [slab/slub/slob]

slab cache can be found by the *name* created in kmem_cache_create().

$ cat /proc/slabinfo

----------------------------------------------------------------------------------------
- CMA -



----------------------------------------------------------------------------------------
- KERNEL SAMEPAGE MERGING (KSM) -

KSM is a memory-saving de-duplication feature, enabled by CONFIG_KSM=y. KSM maintains
reverse mapping information for KSM pages in the stable tree.

----------------------------------------------------------------------------------------
- MEMFD -

memfd_creat() - @syscall mm/memfd.c
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
- MPROTECT -

mprotect() changes the access protections for the calling process's memory pages
containing any part of the address range in the interval [addr, addr+len-1].  addr must
be aligned to a page boundary.

If the calling process tries to access memory in a manner that violates the protections,
then the kernel generates a SIGSEGV signal for the process.

mprotect() - @syscall mm/mprotect.c
   |
   +- do_mprotect_pkey()
              |
              :
              +- if vma->vm_ops && vma->vm_ops->mprotect
                                |NULL
                                +- mprotect_fixup()
                                        |
                                        :
                                        +- change_protection()
                                                |
                                                +- if is_vm_hugetlb_page()
                                                        |yes
                                                        +- hugetlb_change_protection()
                                                        :
                                                        |no
                                                        +- change_protection_range()

----------------------------------------------------------------------------------------
Under /proc/sys/vm directory:

- admin_reserve_kbytes
- compact_memory
- compact_unevictable_allowed
- compaction_proactiveness
- dirty_background_bytes
- dirty_background_ratio
- dirty_bytes
- dirty_expire_centisecs
- dirty_ratio
- dirty_writeback_centisecs
- dirtytime_expire_seconds
- drop_caches
- extfrag_threshold
- hugetlb_shm_group
- laptop_mode
- legacy_va_layout
- lowmem_reserve_ratio
- max_map_count
- memory_failure_early_kill
- memory_failure_recovery
- min_free_kbytes
- min_slab_ratio
- min_unmapped_ratio
- mmap_min_addr
- mmap_rnd_bits
- mmap_rnd_compat_bits
- nr_hugepages
- nr_hugepages_mempolicy
- nr_overcommit_hugepages
- numa_stat
- numa_zonelist_order
- oom_dump_tasks
- oom_kill_allocating_task
- overcommit_kbytes
- overcommit_memory
- overcommit_ratio
- page-cluster
- page_lock_unfairness
- panic_on_oom
- percpu_pagelist_high_fraction
- stat_interval
- stat_refresh
- swappiness
- user_reserve_kbytes
- vfs_cache_pressure
- watermark_boost_factor
- watermark_scale_factor
- zone_reclaim_mode

Above can be used to tune the operation of the virtual memory (VM) subsystem of the
Linux kernel and the writeout of dirty data to disk.

----------------------------------------------------------------------------------------
Reference

[a] ARM®Cortex®-A Series Version: 1.0 Programmer’s Guide for ARMv8-A

+--------------------------------------------------------------------------------------+
| Memory Management                                                                    |
+--------------------------------------------------------------------------------------+
