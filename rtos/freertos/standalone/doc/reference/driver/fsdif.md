# FSDIF 驱动程序

## 1. 概述

SD/SDIF/eMMC控制器，主要支持SD卡，eMMC介质的访问能力，同时支持连接SDIO接口设备，目前 FSDIF 驱动已经支持 SD 卡和 eMMC 卡的访问

## 2. 功能

FSDIF 驱动提供了SD/MMC卡的控制访问方法，
- 初始化 SD/MMC 控制器
- 以轮询方式发送/接收数据和命令

访问 SD/MMC 卡需要兼容一系列协议命令，这一部分驱动不提供，可以通过第三方框架fsl_sdmmc使用

驱动相关的源文件包括，
```
fsdif
    ├── fsdif.c
    ├── fsdif.h
    ├── fsdif_cmd.c
    ├── fsdif_dma.c
    ├── fsdif_g.c
    ├── fsdif_hw.h
    ├── fsdif_intr.c
    ├── fsdif_pio.c
    ├── fsdif_selftest.c
    ├── fsdif_sinit.c
```

## 3. 配置方法

以下部分将指导您完成 FSDIF 驱动的软件配置:

- 初始化 FSDIF 控制器
- 通过协议命令完成 SD/MMC 卡初始化
- 通过协议命令读写 SD/MMC 卡数据

## 4 应用示例


### [通过协议命令读写SD卡](../../../example/peripherals/sd/)

### [在SD卡上使用文件系统](../../../example/storage/fatfs/)

## 5. API参考

### 5.1. 用户数据结构

#### FSdif

- SDIF intance

```c
typedef struct _FSdif
{
    FSdifConfig         config;      /* Current active configs */
    u32                 is_ready;    /* Device is initialized and ready */
    FSdifIDmaDescList   desc_list;   /* DMA descriptor list, valid in DMA trans mode */
    FSdifEvtHandler     evt_handlers[FSDIF_NUM_OF_EVT]; /* call-backs for interrupt event */
    void                *evt_args[FSDIF_NUM_OF_EVT]; /* arguments for event call-backs */
    FSdifRelaxHandler   relax_handler;
    u32                 prev_cmd; /* record previous command code */
    FSdifCmdData        *cur_cmd;
    FSdifTiming         *curr_timing;
} FSdif; /* SDIF intance */
```

#### FSdifConfig

- SDIF intance configuration

```c
typedef struct
{
    u32            instance_id; /* Device instance id */
    uintptr        base_addr;   /* Device base address */
    u32            irq_num;     /* Interrupt num */
    FSdifTransMode trans_mode;  /* Trans mode, PIO/DMA */
    boolean        non_removable; /* No removeable media, e.g eMMC */
    FSdifGetTuning get_tuning; /* Get time-tuning related parameters and method */
} FSdifConfig; /* SDIF intance configuration */
```
#### FSdifCmdData

- SDIF trans command and data 

```c
typedef struct
{
    u32 cmdidx; /* command index */
#define FSDIF_SWITCH_VOLTAGE             11U
    u32 cmdarg; /* command argument */
    u32 response[4]; /* command response buffer */
    u32 flag; /* command flags */
#define FSDIF_CMD_FLAG_NEED_INIT         BIT(1) /* need initialization */
#define FSDIF_CMD_FLAG_EXP_RESP          BIT(2) /* need reply */
#define FSDIF_CMD_FLAG_EXP_LONG_RESP     BIT(3) /* need 136 bits long reply */
#define FSDIF_CMD_FLAG_NEED_RESP_CRC     BIT(4) /* need CRC */
#define FSDIF_CMD_FLAG_EXP_DATA          BIT(5) /* need trans data */
#define FSDIF_CMD_FLAG_WRITE_DATA        BIT(6) /* need trans data to write card */
#define FSDIF_CMD_FLAG_READ_DATA         BIT(7) /* need trans data to read card */
#define FSDIF_CMD_FLAG_NEED_AUTO_STOP    BIT(8) /* need auto stop after command */
#define FSDIF_CMD_FLAG_ADTC              BIT(9) /* need ADTC */
#define FSDIF_CMD_FLAG_SWITCH_VOLTAGE    BIT(10) /* need switch voltage */
#define FSDIF_CMD_FLAG_ABORT             BIT(11) 
    FSdifData *data_p; /* SDIF trans data */
    volatile boolean success; /* TRUE: comand and data transfer success */
} FSdifCmdData; /* SDIF trans command and data */
```
#### FSdifIDmaDesc

