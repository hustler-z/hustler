include $(PROJECT_DIR)/sdkconfig

LIBC_CUR_DIR := $(SDK_DIR)/lib/newlib/libc

##########################libc####################################
ifeq ($(CONFIG_ARCH_ARMV8_AARCH64),y)
	BUILD_INC_PATH_DIR += $(LIBC_CUR_DIR)/machine/aarch64
else
	BUILD_INC_PATH_DIR += $(LIBC_CUR_DIR)/machine/arm
endif

BUILD_INC_PATH_DIR += $(LIBC_CUR_DIR)/include/$(CONFIG_ARCH_EXECUTION_STATE)

BUILD_INC_PATH_DIR += $(LIBC_CUR_DIR)/include

BUILD_INC_PATH_DIR += $(LIBC_CUR_DIR)/include/include-fixed

BUILD_INC_PATH_DIR += $(LIBC_CUR_DIR)/search

BUILD_INC_PATH_DIR += $(LIBC_CUR_DIR)/stdlib

BUILD_INC_PATH_DIR += $(LIBC_CUR_DIR)/stdio

BUILD_INC_PATH_DIR += $(LIBC_CUR_DIR)/time
