ifdef CONFIG_USE_OPENAMP

THIRDP_CUR_DIR := $(FREERTOS_SDK_DIR)/third-party

ifdef CONFIG_USE_FREERTOS

	BUILD_INC_PATH_DIR +=  $(THIRDP_CUR_DIR)/openamp/ports

endif

BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/openamp/lib \
			$(SDK_DIR)/third-party/openamp/lib/include \
			$(SDK_DIR)/third-party/openamp/lib/include/openamp \
			$(SDK_DIR)/third-party/openamp/lib/rpmsg \
			$(SDK_DIR)/third-party/openamp/ports \
			$(SDK_DIR)/third-party/openamp/lib/remoteproc \
			$(SDK_DIR)/third-party/openamp/lib/rpmsg \
			$(SDK_DIR)/third-party/openamp/lib/service/rpmsg/rpc \
			$(SDK_DIR)/third-party/openamp/lib/virtio 
			 

endif #CONFIG_USE_OPENAMP