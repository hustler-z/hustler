include $(PROJECT_DIR)/sdkconfig

ifeq ($(CONFIG_USE_NEWLIB),y)

NEWLIB_CUR_DIR := $(SDK_DIR)/lib/newlib

NEWLIB_LIB_DIR		?=
NEWLIBC		?=

ifeq ($(CONFIG_ARCH_ARMV8_AARCH64),y)
	NEWLIBC += $(NEWLIB_CUR_DIR)/aarch64/aarch64_newlibc_standalone.a
	NEWLIBC += $(NEWLIB_CUR_DIR)/aarch64/aarch64_newlibm_standalone.a
else
	ifeq ($(CONFIG_MFLOAT_ABI_SOFTFP),y)
		NEWLIB_LIB_DIR	= $(NEWLIB_CUR_DIR)/aarch32/softfp
	endif
	ifeq ($(CONFIG_MFLOAT_ABI_HARD),y) 
		NEWLIB_LIB_DIR	= $(NEWLIB_CUR_DIR)/aarch32/hard
	endif
	NEWLIBC += $(NEWLIB_LIB_DIR)/aarch32_newlibc_standalone.a
	NEWLIBC += $(NEWLIB_LIB_DIR)/aarch32_newlibm_standalone.a
endif

EXTRALIBS += $(NEWLIBC)

endif