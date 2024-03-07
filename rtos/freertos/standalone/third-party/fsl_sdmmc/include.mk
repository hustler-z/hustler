BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fsl_sdmmc/common \
					  $(SDK_DIR)/third-party/fsl_sdmmc/host \
					  $(SDK_DIR)/third-party/fsl_sdmmc/partition

ifdef CONFIG_USE_BAREMETAL
BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fsl_sdmmc/osa
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SD
BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fsl_sdmmc/sd
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_MMC
BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fsl_sdmmc/mmc
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SDIO
BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fsl_sdmmc/sdio
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SD_SPI
BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fsl_sdmmc/sdspi
endif