PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../../../..


USER_CSRC := main.c
USER_CSRC += $(wildcard  src/*.c)
USER_CSRC += $(wildcard  ../common/*.c)
			
USER_ASRC := 
USER_CXXSRC := 

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc \
				$(PROJECT_DIR)/../common


include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot

load_d2000_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_test_apu

load_d2000_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_test_apu

load_ft2004_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_dsk_apu

load_ft2004_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_dsk_apu

load_e2000d_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_apu

load_e2000d_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_apu

load_e2000q_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_apu

load_e2000q_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_apu

load_phytiumpi_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_apu

load_phytiumpi_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_apu

boot:
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/apu_libmetal_core0.elf
