FSL_SDMMC_OS_DIR := $(FREERTOS_SDK_DIR)/third-party/fsl_sdmmc
FSL_SDMMC_BM_DIR := $(SDK_DIR)/third-party/fsl_sdmmc

ABSOLUTE_CFILES += $(wildcard $(FSL_SDMMC_BM_DIR)/common/*.c) \
				   $(wildcard $(FSL_SDMMC_BM_DIR)/host/*.c) \
				   $(wildcard $(FSL_SDMMC_BM_DIR)/partition/*.c)

ifdef CONFIG_USE_FREERTOS
ABSOLUTE_CFILES += $(wildcard $(FSL_SDMMC_OS_DIR)/osa/*.c)
endif

ifdef CONFIG_FSL_SDMMC_USE_FSDIF
ABSOLUTE_CFILES += $(wildcard $(FSL_SDMMC_BM_DIR)/host/fsdif/*.c)
endif

ifdef CONFIG_FSL_SDMMC_USE_FSDMMC
ABSOLUTE_CFILES += $(wildcard $(FSL_SDMMC_BM_DIR)/host/fsdmmc/*.c)
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SD
ABSOLUTE_CFILES += $(wildcard $(FSL_SDMMC_BM_DIR)/sd/*.c)
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_MMC
ABSOLUTE_CFILES += $(wildcard $(FSL_SDMMC_BM_DIR)/mmc/*.c)
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SDIO
ABSOLUTE_CFILES += $(wildcard $(FSL_SDMMC_BM_DIR)/sdio/*.c)
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SD_SPI
ABSOLUTE_CFILES += $(wildcard $(FSL_SDMMC_BM_DIR)/sdspi/*.c)
endif

