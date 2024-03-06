/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SDMMC_HOST_H
#define _FSL_SDMMC_HOST_H

#include "fsl_sdmmc_common.h"
#include "fsl_sdmmc_osa.h"

/*!
 * @addtogroup sdmmchost_usdhc
 * @ingroup sdmmchost
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Middleware adapter version. */
#define FSL_SDMMC_HOST_ADAPTER_VERSION (MAKE_VERSION(2U, 6U, 3U)) /*2.6.3*/

#if ((defined __DCACHE_PRESENT) && __DCACHE_PRESENT) || (defined FSL_FEATURE_HAS_L1CACHE && FSL_FEATURE_HAS_L1CACHE)
#define SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER 0
#if SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
#if !(defined FSL_USDHC_ENABLE_SCATTER_GATHER_TRANSFER && FSL_USDHC_ENABLE_SCATTER_GATHER_TRANSFER)
#error "missing definition of macro FSL_USDHC_ENABLE_SCATTER_GATHER_TRANSFER"
#endif
#endif
#endif

/*! @brief sdmmc host capability */
enum
{
    kSDMMCHOST_SupportHighSpeed         = 1U << 0U,  /*!< high speed capability */
    kSDMMCHOST_SupportSuspendResume     = 1U << 1U,  /*!< suspend resume capability */
    kSDMMCHOST_SupportVoltage3v3        = 1U << 2U,  /*!< 3V3 capability */
    kSDMMCHOST_SupportVoltage3v0        = 1U << 3U,  /*!< 3V0 capability */
    kSDMMCHOST_SupportVoltage1v8        = 1U << 4U,  /*!< 1V8 capability */
    kSDMMCHOST_SupportVoltage1v2        = 1U << 5U,  /*!< 1V2 capability */
    kSDMMCHOST_Support4BitDataWidth     = 1U << 6U,  /*!< 4 bit data width capability */
    kSDMMCHOST_Support8BitDataWidth     = 1U << 7U,  /*!< 8 bit data width capability */
    kSDMMCHOST_SupportDDRMode           = 1U << 8U,  /*!< DDR mode capability */
    kSDMMCHOST_SupportDetectCardByData3 = 1U << 9U,  /*!< data3 detect card capability */
    kSDMMCHOST_SupportDetectCardByCD    = 1U << 10U, /*!< CD detect card capability */
    kSDMMCHOST_SupportAutoCmd12         = 1U << 11U, /*!< auto command 12 capability */
    kSDMMCHOST_SupportSDR104            = 1U << 12U, /*!< SDR104 capability */
    kSDMMCHOST_SupportSDR50             = 1U << 13U, /*!< SDR50 capability */
    kSDMMCHOST_SupportHS200             = 1U << 14U, /*!< HS200 capability */
    kSDMMCHOST_SupportHS400             = 1U << 15U, /*!< HS400 capability */
    kSDMMCHOST_SupportDriverTypeC       = 1U << 16U, /*!< Driver Type C capability */
    kSDMMCHOST_SupportSetCurrent        = 1U << 17U, /*!< Set current limit capability */
};

/*!@brief SDMMC host dma descriptor buffer address align size */
#define SDMMCHOST_DMA_DESCRIPTOR_BUFFER_ALIGN_SIZE (4U)

#define SDHC_BLKATTR_BLKCNT_MASK                 (0xFFFF0000U)
#define SDHC_BLKATTR_BLKCNT_SHIFT                (16U)
/*! BLKCNT - Blocks Count For Current Transfer
 *  0b0000000000000000..Stop count.
 *  0b0000000000000001..1 block
 *  0b0000000000000010..2 blocks
 *  0b1111111111111111..65535 blocks
 */

/*! @brief Maximum block count can be set one time */
#define SDMMCHOST_SUPPORT_MAX_BLOCK_LENGTH     (4096U)
#define SDMMCHOST_SUPPORT_MAX_BLOCK_COUNT      (SDHC_BLKATTR_BLKCNT_MASK >> SDHC_BLKATTR_BLKCNT_SHIFT)

/*! @brief SDMMC host reset timoue value */
#define SDMMCHOST_RESET_TIMEOUT_VALUE (1000000U)

