
# 指定工程项目根目录为当前
PROJECT_DIR = $(CURDIR)
SDK_DIR = $(CURDIR)/../..


USER_CSRC := main.c
USER_CSRC += $(wildcard  src/*.c)

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc


include $(SDK_DIR)/tools/build/makeall.mk

# 用户定义的编译目标文件上传路径 
ifeq ($(OS),Windows_NT)
	USR_BOOT_DIR		?= $(subst \,/, $(PHYTIUM_DEV_PATH))/tftp
else
	USR_BOOT_DIR		?= /mnt/d/tftboot
endif

image:
	make clean
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/baremetal.elf
	cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/baremetal.bin
