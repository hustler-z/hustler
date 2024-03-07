ifdef CONFIG_USE_OPENAMP

ifdef CONFIG_USE_FREERTOS

	CSRCS_RELATIVE_FILES += $(wildcard ports/*.c)

endif

OPENAMP_C_DIR = $(SDK_DIR)/third-party/openamp

ABSOLUTE_CFILES += $(wildcard $(OPENAMP_C_DIR)/lib/*.c \
						$(OPENAMP_C_DIR)/lib/remoteproc/*.c \
						$(OPENAMP_C_DIR)/lib/rpmsg/*.c \
						$(OPENAMP_C_DIR)/lib/service/rpmsg/rpc/*.c \
						$(OPENAMP_C_DIR)/lib/virtio/*.c )

endif #CONFIG_USE_OPENAMP