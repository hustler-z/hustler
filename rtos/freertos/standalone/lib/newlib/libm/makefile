include $(PROJECT_DIR)/sdkconfig

include common/src.mk
include complex/src.mk
include fenv/src.mk
include math/src.mk

ifeq ($(CONFIG_ARCH_ARMV8_AARCH64),y)
	include machine/aarch64/src.mk
else
	include machine/arm/src.mk
endif

CSRCS_RELATIVE_FILES := $(foreach file, $(LIBM_CSRCS), $(wildcard common/$(file)))
ASRCS_RELATIVE_FILES := $(foreach file, $(LIBM_CSRCS), $(wildcard common/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard complex/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard complex/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard fenv/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard fenv/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard math/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard math/$(file)))


ifeq ($(CONFIG_ARCH_ARMV8_AARCH64),y)
	CSRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard machine/$(file)))
	ASRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard machine/$(file)))
else
	CSRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard machine/$(file)))
	ASRCS_RELATIVE_FILES += $(foreach file, $(LIBM_CSRCS), $(wildcard machine/$(file)))
endif

include $(SDK_DIR)/tools/build/compiler.mk
