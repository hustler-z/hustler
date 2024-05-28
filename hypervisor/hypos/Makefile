# ----------------------------------------------------------
# Hustler's Project
#
# File:  Makefile
# Date:  2024/05/20
# Usage: build this bare metal project
# ----------------------------------------------------------
WKDIR       := ./
OUTPUT      := out
ARCH        ?= $(shell uname -m | sed -e s/aarch64/arm64/ \
			   -e s/x86_64/x86/)
BOARD       ?= rk3568
TARCH       ?= arm64
BIN         := $(OUTPUT)/hypos.bin
ELF         := $(OUTPUT)/hypos.elf
ifeq ($(ARCH),x86)
CROSS_COMPILE ?= /bsp/pro/toolchains/armcc-64/bin/aarch64-none-linux-gnu-
else
CROSS_COMPILE ?=
endif
CC          := $(CROSS_COMPILE)gcc
LD          := $(CROSS_COMPILE)ld
AS          := $(CROSS_COMPILE)as
OBJCOPY     := $(CROSS_COMPILE)objcopy
OBJDUMP     := $(CROSS_COMPILE)objdump
# ----------------------------------------------------------
ARCH_PATH   := arch/arm64
BOARD_PATH  := bsp/cpu/$(BOARD)
CPU_PATH    := bsp/cpu/common
BSP_PATH    := bsp
COMN_PATH   := common
VCORE_PATH  := vcore

ARCH_OUT    := $(OUTPUT)/arch/arm64
BOARD_OUT   := $(OUTPUT)/bsp/cpu/$(BOARD)
CPU_OUT     := $(OUTPUT)/bsp/cpu/common
BSP_OUT     := $(OUTPUT)/bsp
COMN_OUT    := $(OUTPUT)/common
VCORE_OUT   := $(OUTPUT)/vcore
# ----------------------------------------------------------
LDSCRIPT    := $(ARCH_PATH)/arch_boot.lds.S

INCDIRS     += -I$(ARCH_PATH)/include
INCDIRS     += -I$(BOARD_PATH)/include
INCDIRS     += -I$(CPU_PATH)/include
INCDIRS     += -I$(BSP_PATH)/include
INCDIRS     += -I$(VCORE_PATH)/include
INCDIRS     += -I$(COMN_PATH)/include

CFLAGS      += -nostdlib -g -Wall
CFLAGS      += -fno-builtin -fno-common
LDFLAGS     += -nostdlib -g -Wall

ASFLAGS     += -nostdlib -g -Wall
ASFLAGS     += -D__ASSEMBLY__
# ----------------------------------------------------------
ifeq ($(V),1)
	Q :=
else
	Q := @
endif
# ----------------------------------------------------------
cc_objdump = $(Q)mkdir -p `dirname $(1)`; \
             echo "OBJDUMP  $(patsubst %.elf,%.asm,$(subst $(OUTPUT)/,,$(1)))"; \
             $(OBJDUMP) -S -d $(1) > $(patsubst %.elf,%.asm,$(1))

cc_objcopy = $(Q)mkdir -p `dirname $(1)`; \
             echo "OBJCOPY  $(subst $(OUTPUT)/,,$(1))"; \
             $(OBJCOPY) -O binary $(2) $(1)

cc_ld      = $(Q)mkdir -p `dirname $(1)`; \
             echo "LD       $(subst $(OUTPUT)/,,$(1))"; \
             $(CC) $(3) $(LDFLAGS) $(INCDIRS) -T$(2) -o $(1)

cc_cc      = $(Q)mkdir -p `dirname $(1)`; \
             echo "CC       $(subst $(OUTPUT)/,,$(1))"; \
             $(CC) $(CFLAGS) $(INCDIRS) -c $(2) -o $(1)

cc_as      = $(Q)mkdir -p `dirname $(1)`; \
             echo "AS       $(subst $(OUTPUT)/,,$(1))"; \
             $(CC) $(ASFLAGS) $(INCDIRS) -c $(2) -o $(1)
# ----------------------------------------------------------
include arch/arm64/arm64.mk
include bsp/bsp.mk
include bsp/cpu/common/cpu.mk
ifeq ($(BOARD),rk3568)
include bsp/cpu/rk3568/rk3568.mk
endif

include common/common.mk
include vcore/vcore.mk
# ----------------------------------------------------------
all_sources := $(shell find $(WKDIR) -name '*.[chS]')

arch_aobjs  := $(addprefix $(ARCH_OUT)/, $(arm64-asm-y))
arch_cobjs  := $(addprefix $(ARCH_OUT)/, $(arm64-cc-y))
board_cobjs := $(addprefix $(BOARD_OUT)/,$(board-y))
comn_cobjs  := $(addprefix $(COMN_OUT)/,$(common-y))
cpu_cobjs   := $(addprefix $(CPU_OUT)/,$(cpu-y))
bsp_cobjs   := $(addprefix $(BSP_OUT)/,$(bsp-y))
vcore_cobjs := $(addprefix $(VCORE_OUT)/,$(vcore-y))

hypos_cobjs := $(arch_aobjs)  \
               $(arch_cobjs)  \
               $(cpu_cobjs)   \
               $(board_cobjs) \
               $(bsp_cobjs)   \
               $(comn_cobjs)  \
               $(vcore_cobjs)
# ----------------------------------------------------------
.PHONY: all

all: $(BIN)
$(BIN): $(ELF)
	$(call cc_objcopy,$@,$<)

$(ELF): $(hypos_cobjs)
	$(call cc_ld,$@,$(LDSCRIPT),$^)
	$(call cc_objdump,$@)

###
# Compile submodules
###
$(arch_aobjs):  $(ARCH_OUT)/%.o: $(ARCH_PATH)/%.S
	$(call cc_as,$@,$<)
$(arch_cobjs):  $(ARCH_OUT)/%.o: $(ARCH_PATH)/%.c
	$(call cc_cc,$@,$<)
$(comn_cobjs):  $(COMN_OUT)/%.o: $(COMN_PATH)/%.c
	$(call cc_cc,$@,$<)
$(bsp_cobjs):   $(BSP_OUT)/%.o: $(BSP_PATH)/%.c
	$(call cc_cc,$@,$<)
$(board_cobjs): $(BOARD_OUT)/%.o: $(BOARD_PATH)/%.c
	$(call cc_cc,$@,$<)
$(cpu_cobjs):   $(CPU_OUT)/%.o: $(CPU_PATH)/%.c
	$(call cc_cc,$@,$<)
$(vcore_cobjs): $(VCORE_OUT)/%.o: $(VCORE_PATH)/%.c
	$(call cc_cc,$@,$<)
# ----------------------------------------------------------
.PHONY: tags
tags:
	@set -e; rm -rf tags GTAGS GPATH GRTAGS; \
	$(all_sources) | gtags -f -;             \
	$(all_sources) | xargs ctags --languages=Asm,c,c++,Make -R

.PHONY: clean
clean:
	@rm -rf $(OUTPUT) tags GTAGS GPATH GRTAGS
# ----------------------------------------------------------
