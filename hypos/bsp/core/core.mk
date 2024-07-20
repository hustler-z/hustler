# ----------------------------------------------------------
# Hustler's Project
#
# Date:  2024/05/21
# Usage: coreos vmm core components
# ----------------------------------------------------------

core-y += vcpu.o
core-y += sched.o
core-y += virtio.o
core-y += hvm.o

core-y += mmap.o
core-y += panic.o
core-y += cpu.o
core-y += console.o
core-y += setup.o
core-y += command.o
core-y += print.o
core-y += hypmem.o
core-y += period.o
core-y += env.o
core-y += device.o
core-y += stdio.o
core-y += vmap.o
core-y += symtbl.o
core-y += softirq.o
core-y += wait.o
core-y += rwlock.o
core-y += refcount.o
core-y += syms.o
core-y += numa.o
core-y += board.o
core-y += preempt.o
core-y += rcu.o
core-y += tasklet.o
core-y += calculate.o

core-y += bootcore.o

# ----------------------------------------------------------
