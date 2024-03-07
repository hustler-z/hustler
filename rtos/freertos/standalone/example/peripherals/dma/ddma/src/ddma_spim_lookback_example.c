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
 * FilePath: ddma_spim_lookback_example.c
 * Date: 2024-01-25 14:53:41
 * LastEditTime: 2024-02-04 17:36:17
 * Description:  This file is for ddma spim lookback example function implmentation
 *
 * Modify History:
 *  Ver     Who          Date             Changes
 * -----   ------      --------    --------------------------------------
 *  1.0   liyilun   2024/2/4      first release
 */

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "ddma_spim_lookback_example.h"


/* FT-defined print form */
#define FDDMA_SPIM_DEBUG_TAG "FDDMA_SPIM_EXAMPLE"
#define FDDMA_SPIM_ERROR(format, ...) FT_DEBUG_PRINT_E(FDDMA_SPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_SPIM_WARRN(format, ...) FT_DEBUG_PRINT_W(FDDMA_SPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_SPIM_INFO(format, ...) FT_DEBUG_PRINT_I(FDDMA_SPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_SPIM_DEBUG(format, ...) FT_DEBUG_PRINT_D(FDDMA_SPIM_DEBUG_TAG, format, ##__VA_ARGS__)

static FSpim spim_instance;
static FDdma ddma_instance;
static FDdmaConfig ddma_config;
static FSpimConfig spim_config;
static u32 spi_id = FSPIM_TEST_ID;
static FDdmaChanConfig tx_chan_config;
static FDdmaChanConfig rx_chan_config;


static u32 tx_buff[TRANS_DATA_SIZE] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};
static u32 rx_buff[TRANS_DATA_SIZE]	__attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};

static volatile boolean rx_dma_done = FALSE;
static volatile boolean tx_dma_done = FALSE;


static void FDdmaSpimTxDMADone(void)
{
	printf("********tx done********\r\n");
	tx_dma_done = TRUE;
	return;
}

static void FDdmaSpimRxDMADone(void)
{
	printf("********rx done********\r\n");
	rx_dma_done = TRUE;
	return;
}


void FSpimOpsDeInit(void)
{
	FSpim *spim_p = &spim_instance;
	/*interrupt deinit*/
	InterruptMask(spim_p->config.irq_num);
	/*spim deinit*/
	FSpimDeInitialize(spim_p);
	/*deinit iopad*/
	FIOMuxDeInit();
	return;
}

int FSpimOpsInit(u32 spi_id, boolean test_mode, boolean en_dma, boolean bit_16)
{
	FSpim *spim_p = &spim_instance;
    FError ret = FSPIM_SUCCESS;
    /*deinit the spi first*/
    FSpimOpsDeInit();
    if (spim_p->is_ready == FT_COMPONENT_IS_READY)
    {
    	FDDMA_SPIM_ERROR("Spim init error: spim component is ready.");
        return FSPIM_ERR_BUS_BUSY;
    }

    if (spi_id >= FSPI_NUM)
    {
    	FDDMA_SPIM_ERROR("Spim init error: invalid parameter.");
        return FSPIM_ERR_INVAL_PARAM;
    }

    spim_config = *FSpimLookupConfig(spi_id);
    spim_config.slave_dev_id = FSPIM_SLAVE_DEV_0;
    spim_config.cpha = FSPIM_CPHA_2_EDGE;
    spim_config.cpol = FSPIM_CPOL_LOW;
    spim_config.n_bytes = bit_16 ? FSPIM_2_BYTE : FSPIM_1_BYTE;
    spim_config.en_test = test_mode;
    spim_config.en_dma = en_dma;

    ret = FSpimCfgInitialize(spim_p, &spim_config);
    if(ret != FSPIM_SUCCESS)
    {
    	FDDMA_SPIM_ERROR("Spim init error: spim init configuration failed.");
    }

    return ret;

}

FError FDdmaDeinit(void)
{
	FError ret;
	InterruptMask(ddma_instance.config.irq_num);

	FDdmaStop(&ddma_instance);

	ret = FDdmaChanDeactive(&ddma_instance, TX_CHAN_ID);
	ret = FDdmaChanDeconfigure(&ddma_instance, TX_CHAN_ID);
	if (FDDMA_SUCCESS != ret)
	{
	    FDDMA_SPIM_ERROR("TX channel revoke failed: 0x%x", ret);
	    return FDDMA_ERR_IS_USED;
	}

	ret = FDdmaChanDeactive(&ddma_instance, RX_CHAN_ID);
	ret = FDdmaChanDeconfigure(&ddma_instance, RX_CHAN_ID);
	if (FDDMA_SUCCESS != ret)
	{
	   FDDMA_SPIM_ERROR("RX channel revoke failed: 0x%x", ret);
	   return FDDMA_ERR_IS_USED;
	}

	FDdmaDeInitialize(&ddma_instance);

	return FT_SUCCESS;
}

