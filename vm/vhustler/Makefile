# ----------------------------------------------------------
# Modified the source tree:
#
# vhustler -
#
#
# Date: 2024/04/06 -
# ----------------------------------------------------------

EXECUTABLE := vhustler

WORK_DIR := $(shell pwd)

BUILD_DIR := $(WORK_DIR)/out

# Compiler settings
ARCH := $(shell uname -m)

ifeq ($(ARCH),x86_64)
CROSS_COMPILE :=
endif

ifeq ($(ARCH),aarch64)
CROSS_COMPILE :=
endif

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
AS := $(CROSS_COMPILE)as
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

CFLAGS 	:= -g -Wall
ASFLAGS := -g -Wall -D__ASSEMBLY__
LDFLAGS :=

# ---------------------------------------------------------
obj-y += arch/aarch64/
obj-y += hypervisor/
obj-y += peripherals/
obj-y += libc/

$(BUILD_DIR)/$(EXECUTABLE).elf:
