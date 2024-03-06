# 为用户准备的提供的对象进行编译
.PHONY: newlibc.a newlibm.a newlibc_clean newlibm_clean

define invoke_make_in_directory 
	@echo "         $(1)"
    @$(MAKE) -C $(SDK_DIR)/$(1) -f $(2) SDK_DIR="$(SDK_DIR)" PROJECT_DIR="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)" $(4) $(3) LIBS_NAME=$@
endef

# 用户定义的编译库文件存放路径
ifeq ($(CONFIG_ARCH_ARMV8_AARCH64),y)
	NEWLIB_DIR	= $(SDK_DIR)/lib/newlib/aarch64
else
	ifeq ($(CONFIG_MFLOAT_ABI_SOFTFP),y)
		NEWLIB_DIR	= $(SDK_DIR)/lib/newlib/aarch32/softfp
	endif
	ifeq ($(CONFIG_MFLOAT_ABI_HARD),y) 
		NEWLIB_DIR	= $(SDK_DIR)/lib/newlib/aarch32/hard
	endif
endif

newlibc.a:
	$(call invoke_make_in_directory,lib/newlib/libc,makefile,all,)
	cp -rf $(BUILD_OUT_PATH)/newlibc.a $(NEWLIB_DIR)/$(CONFIG_ARCH_EXECUTION_STATE)_newlibc_standalone.a

newlibc_debug:
	$(call invoke_make_in_directory,lib/newlib/libc,makefile,debug,)
	
newlibc_info:
	$(call invoke_make_in_directory,lib/newlib/libc,makefile,compiler_info,)

newlibc_clean:
	rm -rf $(BUILD_OUT_PATH)/newlibc.a
	rm -rf $(NEWLIB_DIR)/$(CONFIG_ARCH_EXECUTION_STATE)_newlibc_standalone.a

newlibm.a:
	$(call invoke_make_in_directory,lib/newlib/libm,makefile,all,)
	cp -rf $(BUILD_OUT_PATH)/newlibm.a $(NEWLIB_DIR)/$(CONFIG_ARCH_EXECUTION_STATE)_newlibm_standalone.a

newlibm_debug:
	$(call invoke_make_in_directory,lib/newlib/libm,makefile,debug,)

newlibm_info:
	$(call invoke_make_in_directory,lib/newlib/libm,makefile,compiler_info,)

newlibm_clean:
	rm -rf $(BUILD_OUT_PATH)/newlibm.a
	rm -rf $(NEWLIB_DIR)/$(CONFIG_ARCH_EXECUTION_STATE)_newlibm_standalone.a
	