static FError FDdmaSpimDeinit(void)
{
	FError ret=FT_SUCCESS;
	FSpimOpsDeInit();

	ret = FDdmaDeinit();
	if(FDDMA_SUCCESS != ret)
	{
		FDDMA_SPIM_ERROR("Deinit ddma-%d failed, err: 0x%x", DDMA_CONTROLLER_ID, ret);
		return ERR_GENERAL;
	}
	return ret;

}

FError FDdmaInit(void)
{
	FError ret;
	FSpim *spim_p = &spim_instance;
	ddma_config = *FDdmaLookupConfig(DDMA_CONTROLLER_ID);
	ddma_config.irq_prority = IRQ_PRIORITY_VALUE_0;
	ret = FDdmaCfgInitialize(&ddma_instance, &ddma_config);
	if(FDDMA_SUCCESS != ret)
	{
		FDDMA_SPIM_ERROR("Ddma init error: Init ddma-%d configuration failed.", DDMA_CONTROLLER_ID);
		return ret;
	}

	tx_chan_config.slave_id = TX_SLAVE_ID;
	tx_chan_config.ddr_addr = (uintptr)tx_buff;
	tx_chan_config.dev_addr = spim_p->config.base_addr + FSPIM_DR_OFFSET;
	tx_chan_config.req_mode = FDDMA_CHAN_REQ_TX;
	tx_chan_config.timeout = 0xffff;
	tx_chan_config.trans_len = TRANS_DATA_SIZE*4;

	printf("tx channel: %d, slave id: %d, ddr: 0x%x, dev: 0x%x, req mode: %s, trans len: %d \r\n",
	           TX_CHAN_ID,
	           tx_chan_config.slave_id,
	           tx_chan_config.ddr_addr,
	           tx_chan_config.dev_addr,
	           (FDDMA_CHAN_REQ_TX == tx_chan_config.req_mode) ? "mem => dev" : "dev => mem",
	           tx_chan_config.trans_len);

	ret = FDdmaChanConfigure(&ddma_instance, TX_CHAN_ID, &tx_chan_config);
	if (FDDMA_SUCCESS != ret)
	{
		FDDMA_SPIM_ERROR("Ddma init error: tx channel allocate failed: 0x%x.", ret);
		return ret;
	}

	rx_chan_config.slave_id =RX_SLAVE_ID;
	rx_chan_config.ddr_addr = (uintptr)rx_buff;
	rx_chan_config.dev_addr = spim_p->config.base_addr + FSPIM_DR_OFFSET;
	rx_chan_config.req_mode = FDDMA_CHAN_REQ_RX;
	rx_chan_config.timeout = 0xffff;
	rx_chan_config.trans_len = TRANS_DATA_SIZE*4;

	ret = FDdmaChanConfigure(&ddma_instance, RX_CHAN_ID, &rx_chan_config);
	if (FDDMA_SUCCESS != ret)
	{
	   FDDMA_SPIM_ERROR("Ddma init error: rx channel allocate failed: 0x%x.", ret);
	   return ret;
	}

	tx_dma_done = FALSE;
	rx_dma_done = FALSE;


	FDdmaRegisterChanEvtHandler(&ddma_instance, TX_CHAN_ID, FDDMA_CHAN_EVT_REQ_DONE, (void *)FDdmaSpimTxDMADone, NULL);
	FDdmaRegisterChanEvtHandler(&ddma_instance, RX_CHAN_ID, FDDMA_CHAN_EVT_REQ_DONE, (void *)FDdmaSpimRxDMADone, NULL);

	u32 cpu_id = 0;
	GetCpuId(&cpu_id);
	InterruptSetTargetCpus(ddma_instance.config.irq_num, cpu_id);
	InterruptSetPriority(ddma_instance.config.irq_num, ddma_instance.config.irq_prority);
	InterruptInstall(ddma_instance.config.irq_num, FDdmaIrqHandler, &ddma_instance, NULL); /* register intr callback */
	InterruptUmask(ddma_instance.config.irq_num); /* enable ddma0 irq */

	printf("DDMA interrupt setup done! \r\n");
	return FT_SUCCESS;
}
static FError FDdmaSpimInit(void)
{
	FError ret;
	FSpim *spim_p = &spim_instance;

	/* init and set spi */
	ret = FSpimOpsInit(spi_id, SPIM_TEST_MODE_ENABLE, FSPIM_DMA_ENABLE, FSPIM_BIT_16_ENABLE);
	if(ret != FT_SUCCESS)
	{
		FDDMA_SPIM_ERROR("Spim init error, error code %d",ret);
		return ERR_GENERAL;
	}

	/* init and set ddma */
	ret = FDdmaInit();
	if(ret != FT_SUCCESS)
	{
		FDDMA_SPIM_ERROR("Ddma init error, error code %d",ret);
		return ERR_GENERAL;
	}
	return FT_SUCCESS;
}

