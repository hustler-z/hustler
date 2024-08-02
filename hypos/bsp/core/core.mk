# ----------------------------------------------------------
# Hustler's Project
#
# Date:  2024/05/21
# Usage: coreos vmm core components
# ----------------------------------------------------------

core-y += vcpu.o
core-y += sched.o
core-y += virtio.o
core-y += guest.o

core-y += panic.o
core-y += cpu.o
core-y += setup.o
core-y += vsnpr.o
core-y += memz.o
core-y += device.o
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
