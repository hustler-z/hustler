PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../../../..

USER_CSRC := $(wildcard  ./*.c)

USER_INCLUDE := $(PROJECT_DIR)

include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot

# 设置启动镜像名
USER_BOOT_IMAGE    ?= openamp_driver_core

# 完成编译
boot:
	make all -j
	@cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).elf