FError FDdmaTest(void)
{
    FError ret = FT_SUCCESS;
    int timeout = 10000;
    if (FDdmaIsChanRunning(ddma_instance.config.base_addr, TX_CHAN_ID) ||
        FDdmaIsChanRunning(ddma_instance.config.base_addr, RX_CHAN_ID))
    {
        FDDMA_SPIM_ERROR("RX or TX chan is already running!");
        return FDDMA_ERR_IS_USED;
    }
    /* enable tx channel and ready for use */
    FDdmaChanActive(&ddma_instance, TX_CHAN_ID);
    /* enable rx channel and ready for use */
    FDdmaChanActive(&ddma_instance, RX_CHAN_ID);

    FDdmaStart(&ddma_instance);

    //FSpimCsOnOff(&spim_instance, TRUE);

    printf("\nStart transfer and receive data.\n");
    ret = FSpimTransferDMA(&spim_instance, TX_ON, RX_ON);
    if(ret != FDDMA_SPIM_OK)
    {
    	FDDMA_SPIM_ERROR("transfer failed, test abort");
    	return FDDMA_SPIM_TEST_ABORT;
    }


    /* wait in interrupt mode until tx and rx all finished or timeout */
    while (FALSE == tx_dma_done)
    {
        if (--timeout <= 0)
        {
            break;
        }

        fsleep_millisec(1);
    }
    if (0 >= timeout)
    {
        FDDMA_SPIM_ERROR("Wait DDMA TX end timeout %d!", timeout);
        FDdmaDumpRegisters(ddma_instance.config.base_addr);
        FDdmaDumpChanRegisters(ddma_instance.config.base_addr, TX_CHAN_ID);
        FDdmaDumpChanRegisters(ddma_instance.config.base_addr, RX_CHAN_ID);
        return FDDMA_ERR_WAIT_TIMEOUT;
    }

    //FSpimCsOnOff(&spim_instance, FALSE);

    while (FALSE == rx_dma_done)
    {
        fsleep_millisec(1);
    }

    return FT_SUCCESS;
}

int FDdmaSpimExample(void)
{
	FError ret=0;
	u32 loop;
	/* spi and ddma hardware init */
	ret = FDdmaSpimInit();
	if(ret != FT_SUCCESS)
	{
		FDDMA_SPIM_ERROR("Hardware init failed.");
	}else
	{
		/* fill tx data */
		for(loop = 0; loop < TRANS_DATA_SIZE; loop++)
		{
			tx_buff[loop] = loop + 10;
			rx_buff[loop] = 0;
		}
		printf("\nTx data:\n");
		for (loop = 0; loop < TRANS_DATA_SIZE; loop++) {
			printf("%d\t", tx_buff[loop]);
		}

		/* start transfer data... */
		ret = FDdmaTest();

		if(ret != FT_SUCCESS)
		{
			FDDMA_SPIM_ERROR("Ddma test failed.");
		}
		else
		{
			for (loop = 0; loop < TRANS_DATA_SIZE; loop++) {
				if (tx_buff[loop] != rx_buff[loop]) {
					printf("\n%s@%d:Ddma spim_lookback example [failure].\r\n\r\n",__func__,__LINE__);
					return 1;
				}
			}
			printf("Rx data:\n");
			for (loop = 0; loop < TRANS_DATA_SIZE; loop++) {

				f_printk("%d\t", rx_buff[loop]);
				//u32 value =rx_buff[loop];

			}
			printf("\n%s@%d:Ddma spim_lookback example [success].\r\n\r\n",__func__,__LINE__);
			
		}
	}
	ret = FDdmaSpimDeinit();
	if(ret != FT_SUCCESS)
	{
		printf("\n%s@%d:Ddma spim_lookback example [failure].\r\n\r\n",__func__,__LINE__);
		return 1;
	}
	return 0;

}
