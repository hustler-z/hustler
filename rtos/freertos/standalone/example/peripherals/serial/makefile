# 指定工程项目根目录为当前
PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../..

-include $(PROJECT_DIR)/sdkconfig

USER_CSRC := main.c

USER_CSRC += src/cmd_serial.c \
		src/serial_intr_example.c \
		src/serial_poll_example.c 
USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc \
				
ifdef CONFIG_E2000Q_DEMO_BOARD
USER_CSRC += src/serial_flow_example.c
USER_CSRC += src/serial_mio_example.c
USER_CSRC += src/serial_ddma_example.c
USER_CSRC += src/serial_ddma_mio_example.c
USER_CSRC += src/serial_ddma_flow_example.c
endif

ifdef CONFIG_E2000D_DEMO_BOARD
USER_CSRC += src/serial_flow_example.c
USER_CSRC += src/serial_mio_example.c
USER_CSRC += src/serial_ddma_example.c
USER_CSRC += src/serial_ddma_mio_example.c
USER_CSRC += src/serial_ddma_flow_example.c
endif

ifdef CONFIG_FIREFLY_DEMO_BOARD
USER_CSRC += src/serial_mio_example.c
USER_CSRC += src/serial_ddma_example.c
USER_CSRC += src/serial_ddma_mio_example.c
endif

include $(SDK_DIR)/tools/build/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot

image:
	make clean
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/baremetal.elf



