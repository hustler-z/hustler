include $(PROJECT_DIR)/sdkconfig


BUILD_PATH := $(BUILD_OUT_PATH)/$(shell pwd)

CSRCS_RELATIVE_FILES := fassert.c \
				fdebug.c \
				felf.c \
				finterrupt.c \
				fprintf.c \
				fprintk.c \
				fsleep.c  \
				fbitmap.c

ifneq ($(strip $(BUILD_IMAGE_CORE_NUM)),0x1000) # 0x1000 是未定义初始化值，
CSRCS_RELATIVE_FILES += fimage_info.c
MODULE_AMP_DEFINED_CFLAGS += -DCONFIG_IMAGE_CORE=$(BUILD_IMAGE_CORE_NUM) -DCONFIG_IMAGE_DELAY=$(BUILD_IMAGE_DELAY_NUM) -DCONFIG_IMAGE_INFO
endif

include $(SDK_DIR)/tools/build/compiler.mk