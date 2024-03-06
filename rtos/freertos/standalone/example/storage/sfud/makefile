PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../..

USER_CSRC := main.c
USER_CSRC += $(wildcard  src/*.c)

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc \

include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot
image:
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/baremetal.elf
	cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/baremetal.bin