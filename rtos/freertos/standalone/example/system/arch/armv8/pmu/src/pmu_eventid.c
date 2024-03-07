/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: pmu_example.c
 * Created Date: 2023-11-01 19:49:05
 * Last Modified: 2024-03-01 11:00:36
 * Description:  This file is for event id test
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0    huanghe     2023-11-10        first version
 */
#include <stdio.h>
#include "fpmu_perf.h"
#include "fdebug.h"
#include "fparameters.h"
#include "fcpu_info.h"
#include "ftypes.h"
#include "finterrupt.h"

static FPmu instance ;
static volatile u32 irq_flg = 0 ; 

static  int is_event_supported(u64 bitmap, u32 event_id) 
{
    return (bitmap & (1ULL << event_id)) != 0;
}


static void FPmuEventDemoCB(void *args)
{
    fsize_t counter_id = (fsize_t )args ;

    printf("irq is here counter_id:%d : event_id:0x%x \r\n",counter_id,FPmuCheckCounterEventId(&instance,counter_id)) ;
    /* stop counter */
    FPmuCounterDisable(&instance,counter_id) ;
    irq_flg ++ ;
}


void FPmuEventIdExample(void) 
{
    FError error;
    u64 valueAfter;
    u32 timeout_cnt = 0;
    u32 event_id = FPMU_SW_INCR_ID; /* Replace with your specific event ID */
    fsize_t counter_id = 0; /* Replace with your specific counter ID */
    
    /* Initialize the PMU */
    error = FPmuCfgInitialize(&instance);
    if (error != FPMU_SUCCESS) {
        printf("[failure] to initialize PMU: error %d\n", error);
        return; /* Early return on failure */
    }

    /* Output the number of supported counters */
    printf("PMU initialized. Supported counter num: %u\n", instance.counter_num);
    FPmuFeatureProbeDebugPrint(&instance) ;

    InterruptSetPriority(FPMU_IRQ_NUM, 0); /* Setup interrupt */
    InterruptInstall(FPMU_IRQ_NUM,
                     FPmuIrqHandler,
                     &instance,
                     NULL); /* Register interrupt handler */
    InterruptUmask(FPMU_IRQ_NUM);



    /* Check if the event is supported before proceeding */
    if (is_event_supported(instance.event_id_bitmap, event_id) ||
        is_event_supported(instance.event_ext_id_bitmap, event_id)) 
    {

        /* Configure and enable the counter with the event ID. */
        error = FPmuCounterConfig(&instance, counter_id, event_id, FPmuEventDemoCB, (void *)counter_id);
        if (error != FPMU_SUCCESS) {
            printf("[failure] to configure counter %u with event ID %u: error %d\n", counter_id, event_id, error);
            return; /* Early return on failure */
        }

        /* Enable the counter */
        error = FPmuCounterEnable(&instance, counter_id);
        if (error != FPMU_SUCCESS) {
            printf("[failure] to enable counter %u with event ID %u: error %d\n", counter_id, event_id, error);
            return; /* Early return on failure */
        }

        /* Write period to counter, and wait for interrupt */

        error = FPmuWriteCounter(&instance, counter_id, FPMU_PERIOD_CALC_32BIT(10));
        if (error != FPMU_SUCCESS) {
            printf("[failure] to write counter %u: error %d\n", counter_id, error);
            return; /* Early return on failure */
        }

        /* Read the counter before the interrupt */
        error = FPmuReadCounter(&instance, counter_id, &valueAfter);
        if (error != FPMU_SUCCESS) {
            printf("[failure] to read counter %u after interrupt: error %d\n", counter_id, error);
            return; /* Early return on failure */
        }



        FPmuStart() ;

        /* Increase the counter in a loop */
        for (size_t i = 0; i < 11; i++) {
            error = FPmuDebugCounterIncrease(&instance, counter_id);
            if (error != FPMU_SUCCESS) {
                printf("[failure] to increase counter %u: error %d\n", counter_id, error);
                return; /* Early return on failure */
            }
        }
        
        /* Wait for the interrupt flag to be set */
        while (irq_flg == 0)
        {
            if(timeout_cnt ++ >= 0xffffffff)
            {
                printf("wait irq is timeout \r\n") ;
                break; 
            }
        }

        /* Read the counter after the interrupt */
        error = FPmuReadCounter(&instance, counter_id, &valueAfter);
        if (error != FPMU_SUCCESS) {
            printf("[failure] to read counter %u after interrupt: error %d\n", counter_id, error);
            return; /* Early return on failure */
        }

        /* The value after should reflect the increments */
        printf("Counter %u incremented [success]. Final value: %llx\n", counter_id, valueAfter);
    } else {
        printf("[failure]: Event ID %u is not supported.\n", event_id);
    }
}