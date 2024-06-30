# ---------------------------------------------------------------
# Hustler's Project
#
# File:  Makefile
# Date:  2024/05/20
# Usage: build this bare metal project
# ---------------------------------------------------------------
include cfg/config.mk
# ---------------------------------------------------------------
WKDIR       := ./
OUTPUT      := out
HOST        ?= $(shell uname -m | sed -e s/aarch64/arm64/ \
			   -e s/x86_64/x86/)
PLATFORM    ?= rockchip
BOARD       ?= rk3568
BASE        ?= arm
ARCH        ?= arm64
BIN         := $(OUTPUT)/hypos.bin
ELF         := $(OUTPUT)/hypos.elf
ELFI        := $(OUTPUT)/hypos.elf.tmp
MAP         := $(OUTPUT)/hypos.map
ifeq ($(HOST),x86)
CROSS_COMPILE ?= /bsp/pro/toolchains/armcc-bm-64/bin/aarch64-none-elf-
else
CROSS_COMPILE ?=
endif
CC          := $(CROSS_COMPILE)gcc
CX          := $(CROSS_COMPILE)cpp
LD          := $(CROSS_COMPILE)ld
AS          := $(CROSS_COMPILE)as
NM          := $(CROSS_COMPILE)nm
OBJCOPY     := $(CROSS_COMPILE)objcopy
OBJDUMP     := $(CROSS_COMPILE)objdump
READELF     := $(CROSS_COMPILE)readelf
# ---------------------------------------------------------------
ifeq ($(CFG_FRAME_POINTER), y)
CFLAGS      += -fno-omit-frame-pointer
else
CFLAGS      += -fomit-frame-pointer
endif
# ---------------------------------------------------------------
ifeq ($(CFG_LTO), y)
CFLAGS      += -flto
endif
# ---------------------------------------------------------------
ifeq ($(CFG_DEBUG), y)
CFLAGS      += -Og
else
CFLAGS      += -O2
endif
# ---------------------------------------------------------------
CFLAGS      += -nostdlib -g -Wall -nostdinc
CFLAGS      += -std=gnu11 -fno-builtin -fno-common
CFLAGS      += -Wno-unused-variable -Wno-pointer-arith
CFLAGS      += -mno-outline-atomics -Wno-unused-function
LDFLAGS     += -nostdlib -g -Wall
# LDFLAGS     += -Wl,--no-warn-rwx-segments
ASFLAGS     += -nostdlib -g -Wall
ASFLAGS     += -D__ASSEMBLY__

ifeq ($(BOARD),rk3568)
CFLAGS      += -D__RK3568__
ASFLAGS     += -D__RK3568__
endif
# ---------------------------------------------------------------
# Source Tree Setup
# ---------------------------------------------------------------
SPATH_BSPS   := bsp

include $(SPATH_BSPS)/arch/$(BASE)/$(ARCH)/$(ARCH).mk
include $(SPATH_BSPS)/arch/$(BASE)/$(BASE).mk
include $(SPATH_BSPS)/common/common.mk
include $(SPATH_BSPS)/board/$(PLATFORM)/$(BOARD)/$(BOARD).mk
include $(SPATH_BSPS)/drivers/drivers.mk
include $(SPATH_BSPS)/lib/lib.mk
include $(SPATH_BSPS)/core/core.mk

SPATH_ARCX   := $(SPATH_BSPS)/arch/$(BASE)
SPATH_ARCH   := $(SPATH_ARCX)/$(ARCH)
SPATH_COMN   := $(SPATH_BSPS)/common
SPATH_BRDP   := $(SPATH_BSPS)/board/$(PLATFORM)
SPATH_BRDT   := $(SPATH_BRDP)/$(BOARD)
SPATH_DRIV   := $(SPATH_BSPS)/drivers
SPATH_LIBS   := $(SPATH_BSPS)/lib
SPATH_HYPS   := $(SPATH_BSPS)/core

