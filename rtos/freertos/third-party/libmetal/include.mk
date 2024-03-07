ifdef CONFIG_USE_LIBMETAL

THIRDP_CUR_DIR := $(FREERTOS_SDK_DIR)/third-party

ifdef CONFIG_USE_FREERTOS

	BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/libmetal/metal/system/freertos/ft_platform

endif

	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/libmetal \
				$(SDK_DIR)/third-party/libmetal/metal/compiler/gcc 


ifdef CONFIG_TARGET_ARMV8_AARCH32
	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/libmetal/metal/processor/arm 
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/libmetal/metal/processor/aarch64 
endif

endif #CONFIG_USE_LIBMETAL