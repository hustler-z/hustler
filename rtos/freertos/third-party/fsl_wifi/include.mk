ifdef CONFIG_USE_FSL_WIFI
FSL_WIFI_OS_DIR := $(FREERTOS_SDK_DIR)/third-party/fsl_wifi

BUILD_INC_PATH_DIR += $(FSL_WIFI_OS_DIR)/certs \
					  $(FSL_WIFI_OS_DIR)/cli \
					  $(FSL_WIFI_OS_DIR)/dpcpd \
					  $(FSL_WIFI_OS_DIR)/incl \
					  $(FSL_WIFI_OS_DIR)/incl/port \
					  $(FSL_WIFI_OS_DIR)/incl/port/lwip \
					  $(FSL_WIFI_OS_DIR)/incl/port/os \
					  $(FSL_WIFI_OS_DIR)/incl/port/sdio \
					  $(FSL_WIFI_OS_DIR)/incl/wifidriver \
					  $(FSL_WIFI_OS_DIR)/incl/wlcmgr \
					  $(FSL_WIFI_OS_DIR)/wifi_bt_firmware \
					  $(FSL_WIFI_OS_DIR)/wifidriver \
					  $(FSL_WIFI_OS_DIR)/wifidriver/incl \
					  $(FSL_WIFI_OS_DIR)/wifidriver/wpa_supp_if \
					  $(FSL_WIFI_OS_DIR)/wifidriver/wpa_supp_if/incl
	
endif