/*!@brief sdmmc host transfer function */
/*! @brief The command type */
typedef enum sdmmc_card_command_type_t
{
    kCARD_CommandTypeNormal  = 0U, /*!< Normal command */
    kCARD_CommandTypeSuspend = 1U, /*!< Suspend command */
    kCARD_CommandTypeResume  = 2U, /*!< Resume command */
    kCARD_CommandTypeAbort   = 3U, /*!< Abort command */
    kCARD_CommandTypeEmpty   = 4U, /*!< Empty command */
} sdmmc_card_command_type_t;

/*!
 * @brief The command response type.
 *
 * Defines the command response type from card to host controller.
 */
typedef enum sdmmc_card_response_type_t
{
    kCARD_ResponseTypeNone = 0U, /*!< Response type: none */
    kCARD_ResponseTypeR1   = 1U, /*!< Response type: R1 */
    kCARD_ResponseTypeR1b  = 2U, /*!< Response type: R1b */
    kCARD_ResponseTypeR2   = 3U, /*!< Response type: R2 */
    kCARD_ResponseTypeR3   = 4U, /*!< Response type: R3 */
    kCARD_ResponseTypeR4   = 5U, /*!< Response type: R4 */
    kCARD_ResponseTypeR5   = 6U, /*!< Response type: R5 */
    kCARD_ResponseTypeR5b  = 7U, /*!< Response type: R5b */
    kCARD_ResponseTypeR6   = 8U, /*!< Response type: R6 */
    kCARD_ResponseTypeR7   = 9U, /*!< Response type: R7 */
} sdmmc_card_response_type_t;

/*! @brief MMC card boot mode */
typedef enum sdmmc_boot_mode_t
{
    kUSDHC_BootModeNormal      = 0U, /*!< Normal boot */
    kUSDHC_BootModeAlternative = 1U, /*!< Alternative boot */
} sdmmc_boot_mode_t;

/*!
 * @brief Card data descriptor.
 *
 * Defines a structure to contain data-related attribute. The 'enableIgnoreError' is used when upper card
 * driver wants to ignore the error event to read/write all the data and not to stop read/write immediately when an
 * error event happens. For example, bus testing procedure for MMC card.
 */
typedef struct sdmmchost_data_t
{
    bool streamTransfer;      /*!< indicate this is a stream data transfer command */

    bool enableAutoCommand12; /*!< Enable auto CMD12. */
    bool enableAutoCommand23; /*!< Enable auto CMD23. */
    bool enableIgnoreError;   /*!< Enable to ignore error event to read/write all the data. */
    uint8_t dataType;         /*!< this is used to distinguish the normal/tuning/boot data. */
    size_t blockSize;         /*!< Block size. */
    uint32_t blockCount;      /*!< Block count. */
    uint32_t *rxData;         /*!< Buffer to save data read. */
    const uint32_t *txData;   /*!< Data buffer to write. */
} sdmmchost_data_t;

/*!
 * @brief Card command descriptor.
 *
 * Defines card command-related attribute.
 */
typedef struct sdmmchost_cmd_t
{
    uint32_t index;                          /*!< Command index. */
    uint32_t argument;                       /*!< Command argument. */
    sdmmc_card_command_type_t type;          /*!< Command type. */
    sdmmc_card_response_type_t responseType; /*!< Command response type. */
    uint32_t response[4U];                   /*!< Response for this command. */
    uint32_t responseErrorFlags;             /*!< Response error flag, which need to check
                                                 the command reponse. */
    uint32_t flags;                          /*!< Cmd flags. */
} sdmmchost_cmd_t;

/*! @brief Transfer state. */
typedef struct sdmmchost_transfer_t 
{
    sdmmchost_data_t *data;   /*!< Data to transfer. */
    sdmmchost_cmd_t *command; /*!< Command to send. */    
} sdmmchost_transfer_t;

typedef struct sdmmchost_boot_config_t 
{
    uint32_t ackTimeoutCount;      /*!< Timeout value for the boot ACK. The available range is 0 ~ 15. */
    sdmmc_boot_mode_t bootMode;    /*!< Boot mode selection. */
    uint32_t blockCount;           /*!< Stop at block gap value of automatic mode. Available range is 0 ~ 65535. */
    size_t blockSize;              /*!< Block size. */
    bool enableBootAck;            /*!< Enable or disable boot ACK. */
    bool enableAutoStopAtBlockGap; /*!< Enable or disable auto stop at block gap function in boot period. */    
} sdmmchost_boot_config_t;

