ifdef CONFIG_USE_MICROPYTHON	

	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party \
                    $(SDK_DIR)/third-party/micropython\
                    $(SDK_DIR)/third-party/micropython/ports \
                    $(SDK_DIR)/third-party/micropython/py \
                    $(SDK_DIR)/third-party/micropython/ports/genhdr \
                    $(SDK_DIR)/third-party/micropython/ports/user \
                    $(SDK_DIR)/third-party/micropython/shared/readline \
                    $(SDK_DIR)/third-party/micropython/shared/runtime \
                    $(SDK_DIR)/third-party/micropython/shared/timeutils \
                    $(SDK_DIR)/third-party/micropython/lib/oofatfs \
                    $(SDK_DIR)/third-party/micropython/lib/libm 

endif
