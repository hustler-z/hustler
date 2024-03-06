
# makefile 目标为收集所有路径相关的内容，方便在预编译的是否，放遍不同模块的调用
# Output --- BUILD_INC_PATH_DIR
include $(PROJECT_DIR)/sdkconfig
# common

BUILD_INC_PATH_DIR +=  $(SDK_DIR)/common

# libc
BUILD_INC_PATH_DIR +=  $(SDK_DIR)/lib/libc

ifdef CONFIG_USE_NEWLIB
include $(SDK_DIR)/lib/newlib/include.mk
endif

# arch
include $(SDK_DIR)/arch/include.mk

# drivers

include $(SDK_DIR)/drivers/include.mk

# soc
include $(SDK_DIR)/soc/include.mk

#board
include $(SDK_DIR)/board/include.mk

# config
BUILD_INC_PATH_DIR += $(PROJECT_DIR)

ifdef CONFIG_USE_BAREMETAL
# third party
include $(SDK_DIR)/third-party/include.mk

endif