- SDIF DMA descriptor

```c
typedef struct
{
    u32 attribute; /* ds0 */
#define FSDIF_IDMAC_DES0_DIC    BIT(1)/* 内部描述表不触发TI/RI中断 */
#define FSDIF_IDMAC_DES0_LD     BIT(2)/* 数据的最后一个描述符 */
#define FSDIF_IDMAC_DES0_FD     BIT(3)/* 数据的第一个描述符 */
#define FSDIF_IDMAC_DES0_CH     BIT(4)/* 链接下一个描述符地址 */
#define FSDIF_IDMAC_DES0_ER     BIT(5)/* 链表已经到达最后一个链表 */
#define FSDIF_IDMAC_DES0_CES    BIT(30)/* RINTSTS寄存器错误汇总 */
#define FSDIF_IDMAC_DES0_OWN    BIT(31)/* 描述符关联DMA，完成传输后该位置置0 */
    u32 non1; /* ds1 --> unused */
    u32 len;  /* ds2 buffer size*/
    u32 non2; /* ds3 --> unused */
    u32 addr_lo; /* ds4 Lower 32-bits of Buffer Address Pointer 1 --> buffer 1 */
    u32 addr_hi; /* ds5 Upper 32-bits of Buffer Address Pointer 1 */
/* Each descriptor can transfer up to 4KB of data in chained mode */
#define FSDIF_IDMAC_MAX_BUF_SIZE        0x1000U
    u32 desc_lo; /* ds6 Lower 32-bits of Next Descriptor Address --> buffer 2 */
    u32 desc_hi; /* ds7 Upper 32-bits of Next Descriptor Address */
} __attribute__((packed)) __attribute((aligned(4))) FSdifIDmaDesc;  /* SDIF DMA descriptr */


```

### 5.2  错误码定义

- FSDIF_SUCCESS           : 操作成功
- FSDIF_ERR_TIMEOUT       ：操作超时失败
- FSDIF_ERR_NOT_INIT      ：控制器未初始化
- FSDIF_ERR_SHORT_BUF     ：缓冲区大小不足
- FSDIF_ERR_NOT_SUPPORT   ：操作不支持
- FSDIF_ERR_INVALID_STATE ：控制器的状态不合法
- FSDIF_ERR_TRANS_TIMEOUT ：传输数据超时失败
- FSDIF_ERR_CMD_TIMEOUT   ：传输命令超时失败
- FSDIF_ERR_NO_CARD       ：卡不在位
- FSDIF_ERR_BUSY          : 卡处于繁忙状态
- FSDIF_ERR_DMA_BUF_UNALIGN : DMA 缓冲区不对齐
- FSDIF_ERR_INVALID_TIMING  : 没有找到时序配置

### 5.3. 用户API接口

#### FSdifLookupConfig

```c
const FSdifConfig *FSdifLookupConfig(u32 instance_id);
```

Note:

- Get the device instance default configure 

Input:

- {u32} instance_id

Return:

- {const FSdifConfig *} default configure

#### FSdifCfgInitialize

```c
FError FSdifCfgInitialize(FSdif *const instance_p, const FSdifConfig *cofig_p);
```

Note:

- initialization SDIF controller instance

Input:

- {FSdif} *instance_p, SDIF controller instance
- {FSdifConfig} *input_config_p, SDIF controller configure

Return:

- {FError} FSDIF_SUCCESS if initialization success, otherwise failed

#### FSdifDeInitialize

```c
void FSdifDeInitialize(FSdif *const instance_p);
```

Note:

- deinitialization SDIF controller instance

Input:

- {FSdif} *instance_p, SDIF controller instance

Return:

- {NONE}

#### FSdioSetIDMAList

```c
FError FSdifSetIDMAList(FSdif *const instance_p, volatile FSdifIDmaDesc *desc, uintptr desc_dma,  u32 desc_num);
```

Note:

- Setup DMA descriptor for SDIF controller instance

Input:

- {FSdif} *instance_p, SDIF controller instance
- {volatile FSdioIDmaDesc} *desc, first item in DMA descriptor lists
- {u32} desc_num, number of items in DMA descriptor lists

Return:

- {FError} FSDIF_SUCCESS if setup done, otherwise failed

#### FSdifSetClkFreq

```c
FError FSdifSetClkFreq(FSdif *const instance_p, u32 input_clk_hz);
```

