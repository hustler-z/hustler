include $(PROJECT_DIR)/sdkconfig

LIBM_CUR_DIR := $(SDK_DIR)/lib/newlib/libm

##########################libc####################################

BUILD_INC_PATH_DIR += $(LIBM_CUR_DIR)/common

BUILD_INC_PATH_DIR += $(LIBM_CUR_DIR)/complex

ifeq ($(CONFIG_ARCH_ARMV8_AARCH64),y)
	BUILD_INC_PATH_DIR += $(LIBM_CUR_DIR)/machine/aarch64
else
	BUILD_INC_PATH_DIR += $(LIBM_CUR_DIR)/machine/arm
endif


