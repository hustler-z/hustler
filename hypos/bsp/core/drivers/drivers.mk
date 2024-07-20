# ----------------------------------------------------------
# Hustler's Project
#
# File:  drivers.mk
# Date:  2024/06/04
# Usage:
# ----------------------------------------------------------

drivers-y += serial.o
drivers-y += ns16550.o
drivers-y += keyboard.o
drivers-y += input.o

ifeq ($(PLATFORM),rockchip)
drivers-y += rkserial.o
drivers-y += rktimer.o
endif

# ----------------------------------------------------------
