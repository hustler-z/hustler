ifdef CONFIG_USE_LIBMETAL


ifdef CONFIG_USE_FREERTOS

	CSRCS_RELATIVE_FILES += $(wildcard metal/system/freertos/ft_platform/*.c)

	ifdef CONFIG_TARGET_ARMV8_AARCH32
		CSRCS_RELATIVE_FILES += $(wildcard metal/system/freertos/*.c)
	endif

	ifdef CONFIG_TARGET_ARMV8_AARCH64
		CSRCS_RELATIVE_FILES += $(wildcard metal/system/freertos/*.c)
	endif

endif

LIBMETAL_DIR = $(SDK_DIR)/third-party/libmetal

ABSOLUTE_CFILES +=$(wildcard $(LIBMETAL_DIR)/metal/*.c) 


endif #CONFIG_USE_LIBMETAL