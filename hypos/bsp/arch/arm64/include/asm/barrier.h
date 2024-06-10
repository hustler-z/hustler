/**
 * Hustler's Project
 *
 * File:  Barrier.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_BARRIER_H
#define _ARCH_BARRIER_H
// ------------------------------------------------------------------------

/* Instruction Synchronization Barrier flushes the pipeline in the
 * PE and is a context synchronization event.
 */
#define isb()           asm volatile("isb" : : : "memory")

/* Data Synchronization Barrier is a memory barrier that ensures the
 * completion of memory accesses.
 */
#define dsb(scope)      asm volatile("dsb " #scope : : : "memory")

/* Data Memory Barrier is a memory barrier that ensures the ordering
 * of observations of memory accesses.
 */
#define dmb(scope)      asm volatile("dmb " #scope : : : "memory")

/* Send Event is a hint instruction. It causes an event to be
 * signaled to all PEs in the multiprocessor system.
 */
#define sev()           asm volatile("sev" : : : "memory")

/* Wait For Event is a hint instruction that indicates that the PE
 * can enter a low-power state and remain there until a wakeup event
 * occurs. Wakeup events include the event signaled as a result of
 * executing the SEV instruction on any PE in the multiprocessor
 * system.
 */
#define wfe()           asm volatile("wfe" : : : "memory")

/* Wait For Interrupt is a hint instruction that indicates that the
 * PE can enter a low-power state and remain there until a wakeup
 * event occurs.
 */
#define wfi()           asm volatile("wfi" : : : "memory")

#define __nops(n)	    ".rept	" #n "\nnop\n.endr\n"
#define nops(n)		    asm volatile(__nops(n))

#define psb_csync()	    asm volatile("hint #17" : : : "memory")
#define csdb()		    asm volatile("hint #20" : : : "memory")

#define mb()            dsb(sy)
#define rmb()           dsb(ld)
#define wmb()           dsb(st)

#define smp_mb()        dmb(ish)
#define smp_rmb()       dmb(ishld)
#define smp_wmb()       dmb(ishst)

#define dma_mb()	    dmb(osh)
#define dma_rmb()	    dmb(oshld)
#define dma_wmb()	    dmb(oshst)

#define local_fiq_disable()   asm volatile ("msr DAIFSet, #1\n" ::: "memory")
#define local_irq_disable()   asm volatile ("msr DAIFSet, #2\n" ::: "memory")
#define local_abort_disable() asm volatile ("msr DAIFSet, #4\n" ::: "memory")

#define local_fiq_enable()    asm volatile ("msr DAIFClr, #1\n" ::: "memory")
#define local_irq_enable()    asm volatile ("msr DAIFClr, #2\n" ::: "memory")
#define local_abort_enable()  asm volatile ("msr DAIFCLr, #4\n" ::: "memory")

#define local_irq_restore(x) ({ \
    asm volatile (              \
        "msr DAIF, %0"          \
        :                       \
        : "r" (x)               \
        : "memory");            \
})

#define local_save_flags(x) ({  \
    asm volatile (              \
        "mrs %0, DAIF"          \
        : "=r" (x)              \
        :                       \
        : "memory");            \
})

#define local_irq_save(x) ({    \
    local_save_flags(x);        \
    local_irq_disable();        \
})
// ------------------------------------------------------------------------
#endif /* _ARCH_BARRIER_H */
