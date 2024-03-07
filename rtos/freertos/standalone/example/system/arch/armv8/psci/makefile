PROJECT_DIR = $(CURDIR)

SDK_DIR ?= $(CURDIR)/../../../../..


AMP_PATH += $(PROJECT_DIR)/psci_test
AMP_PATH += $(PROJECT_DIR)/psci_test/psci_slave

include $(SDK_DIR)/tools/build/amp_makeall.mk

USR_BOOT_DIR	?= /mnt/d/tftboot

config_e2000d_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_boot
	$(MAKE) -C ./psci_test load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_psci
	$(MAKE) -C ./psci_test/psci_slave load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_psci_slave

config_e2000d_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_boot
	$(MAKE) -C ./psci_test load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_psci
	$(MAKE) -C ./psci_test/psci_slave load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_psci_slave

config_e2000q_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_boot
	$(MAKE) -C ./psci_test load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_psci
	$(MAKE) -C ./psci_test/psci_slave load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_psci_slave

config_e2000q_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_boot
	$(MAKE) -C ./psci_test load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_psci
	$(MAKE) -C ./psci_test/psci_slave load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_psci_slave

config_phytiumpi_aarch32:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_boot
	$(MAKE) -C ./psci_test load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_psci
	$(MAKE) -C ./psci_test/psci_slave load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_psci_slave


config_phytiumpi_aarch64:
	$(MAKE) load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_boot
	$(MAKE) -C ./psci_test load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_psci
	$(MAKE) -C ./psci_test/psci_slave load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_psci_slave

boot:
	$(MAKE) clean
	$(MAKE) all -j
	cp ./*.elf /mnt/d/tftboot/boot.elf
	cp $(PROJECT_DIR)/psci_test/*.elf /mnt/d/tftboot/
	cp $(PROJECT_DIR)/psci_test/psci_slave/*.elf /mnt/d/tftboot/
	 

