ifdef CONFIG_ENABLE_FSDMMC
DRIVERS_CSRCS += \
    fsdmmc.c\
    fsdmmc_dma.c\
    fsdmmc_g.c\
    fsdmmc_hw.c\
    fsdmmc_intr.c\
    fsdmmc_sinit.c
endif

ifdef CONFIG_ENABLE_FSDIF
DRIVERS_CSRCS += \
    fsdif.c\
    fsdif_cmd.c\
    fsdif_dma.c\
    fsdif_g.c\
    fsdif_intr.c\
    fsdif_pio.c\
    fsdif_selftest.c\
    fsdif_sinit.c
endif