/*! @brief host Endian mode
 * corresponding to driver define
 * @anchor _sdmmchost_endian_mode
 */
enum
{
    kSDMMCHOST_EndianModeBig         = 0U, /*!< Big endian mode */
    kSDMMCHOST_EndianModeHalfWordBig = 1U, /*!< Half word big endian mode */
    kSDMMCHOST_EndianModeLittle      = 2U, /*!< Little endian mode */
};

/*! @brief sdmmc host tuning type
 * @anchor _sdmmchost_tuning_type
 */
enum
{
    kSDMMCHOST_StandardTuning = 0U, /*!< standard tuning type */
    kSDMMCHOST_ManualTuning   = 1U, /*!< manual tuning type */
};

/*! @brief sdmmc host maintain cache flag
 * @anchor _sdmmc_host_cache_control
 */
enum
{
    kSDMMCHOST_NoCacheControl       = 0U, /*!< sdmmc host cache control disabled */
    kSDMMCHOST_CacheControlRWBuffer = 1U, /*!< sdmmc host cache control read/write buffer */
};

typedef struct _sdmmchost_ sdmmchost_t;

/*!@brief sdmmc host operations  */
typedef struct _sdmmchost_ops
{
    /* sdmmc host operations */
    void (*deinit)(sdmmchost_t *host);
    status_t (*reset)(sdmmchost_t *host);

    /* set sdmmc host mode and get host status */
    void (*switchToVoltage)(sdmmchost_t *host, uint32_t voltage);
    status_t (*executeTuning)(sdmmchost_t *host,
                              uint32_t tuningCmd,
                              uint32_t *revBuf,
                              uint32_t blockSize);
    void (*enableDDRMode)(sdmmchost_t *host, bool enable, 
                          uint32_t nibblePos);
    void (*enableHS400Mode)(sdmmchost_t *host, bool enable);
    void (*enableStrobeDll)(sdmmchost_t *host, bool enable);
    uint32_t (*getSignalLineStatus)(sdmmchost_t *host, uint32_t signalLine);
    void (*convertDataToLittleEndian)(sdmmchost_t *host, uint32_t *data, uint32_t wordSize, uint32_t format);

    /* card related functions */
    status_t (*cardDetectInit)(sdmmchost_t *host, void *cd);
    void (*cardSetPower)(sdmmchost_t *host, bool enable);
    void (*cardForceClockOn)(sdmmchost_t *host, bool enable);
    void (*cardEnableInt)(sdmmchost_t *host, bool enable);
    status_t (*cardIntInit)(sdmmchost_t *host, void *sdioInt);
    void (*cardSetBusWidth)(sdmmchost_t *host, uint32_t dataBusWidth);
    status_t (*cardPollingDetectStatus)(sdmmchost_t *host, uint32_t waitCardStatus, uint32_t timeout);
    uint32_t (*cardDetectStatus)(sdmmchost_t *host);
    void (*cardSendActive)(sdmmchost_t *host);
    uint32_t (*cardSetClock)(sdmmchost_t *host, uint32_t targetClock);
    bool (*cardIsBusy)(sdmmchost_t *host);
    
    /* data transfer related functions */
    status_t (*transferFunction)(sdmmchost_t *host, sdmmchost_transfer_t *content);

    /* boot related functions */
    status_t (*startBoot)(sdmmchost_t *host,
                          sdmmchost_boot_config_t *hostConfig,
                          sdmmchost_cmd_t *cmd,
                          uint8_t *buffer);
    status_t (*readBootData)(sdmmchost_t *host, 
                          sdmmchost_boot_config_t *hostConfig, 
                          uint8_t *buffer);
    void (*enableBoot)(sdmmchost_t *host, bool enable);
} sdmmchost_ops_t;

