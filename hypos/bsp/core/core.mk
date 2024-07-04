# ----------------------------------------------------------
# Hustler's Project
#
# Date:  2024/05/21
# Usage: coreos vmm core components
# ----------------------------------------------------------

core-y += vcpu.o
core-y += sched.o
core-y += virtio.o
core-y += vgic.o
core-y += vtimer.o
core-y += hvm.o
core-y += core.o

core-y += gicv3.o
core-y += timer.o
core-y += mmap.o
core-y += panic.o

core-y += console.o
core-y += setup.o
core-y += command.o
core-y += print.o
core-y += hypmem.o
core-y += period.o
core-y += env.o
core-y += device.o
core-y += stdio.o
core-y += percpu.o
core-y += vmap.o
core-y += symtbl.o
core-y += softirq.o
core-y += wq.o
core-y += mutex.o
core-y += refcount.o
core-y += syms.o
core-y += numa.o
