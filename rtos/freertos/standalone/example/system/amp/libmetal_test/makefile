PROJECT_DIR = $(CURDIR)

SDK_DIR ?= $(CURDIR)/../../../..


AMP_PATH += $(PROJECT_DIR)/apu_running
AMP_PATH += $(PROJECT_DIR)/rpu_running

include $(SDK_DIR)/tools/build/amp_makeall.mk

USR_BOOT_DIR	?= /mnt/d/tftboot

image:
	$(MAKE) clean
	$(MAKE) all -j
	cp ./*.elf /mnt/d/tftboot/baremetal_package.elf

load_d2000_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_test_boot
	$(MAKE) -C ./apu_running load_d2000_aarch32
	$(MAKE) -C ./rpu_running load_d2000_aarch32

load_d2000_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_test_boot
	$(MAKE) -C ./apu_running load_d2000_aarch64
	$(MAKE) -C ./rpu_running load_d2000_aarch64

load_ft2004_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_dsk_boot
	$(MAKE) -C ./apu_running load_ft2004_aarch32
	$(MAKE) -C ./rpu_running load_ft2004_aarch32

load_ft2004_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_dsk_boot
	$(MAKE) -C ./apu_running load_ft2004_aarch64
	$(MAKE) -C ./rpu_running load_ft2004_aarch64

load_e2000d_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_boot
	$(MAKE) -C ./apu_running load_e2000d_aarch32
	$(MAKE) -C ./rpu_running load_e2000d_aarch32

load_e2000d_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_boot
	$(MAKE) -C ./apu_running load_e2000d_aarch64
	$(MAKE) -C ./rpu_running load_e2000d_aarch64

load_e2000q_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_boot
	$(MAKE) -C ./apu_running load_e2000q_aarch32
	$(MAKE) -C ./rpu_running load_e2000q_aarch32

load_e2000q_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_boot
	$(MAKE) -C ./apu_running load_e2000q_aarch64
	$(MAKE) -C ./rpu_running load_e2000q_aarch64

load_phytiumpi_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_boot
	$(MAKE) -C ./apu_running load_phytiumpi_aarch32
	$(MAKE) -C ./rpu_running load_phytiumpi_aarch32

load_phytiumpi_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_boot
	$(MAKE) -C ./apu_running load_phytiumpi_aarch64
	$(MAKE) -C ./rpu_running load_phytiumpi_aarch64

backupconfig:
	$(MAKE) backup_kconfig
	$(MAKE) -C ./apu_running backup_kconfig
	$(MAKE) -C ./rpu_running backup_kconfig

menuconfig_apu:
	$(MAKE) -C ./apu_running menuconfig

menuconfig_rpu:
	$(MAKE) -C ./rpu_running menuconfig
