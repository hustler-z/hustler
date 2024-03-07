/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sdmmc_osa.h"
#include "fsleep.h"
#include "fmemory_pool.h"

/*******************************************************************************
 * Definitons
 ******************************************************************************/
#define SDMMC_MEM_BUF_SIZE      SZ_1M
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t sdmmc_buf[SDMMC_MEM_BUF_SIZE] __attribute__((aligned(4096))) = {0};
static FMemp sdmmc_mem_pool;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * brief Initialize OSA.
 */
void SDMMC_OSAInit(void)
{
    memset(&sdmmc_mem_pool, 0, sizeof(sdmmc_mem_pool));

    FError result = FMempInit(&sdmmc_mem_pool, sdmmc_buf, sdmmc_buf + SDMMC_MEM_BUF_SIZE); /* init memory pool */
    assert(result == FMEMP_SUCCESS); 
}

/*!
 * brief De-Initialize OSA.
 */
void SDMMC_OSADeInit(void)
{
    assert(FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready);
    FMempDeinit(&sdmmc_mem_pool);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SDMMC_OSAMemoryAllocate
 * Description   : Reserves the requested amount of memory in bytes.
 *
 *END**************************************************************************/
void *SDMMC_OSAMemoryAllocate(uint32_t length)
{
    assert(FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready);
    void *p = FMempMallocAlign(&sdmmc_mem_pool, length, sizeof(uintptr_t));
    return p;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SDMMC_OSAMemoryAllocate
 * Description   : Reserves the requested amount of memory in bytes.
 *
 *END**************************************************************************/
void *SDMMC_OSAMemoryAlignedAllocate(uint32_t length, uint32_t align)
{
    assert(FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready);
    void *p = FMempMallocAlign(&sdmmc_mem_pool, length, align);
    return p;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SDMMC_OSAMemoryFree
 * Description   : Frees the memory previously reserved.
 *
 *END**************************************************************************/
void SDMMC_OSAMemoryFree(void *p)
{
    assert(FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready);
    FMempFree(&sdmmc_mem_pool, p);
}

/*!
 * brief OSA Create event.
 * param event handle.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventCreate(void *eventHandle)
{
    assert(eventHandle != NULL);

#if defined(SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE) && SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE
    (void)OSA_SemaphoreCreate(&(((sdmmc_osa_event_t *)eventHandle)->handle), 0U);
#else
    (void)OSA_EventCreate(&(((sdmmc_osa_event_t *)eventHandle)->handle), true);
#endif

    return kStatus_Success;
}

/*!
 * brief Wait event.
 *
 * param eventHandle The event type
 * param eventType Timeout time in milliseconds.
 * param timeoutMilliseconds timeout value in ms.
 * param event event flags.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventWait(void *eventHandle, uint32_t eventType, uint32_t timeoutMilliseconds, uint32_t *event)
{
    assert(eventHandle != NULL);

    osa_status_t status = KOSA_StatusError;

#if defined(SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE) && SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE
    while (true)
    {
        status = OSA_SemaphoreWait(&(((sdmmc_osa_event_t *)eventHandle)->handle), timeoutMilliseconds);
        if (KOSA_StatusTimeout == status)
        {
            break;
        }

        if (KOSA_StatusSuccess == status)
        {
            (void)SDMMC_OSAEventGet(eventHandle, eventType, event);
            if ((*event & eventType) != 0U)
            {
                return kStatus_Success;
            }
        }
    }

#else
    while (true)
    {
        status = OSA_EventWait(&(((sdmmc_osa_event_t *)eventHandle)->handle), eventType, 0, timeoutMilliseconds, event);
        if ((KOSA_StatusSuccess == status) || (KOSA_StatusTimeout == status))
        {
            break;
        }
    }

    if (KOSA_StatusSuccess == status)
    {
        return kStatus_Success;
    }
#endif

    return kStatus_Fail;
}

/*!
 * brief set event.
 * param event event handle.
 * param eventType The event type
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventSet(void *eventHandle, uint32_t eventType)
{
    assert(eventHandle != NULL);

#if defined(SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE) && SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE
    OSA_SR_ALLOC();
    OSA_ENTER_CRITICAL();
    ((sdmmc_osa_event_t *)eventHandle)->eventFlag |= eventType;
    OSA_EXIT_CRITICAL();

    (void)OSA_SemaphorePost(&(((sdmmc_osa_event_t *)eventHandle)->handle));
#else
    (void)OSA_EventSet(&(((sdmmc_osa_event_t *)eventHandle)->handle), eventType);
#endif

    return kStatus_Success;
}

/*!
 * brief Get event flag.
 * param eventHandle event handle.
 * param eventType The event type
 * param flag pointer to store event value.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventGet(void *eventHandle, uint32_t eventType, uint32_t *flag)
{
    assert(eventHandle != NULL);
    assert(flag != NULL);

#if defined(SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE) && SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE
    *flag = ((sdmmc_osa_event_t *)eventHandle)->eventFlag;
#else
    (void)OSA_EventGet(&(((sdmmc_osa_event_t *)eventHandle)->handle), eventType, flag);
#endif

    return kStatus_Success;
}

/*!
 * brief clear event flag.
 * param eventHandle event handle.
 * param eventType The event type
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventClear(void *eventHandle, uint32_t eventType)
{
    assert(eventHandle != NULL);

#if defined(SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE) && SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE
    OSA_SR_ALLOC();
    OSA_ENTER_CRITICAL();
    ((sdmmc_osa_event_t *)eventHandle)->eventFlag &= ~eventType;
    OSA_EXIT_CRITICAL();
#else
    (void)OSA_EventClear(&(((sdmmc_osa_event_t *)eventHandle)->handle), eventType);
#endif

    return kStatus_Success;
}

/*!
 * brief Delete event.
 * param event The event handle.
 */
status_t SDMMC_OSAEventDestroy(void *eventHandle)
{
    assert(eventHandle != NULL);

#if defined(SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE) && SDMMC_OSA_POLLING_EVENT_BY_SEMPHORE
    (void)OSA_SemaphoreDestroy(&(((sdmmc_osa_event_t *)eventHandle)->handle));
#else
    (void)OSA_EventDestroy(&(((sdmmc_osa_event_t *)eventHandle)->handle));
#endif

    return kStatus_Success;
}

/*!
 * brief Create a mutex.
 * param mutexHandle mutex handle.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAMutexCreate(void *mutexHandle)
{
    assert(mutexHandle != NULL);

    (void)OSA_MutexCreate(&((sdmmc_osa_mutex_t *)mutexHandle)->handle);

    return kStatus_Success;
}

/*!
 * brief set event.
 * param mutexHandle mutex handle.
 * param millisec The maximum number of milliseconds to wait for the mutex.
 *                 If the mutex is locked, Pass the value osaWaitForever_c will
 *                 wait indefinitely, pass 0 will return KOSA_StatusTimeout
 *                 immediately.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAMutexLock(void *mutexHandle, uint32_t millisec)
{
    assert(mutexHandle != NULL);

    (void)OSA_MutexLock(&((sdmmc_osa_mutex_t *)mutexHandle)->handle, millisec);

    return kStatus_Success;
}

/*!
 * brief Get event flag.
 * param mutexHandle mutex handle.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAMutexUnlock(void *mutexHandle)
{
    assert(mutexHandle != NULL);

    (void)OSA_MutexUnlock(&((sdmmc_osa_mutex_t *)mutexHandle)->handle);

    return kStatus_Success;
}

/*!
 * brief Delete mutex.
 * param mutexHandle The mutex handle.
 */
status_t SDMMC_OSAMutexDestroy(void *mutexHandle)
{
    assert(mutexHandle != NULL);

    (void)OSA_MutexDestroy(&((sdmmc_osa_mutex_t *)mutexHandle)->handle);

    return kStatus_Success;
}

/*!
 * brief sdmmc delay.
 * param milliseconds time to delay
 */
void SDMMC_OSADelay(uint32_t milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

/*!
 * brief sdmmc delay us.
 * param microseconds time to delay
 * return actual delayed microseconds
 */
uint32_t SDMMC_OSADelayUs(uint32_t microseconds)
{
    vTaskDelay(10U);
    //fsleep_microsec(microseconds);
    return 0;
}
