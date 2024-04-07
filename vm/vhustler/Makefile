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

# ----------------------------------------------------------

ARCH_PATH := arch/aarch64
HYPR_PATH := hypervisor
PERI_PATH := peripherals
LIBC_PATH := libc

# Architecture related header files
ARCHINC  := -I$(WORK_DIR)/arch/aarch64/inc

# Generic header files
HYPERINC := -I$(WORK_DIR)/hypervisor/inc \
			-I$(WORK_DIR)/libc/inc \
			-I$(WORK_DIR)/peripherals/inc \
			-I$(WORK_DIR)/guest-vm/inc

CFLAGS 	:= -g -Wall $(ARCHINC) $(HYPERINC)
ASFLAGS := -g
LDFLAGS := -g

# ----------------------------------------------------------

compile_cc = @ mkdir -p `dirname $(1)`; \
			 echo " (cc)       $(subst $(BUILD_DIR)/,,$(1))"; \
			 $(CC) $(CFLAGS) -c $(2) -o $(1)
compile_as = @ mkdir -p `dirname $(1)`; \
			 echo " (as)       $(subst $(BUILD_DIR)/,,$(1))"; \
			 $(AS) $(ASFLAGS) $(2) -o $(1)
compile_ld = @ mkdir -p `dirname $(1)`; \
			 echo " (ld)       $(subst $(BUILD_DIR)/,,$(1))"; \
			 $(CC) $(3) $(LDFLAGS) -T$(2) -o $(1)
disassemble = @ mkdir -p `dirname $(1)`; \
			 echo " (objdump)  $(subst $(BUILD_DIR)/,,$(1))"; \
			 $(OBJDUMP) -r -S -l -d $(1) > $(BUILD_DIR)/debug.asm
compile_objcopy = @ mkdir -p `dirname $(1)`; \
			 echo " (objcopy)  $(subst $(BUILD_DIR)/,,$(1))"; \
			 $(OBJCOPY) -O binary $(2) $(1)

# ----------------------------------------------------------
OBJS := $(BUILD_DIR)/$(ARCH_PATH)/arch_gic.o \
		$(BUILD_DIR)/$(ARCH_PATH)/arch_vcpu.o \
		$(BUILD_DIR)/$(ARCH_PATH)/arch_vm.o \
		$(BUILD_DIR)/$(ARCH_PATH)/exception.o \
		$(BUILD_DIR)/$(ARCH_PATH)/generic_timer.o \
		$(BUILD_DIR)/$(ARCH_PATH)/handler.o \
		$(BUILD_DIR)/$(ARCH_PATH)/irq.o \
		$(BUILD_DIR)/$(ARCH_PATH)/ptable.o \
		$(BUILD_DIR)/$(ARCH_PATH)/stack_guard.o \
		$(BUILD_DIR)/$(ARCH_PATH)/start.o \
		$(BUILD_DIR)/$(ARCH_PATH)/transition.o \
		$(BUILD_DIR)/$(ARCH_PATH)/vm_control.o \
		$(BUILD_DIR)/$(ARCH_PATH)/gicv3/gicv3_basic.o \
		$(BUILD_DIR)/$(ARCH_PATH)/gicv3/gicv3_cpuif.o \
		$(BUILD_DIR)/$(HYPR_PATH)/hypervisor.o \
		$(BUILD_DIR)/$(HYPR_PATH)/loader.o \
		$(BUILD_DIR)/$(HYPR_PATH)/mm.o \
		$(BUILD_DIR)/$(HYPR_PATH)/scheduler.o \
		$(BUILD_DIR)/$(HYPR_PATH)/vcpu.o \
		$(BUILD_DIR)/$(HYPR_PATH)/vm.o \
		$(BUILD_DIR)/$(LIBC_PATH)/std.o \
		$(BUILD_DIR)/$(LIBC_PATH)/string.o \
		$(BUILD_DIR)/$(PERI_PATH)/uart.o \
		$(BUILD_DIR)/main.o

# ----------------------------------------------------------
# $@ - Target file
# $< - First dependent file
# $^ - All dependent files
$(BUILD_DIR)/$(EXECUTABLE).bin:$(BUILD_DIR)/$(EXECUTABLE).elf
	$(call compile_objcopy,$@,$<)

$(BUILD_DIR)/$(EXECUTABLE).elf:$(OBJS)
	$(call compile_ld,$@,$(WORK_DIR)/vhustler.ld,$^)
	$(call disassemble,$@)

$(BUILD_DIR)/main.o:$(WORK_DIR)/main.c
	$(call compile_cc,$@,$<)

$(BUILD_DIR)/$(ARCH_PATH)/gicv3/%.o:$(WORK_DIR)/$(ARCH_PATH)/gicv3/%.S
	$(call compile_as,$@,$<)

$(BUILD_DIR)/$(ARCH_PATH)/%.o:$(WORK_DIR)/$(ARCH_PATH)/%.S
	$(call compile_as,$@,$<)

$(BUILD_DIR)/$(ARCH_PATH)/%.o:$(WORK_DIR)/$(ARCH_PATH)/%.c
	$(call compile_cc,$@,$<)

$(BUILD_DIR)/$(ARCH_PATH)/gicv3/%.o:$(WORK_DIR)/$(ARCH_PATH)/gicv3/%.c
	$(call compile_cc,$@,$<)

$(BUILD_DIR)/$(HYPR_PATH)/%.o:$(WORK_DIR)/$(HYPR_PATH)/%.c
	$(call compile_cc,$@,$<)

$(BUILD_DIR)/$(LIBC_PATH)/%.o:$(WORK_DIR)/$(LIBC_PATH)/%.c
	$(call compile_cc,$@,$<)

$(BUILD_DIR)/$(PERI_PATH)/%.o:$(WORK_DIR)/$(PERI_PATH)/%.c
	$(call compile_cc,$@,$<)

# ----------------------------------------------------------
.PHONY: clean
clean:
	@rm -rf out/*
# ----------------------------------------------------------
