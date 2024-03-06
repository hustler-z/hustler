PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../..
-include $(PROJECT_DIR)/sdkconfig

USER_CSRC := main.c
USER_CSRC += src/cmd_fatfs.c

ifeq ($(CONFIG_FATFS_RAM_DISK),y)
USER_CSRC += src/ram_fatfs_example.c
endif

ifeq ($(CONFIG_FATFS_SDMMC_FSDMMC_TF),y)
USER_CSRC += src/tf_fatfs_example.c
else ifeq ($(CONFIG_FATFS_SDMMC_FSDIF_TF),y)
USER_CSRC += src/tf_fatfs_example.c
endif

ifeq ($(CONFIG_FATFS_SDMMC_FSDIF_EMMC),y)
USER_CSRC += src/emmc_fatfs_example.c
endif

ifeq ($(CONFIG_FATFS_SATA_DISK),y)
USER_CSRC += src/sata_fatfs_example.c
endif

ifeq ($(CONFIG_FATFS_FSATA_PCIE),y)
USER_CSRC += src/sata_pcie_fatfs_example.c
endif

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc \

include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot
image:
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/baremetal.elf
	cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/baremetal.bin