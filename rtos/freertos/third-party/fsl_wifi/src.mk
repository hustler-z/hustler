FSL_WIFI_OS_DIR := $(FREERTOS_SDK_ROOT)/third-party/fsl_wifi

ABSOLUTE_CFILES += $(wildcard $(FSL_WIFI_OS_DIR)/cli/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/dhcpd/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/nw_utils/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/port/lwip/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/port/lwip/hooks/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/port/os/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/port/sdio/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/port/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/wifidriver/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/wifidriver/wpa_supp_if/*.c) \
				   $(wildcard $(FSL_WIFI_OS_DIR)/wlcmgr/*.c)


