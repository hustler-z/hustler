PROJECT_DIR = $(CURDIR)
SDK_DIR ?= $(CURDIR)/../../../
include sdkconfig

USER_CSRC := main.c
USER_CSRC += $(wildcard  src/*.c)
USER_ASRC += $(wildcard  src/*.S)
ifdef CONFIG_ENABLE_CXX
USER_CXXSRC := $(wildcard  src/*.cpp)
endif

USER_INCLUDE := $(PROJECT_DIR) \
				$(PROJECT_DIR)/inc

ifeq ($(OS),Windows_NT)
# OPENOCD_CMD := $(SDK_DIR)/tools/debugging/run_openocd_jlink.bat
OPENOCD_CMD := $(SDK_DIR)/tools/debugging/run_openocd_dap.bat
else
# OPENOCD_CMD := $(SDK_DIR)/tools/debugging/run_openocd_jlink.sh
OPENOCD_CMD := $(SDK_DIR)/tools/debugging/run_openocd_dap.sh
endif

include $(SDK_DIR)/tools/build/makeall.mk

image:
	make all -j
	cp ./$(IMAGE_OUT_NAME).elf ./baremetal.elf

run_openocd: image
	$(OPENOCD_CMD)

run_gdb:
ifdef CONFIG_TARGET_ARMV8_AARCH32
	@$(PYTHON) ./.aarch32_mode_change 
endif
	$(GDB) -x ./.gdbinit