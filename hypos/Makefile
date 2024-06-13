# ----------------------------------------------------------
# Hustler's Project
#
# File:  Makefile
# Date:  2024/05/20
# Usage: build this bare metal project
# ----------------------------------------------------------
WKDIR       := ./
OUTPUT      := out
HOST        ?= $(shell uname -m | sed -e s/aarch64/arm64/ \
			   -e s/x86_64/x86/)
PLATFORM    ?= rockchip
BOARD       ?= rk3568
ARCH        ?= arm64
BIN         := $(OUTPUT)/hypos.bin
ELF         := $(OUTPUT)/hypos.elf
ifeq ($(HOST),x86)
CROSS_COMPILE ?= /bsp/pro/toolchains/armcc-bm-64/bin/aarch64-none-elf-
else
CROSS_COMPILE ?=
endif
CC          := $(CROSS_COMPILE)gcc
LD          := $(CROSS_COMPILE)ld
AS          := $(CROSS_COMPILE)as
OBJCOPY     := $(CROSS_COMPILE)objcopy
OBJDUMP     := $(CROSS_COMPILE)objdump
READELF     := $(CROSS_COMPILE)readelf
# ----------------------------------------------------------
CFLAGS      += -nostdlib -g -Wall -nostdinc
CFLAGS      += -fno-builtin -fno-common
CFLAGS      += -Wno-unused-variable
LDFLAGS     += -nostdlib -g -Wall
# LDFLAGS     += -Wl,--no-warn-rwx-segments
ASFLAGS     += -nostdlib -g -Wall
ASFLAGS     += -D__ASSEMBLY__

ifeq ($(BOARD),rk3568)
CFLAGS      += -D__RK3568__
ASFLAGS     += -D__RK3568__
endif
# ----------------------------------------------------------
include bsp/arch/$(ARCH)/$(ARCH).mk
include bsp/board/generic/generic.mk
include bsp/board/$(PLATFORM)/$(BOARD)/$(BOARD).mk
include bsp/drivers/drivers.mk
include bsp/bsp.mk
include hyp/hyp.mk
include lib/lib.mk

SPATH_ASMG   := bsp/arch
SPATH_ARCH   := $(SPATH_ASMG)/$(ARCH)
SPATH_BRDG   := bsp/board/generic
SPATH_BRDP   := bsp/board/$(PLATFORM)
SPATH_BRDT   := $(SPATH_BRDP)/$(BOARD)
SPATH_DRIV   := bsp/drivers
SPATH_BSPS   := bsp
SPATH_HYPS   := hyp
SPATH_LIBS   := lib
INCDIRS      += -I$(SPATH_ASMG)/include
INCDIRS      += -I$(SPATH_ARCH)/include
INCDIRS      += -I$(SPATH_BRDG)/include
INCDIRS      += -I$(SPATH_BRDP)/include
INCDIRS      += -I$(SPATH_BSPS)/include
INCDIRS      += -I$(SPATH_LIBS)/include
INCDIRS      += -I$(SPATH_HYPS)/include

PATH_ARCH    := $(OUTPUT)/bsp/arch/$(ARCH)
PATH_BRDG    := $(OUTPUT)/bsp/board/generic
PATH_BRDT    := $(OUTPUT)/bsp/board/$(PLATFORM)/$(BOARD)
PATH_DRIV    := $(OUTPUT)/bsp/drivers
PATH_BSPS    := $(OUTPUT)/bsp
PATH_HYPS    := $(OUTPUT)/hyp
PATH_LIBS    := $(OUTPUT)/lib

ldscript     := bsp/arch/$(ARCH)/hypos.ld

bspsco       := $(addprefix $(PATH_BSPS)/,$(bsp-y))
archco       := $(addprefix $(PATH_ARCH)/,$(armc-y))
archao       := $(addprefix $(PATH_ARCH)/,$(arma-y))
brdgco       := $(addprefix $(PATH_BRDG)/,$(generic-y))
brdtco       := $(addprefix $(PATH_BRDT)/,$($(BOARD)-y))
drivco       := $(addprefix $(PATH_DRIV)/,$(drivers-y))
hypsco       := $(addprefix $(PATH_HYPS)/,$(hyps-y))
libsco       := $(addprefix $(PATH_LIBS)/,$(libs-y))

hyposo       := $(archao) $(archco) $(bspsco) $(brdgco) \
                $(brdtco) $(drivco) $(hypsco) $(libsco)
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
# ----------------------------------------------------------
define all_sources
	(find $(WKDIR) -type f -name '*.[chS]' -print)
endef
# ----------------------------------------------------------
.PHONY: all
all: $(BIN)
$(BIN): $(ELF)
	$(call cc_objcopy,$@,$<)

$(ELF): $(hyposo)
	$(call cc_ld,$@,$(ldscript),$^)
	$(call cc_objdump,$@)
	$(call cc_readelf,$@)

###
# Compile submodules
###
$(archao): $(PATH_ARCH)/%.o: $(SPATH_ARCH)/%.S
	$(call cc_as,$@,$<)
$(archco): $(PATH_ARCH)/%.o: $(SPATH_ARCH)/%.c
	$(call cc_cc,$@,$<)
$(brdgco): $(PATH_BRDG)/%.o: $(SPATH_BRDG)/%.c
	$(call cc_cc,$@,$<)
$(brdtco): $(PATH_BRDT)/%.o: $(SPATH_BRDT)/%.c
	$(call cc_cc,$@,$<)
$(drivco): $(PATH_DRIV)/%.o: $(SPATH_DRIV)/%.c
	$(call cc_cc,$@,$<)
$(bspsco): $(PATH_BSPS)/%.o: $(SPATH_BSPS)/%.c
	$(call cc_cc,$@,$<)
$(hypsco): $(PATH_HYPS)/%.o: $(SPATH_HYPS)/%.c
	$(call cc_cc,$@,$<)
$(libsco): $(PATH_LIBS)/%.o: $(SPATH_LIBS)/%.c
	$(call cc_cc,$@,$<)
# ----------------------------------------------------------
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
	echo "CLEAN    (done)"

.PHONY: help
help:
	@echo "make         - build hypos binary";    \
	 echo "make clean   - clean all objects";     \
	 echo "make tags    - build ctags and gtags"; \
	 echo "make remove  - remove all tags"

# ----------------------------------------------------------
