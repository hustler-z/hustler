/**
 * Hustler's Projects                                             2024/04/22 MON
 * -----------------------------------------------------------------------------
 * XVISOR - eXtensible Versatile hypervISOR
 * -----------------------------------------------------------------------------
 * Popek and Goldberg virtualization requirements
 *
 * Three properties of interest when analyzing the environment created by a VMM
 * (Virtual Machine Monitor)
 * (1) Equivalence
 * (2) Resource Control
 * (3) Effiency
 *
 * Virtualization requirements
 * Popek and Goldberg introduce 3 different groups of instructions:
 * (1) Privileged instructions
 *     - trap if the processor is in user mode and do not trap if it is in
 *       system mode
 * (2) Control sensitive instructions
 *     - Attempt to change the configuration of resources in the system
 * (3) Behavior sensitive instructions
 *     - behavior or result depends on the configuration of resources
 *
 * Theorem 1  For any conventional third-generation computer, a VMM may be
 * constructed if the set of sensitive instructions for that computer is a
 * subset of the set of privileged instructions.
 *
 * Theorem 2  A conventional third-generation computer is recursively
 * virtualizable if it is virtualizable and a VMM without any timing
 * dependencies can be constructed for it.
 *
 * -----------------------------------------------------------------------------
 * hypervisor => VMM
 *
 * Type-1 hypervisors, a.k.a., native, bare-metal, hypervisors
 *                     (a) monolithic   - device drivers in hypervisor
 *                                        Complete monolithic  - xvisor
 *                                        Partially monolithic - KVM
 *                     (b) microkernel  - device drivers in parent guest
 *                                        parent guest => privileged VM
 *                                        child guests => non-privileged VMs
 *
 *                                        +---------------+ +-------+
 *                                        | Privileged VM | |  VM1  | ...
 *                                        +---------------+ +-------+
 *                                        +-------------------------------+
 *                                        |          Hypervisor           |
 *                                        +-------------------------------+
 *
 * Type-2 hypervisors, a.k.a., hosted hypervisors
 *
 * Components of VMM:
 *
 * (1) Dispatcher
 * (2) Allocator
 * (3) Interpreter
 *
 * Implementation of Virtualization
 *
 * (1) Full Virtualization
 * (2) Para Virtualization
 * (3) Hardware Assisted Virtualization
 *
 * -----------------------------------------------------------------------------
 * Problematic Instructions
 *
 * (1) undefine when execute on user mode
 * (2) no effect when execute on user mode
 *     MSR, MRS
 * (3) unpredictable behavior when execute on user mode
 * (4) ARM sensitive instructions
 *     CPSR
 *
 * Solutions:
 * (a) Dynamic binary translation
 * (b) Hypercall
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * -----------------------------------------------------------------------------
 **/
