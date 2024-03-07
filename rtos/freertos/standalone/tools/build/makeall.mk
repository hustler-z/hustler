# 定义环境变量用于承接sdk的路径
SDK_DIR ?= $(CURDIR)/../..
PROJECT_DIR ?= $(CURDIR)
# 用户自定义的变量与路径
IMAGE_NAME ?= baremetal
# 
IMAGE_OUTPUT ?=.


Q ?= @

export Q

MAKEFLAGS += --no-print-directory

ifdef IDE_EXPORT
export DRY_RUN_SRC_OUT_NAME ?= src.map
export DRY_RUN_INC_OUT_NAME ?= inc.map
endif

# sdk tools path
# config path
SDK_KCONFIG_DIR ?= $(SDK_DIR)/tools/build/Kconfiglib
# Pyhthon 工具目录
SDK_PYTHON_TOOLS_DIR ?= $(SDK_DIR)/tools/build/py_tools
# 编译中间文件输出的路径
BUILD_OUT_PATH ?= $(PROJECT_DIR)/build

# export BUILD_OUT_PATH
export SDK_DIR
ifneq ($(wildcard $(SDK_DIR)/tools/build_baremetal.mk),)
$(error SDK_DIR/tools/build_baremetal.mk dose not exist)
endif


all: $(IMAGE_NAME).elf
# user makefile
include $(SDK_DIR)/board/user/user_make.mk
# linker
include $(SDK_DIR)/tools/build/build.mk

ifdef CONFIG_USE_NEWLIB
include $(SDK_DIR)/lib/newlib/makelib.mk
endif

# make menuconfig tools
include $(SDK_DIR)/tools/build/menuconfig/preconfig.mk
include $(SDK_DIR)/tools/build/menuconfig/menuconfig.mk
include $(SDK_DIR)/tools/export_ide/gen_proj.mk

amp_make:
# 基于当前目录下current_config.config 进行amp 操作
	@$(PYTHON) $(SDK_PYTHON_TOOLS_DIR)/amp_parse_config.py
	$(MAKE) -C  $(SDK_DIR)/tools/build/new_boot_code all -j AMP_IMG_EXPORT_IMG=$(PROJECT_DIR)/packed.bin BUILD_OUT_PATH=$(PROJECT_DIR)/build/boot_build IMAGE_OUT_NAME=packed_image IMAGE_OUTPUT=$(PROJECT_DIR)/
	cp ./packed_image.elf $(USR_BOOT_DIR)/baremetal.elf
