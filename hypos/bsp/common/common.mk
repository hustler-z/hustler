# ----------------------------------------------------------
# Hustler's Project
#
# File:  common.mk
# Date:  2024/06/04
# Usage:
# ----------------------------------------------------------

common-y += gicv3.o
common-y += timer.o
common-y += mmap.o
common-y += panic.o
common-y += globl.o

common-y += console.o
common-y += setup.o
common-y += command.o
common-y += print.o
common-y += hackmem.o
common-y += period.o
common-y += env.o
common-y += device.o
common-y += stdio.o
common-y += percpu.o
common-y += vmap.o
common-y += symtbl.o
common-y += softirq.o
common-y += wq.o
common-y += mutex.o
common-y += refcount.o
common-y += notifier.o
