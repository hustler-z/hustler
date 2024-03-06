/*
 * Copyright : (C) 2024 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: ddma_spim_example.h
 * Date: 2024-01-25 14:53:41
 * LastEditTime: 2024-02-01 17:36:17
 * Description:  This file is for ddma spim example main functions.
 *
 * Modify History:
 *  Ver     Who          Date             Changes
 * -----   ------      --------    --------------------------------------
 *  1.0   liyilun   2024/1/25      first release
 */

#ifndef DDMA_SPIM_EXAMPLE_H
#define DDMA_SPIM_EXAMPLE_H

#include <stdio.h>
#include <string.h>

#include "finterrupt.h"
#include "fdebug.h"
#include "fddma.h"
#include "fddma_hw.h"

#include "fspim_hw.h"
#include "fspim.h"
#include "ferror_code.h"
#include "fparameters.h"
#include "fsleep.h"
#include "fio_mux.h"
#ifdef __cplusplus
extern "C"
{
#endif


/* user-defined parameters */
#define DDMA_CONTROLLER_ID  FDDMA0_ID

#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
#define TX_SLAVE_ID			FDDMA0_SPIM2_TX_SLAVE_ID
#define RX_SLAVE_ID			FDDMA0_SPIM2_RX_SLAVE_ID

#define FSPIM_TEST_ID       2     /*default spim test id*/

#else
#define TX_SLAVE_ID 		FDDMA0_SPIM0_TX_SLAVE_ID
#define RX_SLAVE_ID 		FDDMA0_SPIM0_RX_SLAVE_ID
#define FSPIM_TEST_ID       0    /*default spim test id*/
#endif

#define TX_CHAN_ID			0
#define RX_CHAN_ID			1

#define TRANS_DATA_SIZE		32
#define TX_ON				TRUE
#define RX_ON				TRUE



#define FSPIM_TX_RX_LENGTH          256   /*default spim tx and rx length */

#define FSPIM_TEST_BYTES_LENGTH     32    /*default spim test bytes_length*/
#define FSPIM_DELAY_MS              100   /*timer Delay in milli-seconds */

#define FSPIM_DMA_ENABLE            TRUE
#define FSPIM_DMA_DISABLE           FALSE

/*if bit_16 enable, the transmission data width is 16 bits, otherwise it is 8 bits*/
#define FSPIM_BIT_16_ENABLE         TRUE
#define FSPIM_BIT_16_DISABLE        FALSE

/*spim test mode enable or disable*/
#define SPIM_TEST_MODE_ENABLE       FALSE

enum
{
	FDDMA_SPIM_OK = 0,
	FDDMA_SPIM_INIT_FAILED,
	FDDMA_SPIM_TEST_ABORT,
	FDDMA_SPIM_TEST_FAILED,
};

/* entry function for spi transfer under control of DDMA example */
int FDdmaSpimExample(void);
FError FDdmaTest(void);

static FError FDdmaSpimInit(void);
static FError FDdmaSpimDeinit(void);

FError FDdmaInit(void);
FError FDdmaDeinit(void);

/*init spim function*/
int FSpimOpsInit(u32 spi_id, boolean test_mode, boolean en_dma, boolean bit_16);
/*deinit spim function*/
void FSpimOpsDeInit(void);


#ifdef __cplusplus
}
#endif

#endif /* DDMA_SPIM_EXAMPLE_H */
