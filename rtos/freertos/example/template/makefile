PROJECT_DIR = $(CURDIR)
FREERTOS_SDK_DIR = $(CURDIR)/../..


# # 设置启动镜像名
BOOT_IMG_NAME      ?= freertos

USER_CSRC := main.c
			
USER_ASRC :=  #添加汇编代码
USER_CXXSRC := #添加C++代码

#添加 .h 文件所在目录
USER_INCLUDE := $(PROJECT_DIR)	


include $(FREERTOS_SDK_DIR)/tools/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot

image:
	$(MAKE) all -j
	@cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).elf
ifdef CONFIG_OUTPUT_BINARY
	@cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).bin
endif
	@ls $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).* -l