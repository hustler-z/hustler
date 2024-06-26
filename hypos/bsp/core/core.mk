# ----------------------------------------------------------
# Hustler's Project
#
# Date:  2024/05/21
# Usage: coreos vmm core components
# ----------------------------------------------------------

core-y += vcpu.o
core-y += vmem.o
core-y += sched.o
core-y += virtio.o
core-y += vgic.o
core-y += vtimer.o
core-y += passthru.o
core-y += vm.o