Note:

- Set the Card clock freqency

Input:

- {FSdif} *instance_p, SDIF controller instance
- {u32} input_clk_hz, Card clock freqency in Hz

Return:

- {None}

#### FSdifDMATransfer

```c
FError FSdifDMATransfer(FSdif *const instance_p, FSdifCmdData *const cmd_data_p);
```

Note:

- Start command and data transfer in DMA mode

Input:

- {FSdif} *instance_p, SDIF controller instance
- {FSdifCmdData} *cmd_data_p, contents of transfer command and data

Return:

- {FError} FSDIF_SUCCESS if transfer success, otherwise failed

#### FSdioPollWaitDMAEnd

```c
FError FSdifPollWaitDMAEnd(FSdif *const instance_p, FSdifCmdData *const cmd_data_p);
```

Note:

- Wait DMA transfer finished by poll

Input:

- {FSdif} *instance_p, SDIF controller instance
- {FSdifCmdData} *cmd_data_p, contents of transfer command and data

Return:

- {FError} FSDIF_SUCCESS if wait success, otherwise wait failed

#### FSdioGetInterruptMask


```c
u32 FSdifGetInterruptMask(FSdif *const instance_p, FSdifIntrType intr_type);
```

Note:

- Get SDIF controller interrupt mask

Input:

- {FSdif} *instance_p, SDIF controller instance
- {FSdifIntrType} type, Type of interrupt, controller/DMA interrupt

Return:

- {u32} interrupt mask bits

#### FSdioSetInterruptMask


```c
void FSdifSetInterruptMask(FSdif *const instance_p, FSdifIntrType type, u32 set_mask, boolean enable);

```

Note:

- Enable/Disable SDIF controller interrupt

Input:

- {FSdif} *instance_p, SDIF controller instance
- {FSdifIntrType} type, Type of interrupt, controller/DMA interrupt
- {u32} set_mask, interrupt mask bits
- {boolean} enable, TRUE: enable interrupt mask bits

Return:

- {NONE}

#### FSdioInterruptHandler

```c
void FSdioInterruptHandler(s32 vector, void *param)
```

Note:

- Interrupt handler for SDIF instance

Input:

- {s32} vector, Interrupt id
- {void} *param, Interrupt params, is SDIF instance

Return:

- {NONE}

#### FSdifRegisterEvtHandler

```c
void FSdifRegisterEvtHandler(FSdif *const instance_p, FSdifEvtType evt, FSdifEvtHandler handler, void *handler_arg);
```

Note:

- Register event call-back function as handler for interrupt events

Input:

- {FSdif} *instance_p, SDIF controller instance
- {FSdifEvtType} evt, interrupt event
- {FSdifEvtHandler} handler, event call-back function
- {void} *handler_arg, argument of event call-back function

Return:

- {NONE}


#### FSdifPIOTransfer

```c
FError FSdifPIOTransfer(FSdif *const instance_p, FSdifCmdData *const cmd_data_p);
```

Note:

- Start command and data transfer in PIO mode

Input:

- {FSdif} *instance_p, SDIF controller instance
- {FSdifCmdData} *cmd_data_p, contents of transfer command and data

Return:

- {FError} FSDIF_SUCCESS if transfer success, otherwise failed

#### FSdifPollWaitPIOEnd

```c
FError FSdifPollWaitPIOEnd(FSdif *const instance_p, FSdifCmdData *const cmd_data_p);
```

Note:

- Wait PIO transfer finished by poll

Input:

- {FSdif} *instance_p, SDIF controller instance
- {FSdifCmdData} *cmd_data_p, contents of transfer command and data

Return:

- {FError} FSDIF_SUCCESS if wait success, otherwise wait failed

#### FSdioGetCmdResponse

```c
FError FSdioGetCmdResponse(FSdif *const instance_p, FSdioCmdData *const cmd_data_p)
```

Note:

- Get cmd response and received data after wait poll status or interrupt signal

Input:

- {FSdif} *instance_p, SDIF controller instance
- {FSdioCmdData} *cmd_data_p, contents of transfer command and data

Return:

- {FError} FSDIF_SUCCESS if get success

#### FSdifRestart

```c
FError FSdifRestart(FSdif *const instance_p);
```

Note:

- reset controller from error state

Input:

- {FSdif} *instance_p, instance of controller

Return:

- {FError} FSDIF_SUCCESS if restart success
