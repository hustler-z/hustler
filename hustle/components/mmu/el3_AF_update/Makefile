# ARMv8 Startup Example Makefile
#
# Copyright (C) ARM Limited, 2016. All rights reserved.
#
# This makefile is intended for use with GNU make
# This example is intended to be built with ARM Compiler 6

EXECUTABLE=image.axf

#
# **********************************************************************
#
# Environmental variables to control build options
#
#   TARGET      defaults to AEM, but setting it to CORTEXA will
#               build extra code for initialising Cortex-A5[37]
#               processors
#
#   GICDALIAS	alias address for message-based SPIS; setting it
#		to 0 will select the non-aliased address
#
ASFLAGS=	-c -gdwarf-3 -march=armv8.1-a --target=aarch64-arm-none-eabi
CFLAGS= 	-c -gdwarf-3 -march=armv8.1-a --target=aarch64-arm-none-eabi

OBJFILES=	startup.o

CC=		armclang
AS=		armclang
LD=		armlink

# Select build rules based on Windows or Unix
ifdef WINDIR
DONE=@if exist $(1) if exist $(2) echo Build completed.
RM=if exist $(1) del /q $(1)
SHELL=$(WINDIR)\system32\cmd.exe
else
ifdef windir
DONE=@if exist $(1) if exist $(2) echo Build completed.
RM=if exist $(1) del /q $(1)
SHELL=$(windir)\system32\cmd.exe
else
DONE=@if [ -f $(1) ]; then if [ -f $(2) ]; then echo Build completed.; fi; fi
RM=rm -f $(1)
endif
endif

all: $(EXECUTABLE)
	$(call DONE,$(EXECUTABLE))

rebuild: clean all

startup.o: startup.s
	$(AS) ${ASFLAGS} startup.s

clean:
	$(call RM,*.o)
	$(call RM,*.d)
	$(call RM,$(EXECUTABLE))

$(EXECUTABLE): ${OBJFILES}
	$(LD) --ro-base=0x80000000 ${OBJFILES} -o $@ --entry=start64 --diag_suppress L6412

