BOARD_CSRC += firefly/fboard_init.c \
				firefly/fio_mux.c

ifdef CONFIG_FSL_SDMMC_USE_FSDIF
BOARD_CSRC += firefly/fsdif_timing.c
endif