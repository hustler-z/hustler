# 指定工程项目根目录为当前
PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../..
-include $(PROJECT_DIR)/sdkconfig

USER_CSRC := main.c
USER_CSRC += src/cmd_sd.c
USER_INCLUDE := $(PROJECT_DIR)

ifdef CONFIG_FSL_SDMMC_USE_FSDIF
USER_CSRC += $(wildcard  src/sdif/*.c)
USER_INCLUDE += $(PROJECT_DIR)/inc/sdif
endif

ifdef CONFIG_FSL_SDMMC_USE_FSDMMC
USER_CSRC += $(wildcard  src/sdmmc/*.c)
USER_INCLUDE += $(PROJECT_DIR)/inc/sdmmc
endif

include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot
image:
	make clean
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/baremetal.elf
	cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/baremetal.bin