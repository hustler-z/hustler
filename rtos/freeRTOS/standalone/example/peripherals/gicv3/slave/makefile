
PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../../..


USER_CSRC := main.c
USER_CSRC += src/slave_core_host.c \
			src/slave_core_sgi.c \
			src/slave_core_spi.c 
			
USER_ASRC := 
USER_CXXSRC := 

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc \
				$(PROJECT_DIR)/../common/inc


include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot
boot:
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/baremetal_rc.elf
