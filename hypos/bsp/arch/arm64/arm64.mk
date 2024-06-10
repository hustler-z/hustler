# ----------------------------------------------------------
# Hustler's Project
#
# Date:  2024/05/20
# Usage:
# ----------------------------------------------------------

arma-y += cache.o
arma-y += debug.o
arma-y += entry.o
arma-y += start.o
arma-y += hypervisor.o
arma-y += smp.o
arma-y += mmu.o
arma-y += timer.o

armc-y += spinlock.o
armc-y += boot.o
armc-y += gicv3.o
armc-y += exception.o
armc-y += ttbl.o
armc-y += mutex.o
