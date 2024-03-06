
BOARD_CSRC += e2000q_demo/fboard_init.c \
			  e2000q_demo/fio_mux.c

ifdef CONFIG_FSL_SDMMC_USE_FSDIF
BOARD_CSRC += e2000q_demo/fsdif_timing.c
endif