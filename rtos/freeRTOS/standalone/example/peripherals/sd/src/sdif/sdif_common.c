#include "sdif_common.h"

uint8_t s_dma_buffer[SZ_1M * 4] = {0};
u8 rw_buf[SD_MAX_RW_BLK * SD_BLOCK_SIZE] __attribute((aligned(SD_BLOCK_SIZE))) = {0};