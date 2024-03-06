
-include $(PROJECT_DIR)/sdkconfig

ifdef CONFIG_ENABLE_FDC_DP_USE_LIB

DCDP_LIBS		?=

ifeq ($(CONFIG_ARCH_ARMV8_AARCH64),y)
	DCDP_LIBS += $(SDK_DIR)/drivers/media/fdcdp_lib/libfdcdp_standalone_a64.a
else
	ifeq ($(CONFIG_MFLOAT_ABI_SOFTFP),y)
		DCDP_LIBS += $(SDK_DIR)/drivers/media/fdcdp_lib/libfdcdp_standalone_soft_a32.a
	else
		DCDP_LIBS += $(SDK_DIR)/drivers/media/fdcdp_lib/libfdcdp_standalone_hard_a32.a
	endif
endif

SOURCE_DEFINED_LIBS += $(DCDP_LIBS)
endif