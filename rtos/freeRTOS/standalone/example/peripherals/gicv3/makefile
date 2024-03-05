PROJECT_DIR = $(CURDIR)

SDK_DIR ?= $(CURDIR)/../../..

MASTER_PROJECT_DIR	?= $(PROJECT_DIR)/master
SLAVE_PROJECT_DIR	?= $(PROJECT_DIR)/slave

AMP_PATH += $(PROJECT_DIR)/master
AMP_PATH += $(PROJECT_DIR)/slave

include $(SDK_DIR)/tools/build/amp_makeall.mk

USR_BOOT_DIR	?= /mnt/d/tftboot

config_d2000_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_test_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_test_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_test_gic_slave

config_d2000_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_test_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_test_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_test_gic_slave


config_ft2004_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_dsk_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_dsk_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_dsk_gic_slave
	
config_ft2004_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_dsk_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_dsk_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_dsk_gic_slave

config_e2000d_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_gic_slave

config_e2000d_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_gic_slave

config_e2000q_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_gic_slave

config_e2000q_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_gic_slave

config_phytiumpi_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_gic_slave

config_phytiumpi_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_boot
	$(MAKE) -C $(MASTER_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_gic_master
	$(MAKE) -C $(SLAVE_PROJECT_DIR) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_gic_slave

menuconfig_master:
	$(MAKE) -C $(MASTER_PROJECT_DIR) menuconfig

menuconfig_slave:
	$(MAKE) -C $(SLAVE_PROJECT_DIR) menuconfig

backupconfig:
	$(MAKE) backup_kconfig
	$(MAKE) -C $(MASTER_PROJECT_DIR) backup_kconfig
	$(MAKE) -C $(SLAVE_PROJECT_DIR) backup_kconfig

image:
	$(MAKE) clean
	$(MAKE) all -j
	cp ./*.elf /mnt/d/tftboot/baremetal_package.elf


