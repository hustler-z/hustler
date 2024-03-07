BOARD_CSRC += rk3566_rz3w/fboard_init.c \
			  rk3566_rz3w/fio_mux.c

ifdef CONFIG_FSL_SDMMC_USE_FSDIF
BOARD_CSRC += rk3566_rz3w/fsdif_timing.c
endif