INCDIRS      += -I$(SPATH_ARCX)/include
INCDIRS      += -I$(SPATH_ARCH)/include
INCDIRS      += -I$(SPATH_COMN)/include
INCDIRS      += -I$(SPATH_BRDP)/include
INCDIRS      += -I$(SPATH_BSPS)/include
INCDIRS      += -I$(SPATH_LIBS)/include
INCDIRS      += -I$(SPATH_HYPS)/include

PATH_ARCX    := $(OUTPUT)/$(SPATH_BSPS)/arch/$(BASE)
PATH_ARCH    := $(OUTPUT)/$(SPATH_BSPS)/arch/$(BASE)/$(ARCH)
PATH_COMN    := $(OUTPUT)/$(SPATH_BSPS)/common
PATH_BRDT    := $(OUTPUT)/$(SPATH_BSPS)/board/$(PLATFORM)/$(BOARD)
PATH_DRIV    := $(OUTPUT)/$(SPATH_BSPS)/drivers
PATH_LIBS    := $(OUTPUT)/$(SPATH_BSPS)/lib
PATH_HYPS    := $(OUTPUT)/$(SPATH_HYPS)

ASM_OFS_H   := $(SPATH_ARCH)/include/asm/offset.h
ASM_OFS_C   := $(SPATH_ARCH)/offset.c

lds          := $(addprefix $(PATH_ARCH)/,$(lds-y))
ifeq ($(BASE), arm)
archxo       := $(addprefix $(PATH_ARCX)/,$(arm-y))
archco       := $(addprefix $(PATH_ARCH)/,$(armc-y))
archao       := $(addprefix $(PATH_ARCH)/,$(arma-y))
endif
comnco       := $(addprefix $(PATH_COMN)/,$(common-y))
brdtco       := $(addprefix $(PATH_BRDT)/,$($(BOARD)-y))
drivco       := $(addprefix $(PATH_DRIV)/,$(drivers-y))
hypsco       := $(addprefix $(PATH_HYPS)/,$(core-y))
libsco       := $(addprefix $(PATH_LIBS)/,$(libs-y))

hyposo       := $(archao) $(archco) $(archxo) \
                $(comnco) $(brdtco) $(drivco) \
                $(libsco) $(hypsco)

comsym       := $(PATH_COMN)/sym.o
hyposa       := $(archao) $(archco) $(archxo) \
                $(comnco) $(brdtco) $(drivco) \
                $(libsco) $(hypsco) $(comsym)
# ---------------------------------------------------------------
ifeq ($(V),1)
	Q :=
else
	Q := @
endif
# ---------------------------------------------------------------

SYSTEM_MAP = $(NM) $1 | \
             grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
             LC_ALL=C sort
cc_smap    = $(Q)mkdir -p `dirname $(1)`; \
             smap=`$(call SYSTEM_MAP,$(ELF)) | \
             awk '$$2 ~ /[tTwW]/ {printf $$1 $$3 "\\\\000"}'`; \
			 echo "GEN      $(subst $(OUTPUT)/,,$(1))"; \
             $(CC) $(CFLAGS) -DSYSTEM_MAP="\"$${smap}\"" \
             -c $(2) -o $(1)
# ---------------------------------------------------------------
# Compile Quick Macros
# ---------------------------------------------------------------
cc_objdump = $(Q)mkdir -p `dirname $(1)`; \
             echo "OBJDUMP  $(patsubst %.elf,%.asm,$(subst $(OUTPUT)/,,$(1)))"; \
             $(OBJDUMP) -S -r -l -d $(1) > $(patsubst %.elf,%.asm,$(1))

cc_readelf = $(Q)mkdir -p `dirname $(1)`; \
             echo "READELF  $(patsubst %.elf,%.sec,$(subst $(OUTPUT)/,,$(1)))"; \
             $(READELF) -S -lW $(1) > $(patsubst %.elf,%.sec,$(1)); \
             echo "OBJDUMP  $(patsubst %.elf,%.hdr,$(subst $(OUTPUT)/,,$(1)))"; \
             $(OBJDUMP) -h $(1) > $(patsubst %.elf,%.hdr,$(1))

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

