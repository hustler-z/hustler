/**
 * Hustler's Project
 *
 * File:  spinlock.S
 * Date:  2024/06/04
 * Usage: Spinlock Implementation
 */

#include <asm/lock.h>
#include <asm/define.h>
#include <asm/sysregs.h>

// --------------------------------------------------------------

FUNC(__cpu_spin_lock)
    mov	w2, #SPINLOCK_LOCK
    sevl
l1:	wfe
l2:	ldaxr w1, [x0]
    cbnz  w1, l1
    stxr  w1, w2, [x0]
    cbnz  w1, l2
    ret
END(__cpu_spin_lock)

FUNC(__cpu_spin_trylock)
    mov   x1, x0
    mov   w2, #SPINLOCK_LOCK
.loop:
    ldaxr w0, [x1]
    cbnz  w0, .cpu_spin_trylock_out
    stxr  w0, w2, [x1]
    cbnz  w0, .loop
.cpu_spin_trylock_out:
    ret
END(__cpu_spin_trylock)

FUNC(__cpu_spin_unlock)
    stlr  wzr, [x0]
    ret
END(__cpu_spin_unlock)


FUNC(__get_percpu_pos)
    mrs  x0, MPIDR_EL1
    b    get_percpu_pos_mpidr
END(__get_percpu_pos)


FUNC_LOCAL(get_percpu_pos_mpidr)
    /*
     * Shift MPIDR value if it's not already shifted.
     * Using logical shift ensures AFF0 to be filled with zeroes.
     * This part is necessary even if CFG_CORE_THREAD_SHIFT is 0 because
     * MT bit can be set on single threaded systems where all the AFF0
     * values are zeroes.
     */
    tst   x0, #MPIDR_MT_MASK
    lsl   x3, x0, #MPIDR_LEVEL_BITS
    csel  x3, x3, x0, eq

    /*
     * At this point the MPIDR layout is always shifted so it looks
     * as follows AFF2 -> cluster, AFF1 -> core, AFF0 -> thread
     */
#if 1
    /* Calculate CorePos = (ClusterId * (cores/cluster)) + CoreId */
    ubfx  x0, x3, #MPIDR_AFF1_SHIFT, #MPIDR_LEVEL_BITS
    ubfx  x1, x3, #MPIDR_AFF2_SHIFT, #MPIDR_LEVEL_BITS
    add   x0, x0, x1, lsl #2
#else
    /*
     * Calculate CorePos =
     * ((ClusterId * (cores/cluster)) + CoreId) * (threads/core) + ThreadId
     */
    ubfx  x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_LEVEL_BITS
    ubfx  x1, x3, #MPIDR_AFF1_SHIFT, #MPIDR_LEVEL_BITS
    ubfx  x2, x3, #MPIDR_AFF2_SHIFT, #MPIDR_LEVEL_BITS
    add   x1, x1, x2, lsl #2    /* 2 ^ 2 = 4 cores per cluster */
    add   x0, x0, x1, lsl #0
#endif

    ret
END(get_percpu_pos_mpidr)

// --------------------------------------------------------------
