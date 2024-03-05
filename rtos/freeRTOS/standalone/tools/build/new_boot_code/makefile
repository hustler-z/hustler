PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../..

# packed.bin 的路径
AMP_IMG_EXPORT_IMG ?= 
# boot 目标文件存在地址
BUILD_OUT_PATH ?= $(PROJECT_DIR)/build

IMAGE_OUT_NAME ?= packed_image

IMAGE_NAME ?= boot

USER_CSRC := main.c

USER_INCLUDE := $(PROJECT_DIR)
# AMP 镜像打包相关路径
AMP_PACK_FILE = ./amp_pack.c

AMP_PACK_OBJS :=  $(AMP_PACK_FILE:.c=.o)


$(BUILD_OUT_PATH)/$(AMP_PACK_OBJS):$(AMP_PACK_FILE) $(AMP_IMG_EXPORT_IMG)
	@echo $(BUILD_OUT_PATH)/$(AMP_PACK_OBJS)	
	@mkdir -p $(BUILD_OUT_PATH)/
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -c   $(AMP_PACK_FILE) -o $(BUILD_OUT_PATH)/$(AMP_PACK_OBJS)
	$(OBJCOPY) --add-section .my_amp_img=$(AMP_IMG_EXPORT_IMG) --set-section-flags .my_amp_img=alloc,load,readonly $(BUILD_OUT_PATH)/$(AMP_PACK_OBJS)

EXTRA_OBJS += $(BUILD_OUT_PATH)/$(AMP_PACK_OBJS)

$(IMAGE_NAME).elf:gen_kconfig

include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot
image:
	make clean
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/baremetal.elf





