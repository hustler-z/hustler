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
arma-y += lock.o
# ----------------------------------------------------------
armc-y += boot.o
armc-y += gicv3.o
armc-y += traps.o
armc-y += ttbl.o
armc-y += mutex.o
armc-y += early.o
armc-y += map.o
armc-y += tlbflush.o
armc-y += bitops.o
# ----------------------------------------------------------
lds-y  += hypos.lds
# ----------------------------------------------------------