cc_nm      = $(Q)mkdir -p `dirname $(1)`; \
             echo "SYMTABS  $(subst $(OUTPUT)/,,$(1))"; \
             $(call SYSTEM_MAP,$(2)) > $(1)

cc_lds     = $(Q)mkdir -p `dirname $(1)`; \
             echo "LDS      $(subst $(OUTPUT)/,,$(1))"; \
             $(CX) -P $(ASFLAGS) $(INCDIRS) -DLINKER_SCRIPT -o $(1) $(2)
# ---------------------------------------------------------------
define all_sources
	(find $(WKDIR) -type f -name '*.[chS]' -print)
endef
# ---------------------------------------------------------------
# Build HYPOS Binary
# ---------------------------------------------------------------
.PHONY: all
all: $(BIN) $(MAP)
$(BIN): $(ELF)
	$(call cc_objcopy,$@,$<)

$(MAP): $(ELF)
	$(call cc_nm,$@,$<)

$(ELF): $(ASM_OFS_H) $(hyposo) $(lds)
	$(call cc_ld,$@,$(lds),$(filter-out $(ASM_OFS_H),$(filter-out $(lds),$^)))
	$(call cc_objdump,$@)
	$(call cc_readelf,$@)
# ---------------------------------------------------------------
$(PATH_COMN)/sym.o: $(SPATH_COMN)/sym.c
	$(call cc_smap,$@,$<)
# ---------------------------------------------------------------
# Generate asm/offset.h
# ---------------------------------------------------------------
$(ASM_OFS_H): offset.S
	$(call asm_offsets)
	$(Q)rm -f $<
offset.S: $(ASM_OFS_C)
	$(Q)$(CC) $(CFLAGS) $(INCDIRS) -S -g0 -o $@ $<

# ---------------------------------------------------------------
# Compile Submodules
# ---------------------------------------------------------------
$(archao): $(PATH_ARCH)/%.o: $(SPATH_ARCH)/%.S
	$(call cc_as,$@,$<)
$(archxo): $(PATH_ARCX)/%.o: $(SPATH_ARCX)/%.c
	$(call cc_cc,$@,$<)
$(archco): $(PATH_ARCH)/%.o: $(SPATH_ARCH)/%.c
	$(call cc_cc,$@,$<)
$(comnco): $(PATH_COMN)/%.o: $(SPATH_COMN)/%.c
	$(call cc_cc,$@,$<)
$(brdtco): $(PATH_BRDT)/%.o: $(SPATH_BRDT)/%.c
	$(call cc_cc,$@,$<)
$(drivco): $(PATH_DRIV)/%.o: $(SPATH_DRIV)/%.c
	$(call cc_cc,$@,$<)
$(hypsco): $(PATH_HYPS)/%.o: $(SPATH_HYPS)/%.c
	$(call cc_cc,$@,$<)
$(libsco): $(PATH_LIBS)/%.o: $(SPATH_LIBS)/%.c
	$(call cc_cc,$@,$<)
$(lds): $(PATH_ARCH)/%.lds: $(SPATH_ARCH)/%.lds.S
	$(call cc_lds,$@,$<)
# ---------------------------------------------------------------
.PHONY: tags
tags:
	@set -e; rm -rf tags GTAGS GPATH GRTAGS; \
	echo "TAGS     (done)";                  \
	$(all_sources) | gtags -f -;             \
	$(all_sources) | xargs ctags --languages=Asm,c,c++,Make -R

.PHONY: remove
remove:
	@rm -rf tags GTAGS GPATH GRTAGS;         \
	echo "TAGS     (removed)"

.PHONY: clean
clean:
	@rm -rf $(OUTPUT);                       \
	rm -f $(ASM_OFS_H);                      \
	echo "CLEAN    (done)"

.PHONY: help
help:
	@echo "make         - build hypos binary";    \
	 echo "make clean   - clean all objects";     \
	 echo "make tags    - build ctags and gtags"; \
	 echo "make remove  - remove all tags"
# ---------------------------------------------------------------
