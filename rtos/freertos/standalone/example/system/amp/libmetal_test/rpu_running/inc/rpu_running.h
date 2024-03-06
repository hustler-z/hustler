/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * FilePath: rpu_running.h
 * Date: 2023-03-06 15:07:50
 * LastEditTime: 2023-03-06 15:07:50
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who           Date        Changes
 * ----- ------         --------    --------------------------------------
 * 1.0  liushengming    2023/03/06  init
 */

#ifndef RPU_RUNNING_H
#define RPU_RUNNING_H

#ifdef __cplusplus
extern "C"
{
#endif
#define CONFIG_TARGET_CPU_MASK (1 << CONFIG_TARGET_CPU_ID)
/**
 * @brief   atomic_shmem_demod() - Shared memory atomic operation demo
 *          This task will:
 *          * Wait for the remote to write to shared memory.
 *          * Once it receives the notification via polling, start atomic add by
 *	      1 for 1000 times to first 32 bits of memory in the
 *	      shared memory location at 3ed00000 which is pointed to by shm_io.
 *          * Write to shared mem to notify the remote once it finishes
 *            calculation.
 *
 * @return - If setup failed, return the corresponding error number. Otherwise
 *          return 0 on success.
 */
int atomic_shmem_demod(void);

/**
 * @brief ipi_latency_demod() - Show performance of  IPI with Libmetal.
 *        Loop until APU tells RPU to stop via shared memory.
 *        In loop, wait for interrupt (interrupt handler stops APU to
 *        RPU timer). Then reset count on RPU to APU timer to 0, start
 *        counting and send interrupt to notify APU.
 *
 * @return - 0 on success, error code if failure.
 */
int ipi_latency_demod(void);

/**
 * @brief   ipi_shmem_demod() - shared memory IPI demo
 *          This task will:
 *          * Wait for IPI interrupt from the remote
 *          * Once it received the interrupt, copy the content from
 *            the ping buffer to the pong buffer.
 *          * Update the shared memory descriptor for the new available
 *            pong buffer.
 *          * Trigger IPI to notifty the remote.
 *
 * @return - 0 on success, error code if failure.
 */
int ipi_shmem_demod(void);

/**
 * @brief shmem_demod() - Show use of shared memory with Libmetal.
 *        Until KEEP_GOING signal is stopped, keep looping.
 *        In the loop, read message from remote, add one to message and
 *        then respond. After the loop, cleanup resources.
 *
 * @return - return 0 on success, otherwise return error number indicating
 *           type of error
 */
int shmem_demod(void);

/**
 * @brief shmem_latency_demod() - Show performance of shared mem.
 *        Loop until APU tells RPU to stop via shared memory.
 *        In loop, wait for interrupt (interrupt handler stops APU to
 *        RPU timer). Then reset count on RPU to APU timer to 0, start
 *        counting and send interrupt to notify APU.
 *
 * @return - 0 on success, error code if failure.
 */
int shmem_latency_demod(void);

/**
 * @brief shmem_throughput_demod() - Show throughput of shared mem.
 *        At signal of remote, record total time to do block read and write
 *        operation Loop until APU tells RPU to stop via shared memory.
 *        In loop, wait for interrupt (interrupt handler stops APU to
 *        RPU timer). Then reset count on RPU to APU timer to 0, start
 *        counting and send interrupt to notify APU.
 *
 * @return - 0 on success, error code if failure.
 */
int shmem_throughput_demod(void);

/**
 * @brief ipi_waking_up_demod() - wait signal waking up.
 *        At signal of remote, start running and send interrupt to notify APU.
 *
 * @return - 0 on success, error code if failure.
 */
int ipi_waking_up_demod(void);

#ifdef __cplusplus
}
#endif

#endif