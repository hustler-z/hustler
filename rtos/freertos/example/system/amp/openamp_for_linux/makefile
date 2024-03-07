PROJECT_DIR = $(CURDIR)
FREERTOS_SDK_DIR = $(CURDIR)/../../../..

USER_CSRC := main.c
USER_CSRC += $(wildcard  src/*.c)

USER_INCLUDE := $(PROJECT_DIR)

include $(FREERTOS_SDK_DIR)/tools/makeall.mk

# 用户定义的编译目标文件上传路径 
ifeq ($(OS),Windows_NT)
	USR_BOOT_DIR		?= $(subst \,/, $(PHYTIUM_DEV_PATH))/tftp
else
	USR_BOOT_DIR		?= /mnt/d/tftboot
endif

# 设置启动镜像名
USER_BOOT_IMAGE    ?= openamp_core0

# 完成编译
image:
	$(MAKE) clean
	$(MAKE) all -j
	@cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).elf
ifdef CONFIG_OUTPUT_BINARY
	@cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).bin
endif
	@ls $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).* -l

config_d2000_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_test_openamp_for_linux

config_d2000_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_test_openamp_for_linux

config_ft2004_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_dsk_openamp_for_linux

config_ft2004_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_dsk_openamp_for_linux

config_e2000d_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_openamp_for_linux

config_e2000d_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_openamp_for_linux

config_e2000q_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_openamp_for_linux

config_e2000q_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_openamp_for_linux

config_phytiumpi_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_openamp_for_linux

config_phytiumpi_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_openamp_for_linux

menuconfig_core0:
	$(MAKE) menuconfig
