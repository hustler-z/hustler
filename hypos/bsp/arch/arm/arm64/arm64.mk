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
arma-y += hypos.o
arma-y += smp.o
arma-y += smc.o
arma-y += mmu.o
arma-y += zcr.o
# ----------------------------------------------------------
armc-y += traps.o
armc-y += vsysreg.o
armc-y += bitops.o
armc-y += sve.o
# ----------------------------------------------------------
lds-y  += hypos.lds
# ----------------------------------------------------------
