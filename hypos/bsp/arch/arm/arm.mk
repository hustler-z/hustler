# ----------------------------------------------------------
# Hustler's Project
#
# Date:  2024/06/19
# Usage:
# ----------------------------------------------------------

arm-y += spinlock.o
arm-y += atomic.o
arm-y += traps.o
arm-y += smp.o
arm-y += membank.o
arm-y += globl.o
arm-y += at.o
arm-y += io.o
arm-y += gic.o
arm-y += vgic.o
arm-y += time.o
arm-y += timer.o
arm-y += vtimer.o
arm-y += irq.o
arm-y += hypos.o
arm-y += percpu.o
arm-y += ttbl.o
arm-y += vttbl.o
arm-y += map.o
arm-y += boot.o
arm-y += early.o
arm-y += tlbflush.o
arm-y += psci.o
arm-y += cpu.o