/*!@brief sdmmc instance configurations  */
typedef struct _sdmmchost_config_
{
    uint32_t hostId;
    uint32_t hostType;
#define kSDMMCHOST_TYPE_FSDMMC       1
#define kSDMMCHOST_TYPE_FSDIF        2
    uint32_t cardType;
#define kSDMMCHOST_CARD_TYPE_STANDARD_SD   1
#define kSDMMCHOST_CARD_TYPE_MICRO_SD      2
#define kSDMMCHOST_CARD_TYPE_EMMC          3
#define kSDMMCHOST_CARD_TYPE_SDIO          4  
    bool     enableIrq;
    bool     enableDMA;
    uint32_t endianMode;
    size_t   maxTransSize;
    size_t   defBlockSize;
    uint32_t cardClock;
    bool     isUHSCard;
    void     *timeTuner;
    /* for SDIO card, to support card customized interrupt handling */
    void    (*sdioCardInt)(void *userData);
    void     *sdioCardIntArg;
} sdmmchost_config_t;

/*!@brief sdmmc host handler  */
typedef struct _sdmmchost_
{
    void *dev;                /*!< host controller handler */
    sdmmchost_ops_t ops;
    sdmmchost_config_t config;
    uint32_t currVoltage;
    uint32_t currBusWidth;
    uint32_t currClockFreq;

    uint32_t sourceClock_Hz;        /*!< host configuration */
    uint32_t capability;           /*!< host controller capability */
    uint32_t maxBlockCount;        /*!< host controller maximum block count */
    uint32_t maxBlockSize;         /*!< host controller maximum block size */

    uint8_t tuningType; /*!< host tuning type */

    sdmmc_osa_event_t hostEvent; /*!< host event handler */
    void *card;                  /*!< card instance */
    void *cd;                    /*!< card detect */
    void *cardInt;               /*!< call back function for card interrupt */

    sdmmc_osa_mutex_t lock; /*!< host access lock */
} sdmmchost_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name SDMMC host controller function
 * @{
 */

/*!
 * @brief Init host controller.
 *
 * Thread safe function, please note that the function will create the mutex lock dynamically by default,
 * so to avoid the mutex create redundantly, application must follow bellow sequence for card re-initialization
 * @code
 * SDMMCHOST_Deinit(host);
 * SDMMCHOST_Init(host);
 * @endcode
 *
 * @param host host handler
 * @retval kStatus_Success host init success
 * @retval kStatus_Fail event fail
 */
status_t SDMMCHOST_Init(sdmmchost_t *host);

/*!
 * @brief host reset function.
 *
 * @param host host handler
 */
static inline status_t SDMMCHOST_Reset(sdmmchost_t *host)
{
    if (host->ops.reset)
    {
        return host->ops.reset(host);
    }

    return kStatus_Fail;
}

/*!
 * @brief Deinit host controller.
 * Please note it is a thread safe function.
 *
 * @param host host handler
 */
static inline void SDMMCHOST_Deinit(sdmmchost_t *host)
{
    if (host->ops.deinit)
    {
        host->ops.deinit(host);
    }    
}

/*!
 * @brief card detect init function.
 * @param host host handler
 * @param cd card detect configuration
 */
static inline status_t SDMMCHOST_CardDetectInit(sdmmchost_t *host, void *cd)
{
    if (host->ops.cardDetectInit)
    {
        return host->ops.cardDetectInit(host, cd);
    }

    return kStatus_Fail;
}

/*!
 * @brief host power off card function.
 * @param host host handler
 * @param enable true is power on, false is power down.
 */
static inline void SDMMCHOST_SetCardPower(sdmmchost_t *host, bool enable)
{
    if (host->ops.cardSetPower)
    {
        host->ops.cardSetPower(host, enable);
    }    
}

/*!
 * @brief set data bus width.
 * @param host host handler
 * @param dataBusWidth data bus width
 */
static inline void SDMMCHOST_SetCardBusWidth(sdmmchost_t *host, uint32_t dataBusWidth)
{
    if (host->ops.cardSetBusWidth)
    {
        host->ops.cardSetBusWidth(host, dataBusWidth);
    }      
}

/*!
 * @brief Send initilization active 80 clocks to card.
 * @param host host handler
 */
static inline void SDMMCHOST_SendCardActive(sdmmchost_t *host)
{
    if (host->ops.cardSendActive)
    {
        host->ops.cardSendActive(host);
    } 
}

/*!
 * @brief Set card bus clock.
 * @param host host handler
 * @param targetClock target clock frequency
 * @retval actual clock frequency can be reach.
 */
static inline uint32_t SDMMCHOST_SetCardClock(sdmmchost_t *host, uint32_t targetClock)
{
    if (host->ops.cardSetClock)
    {
        return host->ops.cardSetClock(host, targetClock);
    }

    return 0U;    
}

/*!
 * @brief check card status by DATA0.
 * @param host host handler
 * @retval true is busy, false is idle.
 */
static inline bool SDMMCHOST_IsCardBusy(sdmmchost_t *host)
{
    if (host->ops.cardIsBusy)
    {
        return host->ops.cardIsBusy(host);
    }

    return true;      
}

/*!
 * @brief Get signal line status.
 * @param host host handler
 * @param signalLine signal line type, reference _sdmmc_signal_line
 */
static inline uint32_t SDMMCHOST_GetSignalLineStatus(sdmmchost_t *host, uint32_t signalLine)
{
    if (host->ops.getSignalLineStatus)
    {
        return host->ops.getSignalLineStatus(host, signalLine);
    }

    return 1U;
}

/*!
 * @brief enable card interrupt.
 * @param host host handler
 * @param enable true is enable, false is disable.
 */
static inline void SDMMCHOST_EnableCardInt(sdmmchost_t *host, bool enable)
{
    if (host->ops.cardEnableInt)
    {
        host->ops.cardEnableInt(host, enable);
    }     
}

/*!
 * @brief enable DDR mode.
 * @param host host handler
 * @param enable true is enable, false is disable.
 * @param nibblePos nibble position indictation. 0- the sequence is 'odd high nibble -> even high nibble ->
 * odd low nibble -> even low nibble'; 1- the sequence is 'odd high nibble -> odd low nibble -> even high
 * nibble -> even low nibble'.
 */
static inline void SDMMCHOST_EnableDDRMode(sdmmchost_t *host, bool enable, uint32_t nibblePos)
{
    if (host->ops.enableDDRMode)
    {
        host->ops.enableDDRMode(host, enable, nibblePos);
    }         
}

/*!
 * @brief enable HS400 mode.
 * @param host host handler
 * @param enable true is enable, false is disable.
 */
static inline void SDMMCHOST_EnableHS400Mode(sdmmchost_t *host, bool enable)
{
    if (host->ops.enableHS400Mode)
    {
        host->ops.enableHS400Mode(host, enable);
    }     
}

/*!
 * @brief enable STROBE DLL.
 * @param host host handler
 * @param enable true is enable, false is disable.
 */
static inline void SDMMCHOST_EnableStrobeDll(sdmmchost_t *host, bool enable)
{
    if (host->ops.enableStrobeDll)
    {
        host->ops.enableStrobeDll(host, enable);
    }   
}

/*!
 * @brief start read boot data.
 * @param host host handler
 * @param hostConfig boot configuration
 * @param cmd boot command
 * @param buffer buffer address
 */
static inline status_t SDMMCHOST_StartBoot(sdmmchost_t *host,
                                           sdmmchost_boot_config_t *hostConfig,
                                           sdmmchost_cmd_t *cmd,
                                           uint8_t *buffer)
{
    if (host->ops.getSignalLineStatus)
    {
        return host->ops.startBoot(host, hostConfig, cmd, buffer);
    }

    return kStatus_Fail;
}

/*!
 * @brief read boot data.
 * @param host host handler
 * @param hostConfig boot configuration
 * @param buffer buffer address
 */
static inline status_t SDMMCHOST_ReadBootData(sdmmchost_t *host, 
                                              sdmmchost_boot_config_t *hostConfig, 
                                              uint8_t *buffer)
{
    if (host->ops.readBootData)
    {
        return host->ops.readBootData(host, hostConfig, buffer);
    }

    return kStatus_Fail;
}

/*!
 * @brief enable boot mode.
 * @param host host handler
 * @param enable true is enable, false is disable
 */
static inline void SDMMCHOST_EnableBoot(sdmmchost_t *host, bool enable)
{
    if (host->ops.enableBoot)
    {
        host->ops.enableBoot(host, enable);
    }
}

/*!
 * @brief card interrupt function.
 * @param host host handler
 * @param sdioInt card interrupt configuration
 */
static inline status_t SDMMCHOST_CardIntInit(sdmmchost_t *host, void *sdioInt)
{
    if (host->ops.cardIntInit)
    {
        return host->ops.cardIntInit(host, sdioInt);
    }

    return kStatus_Fail;    
}

/*!
 * @brief force card clock on.
 * @param host host handler
 * @param enable true is enable, false is disable.
 */
static inline void SDMMCHOST_ForceClockOn(sdmmchost_t *host, bool enable)
{
    if (host->ops.cardForceClockOn)
    {
        host->ops.cardForceClockOn(host, enable);
    }    
}

/*!
 * @brief switch to voltage.
 * @param host host handler
 * @param voltage switch to voltage level.
 */
static inline void SDMMCHOST_SwitchToVoltage(sdmmchost_t *host, uint32_t voltage)
{
    if (host->ops.switchToVoltage)
    {
        host->ops.switchToVoltage(host, voltage);
    }     
}

/*!
 * @brief Detect card insert, only need for SD cases.
 * @param host host handler
 * @param waitCardStatus status which user want to wait
 * @param timeout wait time out.
 * @retval kStatus_Success detect card insert
 * @retval kStatus_Fail card insert event fail
 */
static inline status_t SDMMCHOST_PollingCardDetectStatus(sdmmchost_t *host, 
                                                         uint32_t waitCardStatus, 
                                                         uint32_t timeout)
{
    if (host->ops.cardPollingDetectStatus)
    {
        return host->ops.cardPollingDetectStatus(host, waitCardStatus, timeout);
    }

    return kStatus_Fail;   
}

/*!
 * @brief card detect status.
 * @param host host handler
 * @retval kSD_Inserted, kSD_Removed
 */
static inline uint32_t SDMMCHOST_CardDetectStatus(sdmmchost_t *host)
{
    if (host->ops.cardDetectStatus)
    {
        return host->ops.cardDetectStatus(host);
    }

    return 0U;
}

/*!
 * @brief host transfer function.
 *
 * Please note it is a thread safe function.
 *
 * @note the host transfer function support below functionality,
 * 1. Non-cache line size alignment check on the data buffer, it is means that no matter the data buffer used for data
 * transfer is align with cache line size or not, sdmmc host driver will use the address directly.
 * 2. Cache line size alignment check on the data buffer, sdmmc host driver will check the data buffer address, if the
 * buffer is not align with cache line size, sdmmc host driver will convert it to cache line size align buffer, the
 * functionality is enabled by \#define SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER 1 \#define
 * FSL_USDHC_ENABLE_SCATTER_GATHER_TRANSFER 1U If application would like to enable the cache line size align
 * functionality, please make sure the
 *  SDMMCHOST_InstallCacheAlignBuffer is called before submit data transfer request and make sure the installing buffer
 * size is not smaller than 2 * cache line size.
 *
 * @param host host handler
 * @param content transfer content.
 */
static inline status_t SDMMCHOST_TransferFunction(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    if (host->ops.transferFunction)
    {
        return host->ops.transferFunction(host, content);
    }

    return kStatus_Fail;     
}

/*!
 * @brief sdmmc host excute tuning.
 *
 * @param host host handler
 * @param tuningCmd tuning command.
 * @param revBuf receive buffer pointer
 * @param blockSize tuning data block size.
 */
static inline status_t SDMMCHOST_ExecuteTuning(sdmmchost_t *host, 
                                               uint32_t tuningCmd, 
                                               uint32_t *revBuf, 
                                               uint32_t blockSize)
{
    if (host->ops.executeTuning)
    {
        return host->ops.executeTuning(host, tuningCmd, revBuf, blockSize);
    }

    return kStatus_Fail;      
}

/*!
 * @brief sdmmc host convert data sequence to little endian sequence
 *
 * @param host host handler.
 * @param data data buffer address.
 * @param wordSize data buffer size in word.
 * @param format data packet format.
 */
static inline void SDMMCHOST_ConvertDataToLittleEndian(sdmmchost_t *host, 
                                                       uint32_t *data, 
                                                       uint32_t wordSize, 
                                                       uint32_t format)
{
    if (host->ops.convertDataToLittleEndian)
    {
        host->ops.convertDataToLittleEndian(host, data, wordSize, format);
    }
}

/* @} */

#if defined(__cplusplus)
}
#endif
/* @} */
#endif /* _FSL_SDMMC_HOST_H */
