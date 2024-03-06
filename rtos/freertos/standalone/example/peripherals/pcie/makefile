
PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../..


USER_CSRC := main.c
USER_CSRC += src/cmd_pcie.c \
			 src/rc_enumerate_example.c


			
USER_ASRC := 
USER_CXXSRC := 

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc


include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot
boot:
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/baremetal.elf
