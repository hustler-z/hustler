PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../../..

USER_CSRC := main.c
USER_CSRC += $(wildcard  src/*.c)

USER_INCLUDE := $(PROJECT_DIR)

include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot

# 设置启动镜像名
USER_BOOT_IMAGE    ?= openamp_core0

config_d2000_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_test_openamp_core0

config_d2000_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_test_openamp_core0

config_ft2004_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_dsk_openamp_core0

config_ft2004_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_dsk_openamp_core0

config_e2000d_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_openamp_core0

config_e2000d_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_openamp_core0

config_e2000q_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_openamp_core0

config_e2000q_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_openamp_core0

config_phytiumpi_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_openamp_core0

config_phytiumpi_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_openamp_core0

# 完成编译
image:
	make clean
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).elf
	
scp:
	scp /mnt/d/tftboot/openamp_core0.elf root@192.168.4.51:/lib/firmware/