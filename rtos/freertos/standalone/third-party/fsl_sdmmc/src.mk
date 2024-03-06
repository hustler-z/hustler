CSRCS_RELATIVE_FILES += $(wildcard common/*.c) \
						$(wildcard host/*.c) \
						$(wildcard partition/*.c)

ifdef CONFIG_USE_BAREMETAL
CSRCS_RELATIVE_FILES += $(wildcard osa/*.c)
endif

ifdef CONFIG_FSL_SDMMC_USE_FSDIF
CSRCS_RELATIVE_FILES += $(wildcard host/fsdif/*.c)
endif

ifdef CONFIG_FSL_SDMMC_USE_FSDMMC
CSRCS_RELATIVE_FILES += $(wildcard host/fsdmmc/*.c)
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SD
CSRCS_RELATIVE_FILES += $(wildcard sd/*.c)
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_MMC
CSRCS_RELATIVE_FILES += $(wildcard mmc/*.c)
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SDIO
CSRCS_RELATIVE_FILES += $(wildcard sdio/*.c)
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SD_SPI
CSRCS_RELATIVE_FILES += $(wildcard sdspi/*.c)
endif