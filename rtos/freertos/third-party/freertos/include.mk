
ifdef CONFIG_USE_FREERTOS

THIRDP_CUR_DIR := $(FREERTOS_SDK_DIR)/third-party

# src files
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/include 
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/portable/GCC/ft_platform 

ifdef CONFIG_FREERTOS_USE_POSIX
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/posix/include
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/posix/FreeRTOS-Plus-POSIX/include
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/posix/FreeRTOS-Plus-POSIX/include/portable
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/posix/FreeRTOS-Plus-POSIX/include/portable/phytium
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/posix/include/FreeRTOS_POSIX
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/posix/include/FreeRTOS_POSIX/sys
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/posix/include/private
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch64
endif #CONFIG_TARGET_ARMV8_AARCH64

ifdef CONFIG_TARGET_ARMV8_AARCH32
	BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch32
endif

endif