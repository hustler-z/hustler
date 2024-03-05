# FDDMA 驱动程序

## 1. 概述

DDMA(Device Direct Memory Access)是E2000提供的一个通用DMA控制模块，支持典型的DMA操作，提供多个DMA通道，多个通道可以同时工作，独立配置给不同外设使用

## 2. 功能

FDDMA 驱动程序主要完成 DDMA 模块的初始化，DDMA通道的分配与释放。
相关源文件为：
```
fddma
    .
    ├── fddma.c
    ├── fddma.h
    ├── fddma_bdl.c
    ├── fddma_bdl.h
    ├── fddma_g.c
    ├── fddma_hw.c
    ├── fddma_hw.h
    ├── fddma_intr.c
    ├── fddma_selftest.c
    └── fddma_sinit.c
```
其中fddma_bdl.c与fddma_bdl.h是用于I2S相关用例的，仅在DDMA2等控制器上才能使用BDL功能

## 3. 配置方法

以下部分将指导您完成 FDDMA 驱动的软件配置:

- 初始化 DDMA 控制器
- 配置 DDMA 通道，与外设完成绑定
- 启动 DDMA 通道 

## 4 应用示例

### [通过DDMA搬运串口数据完成回环测试](../../../example/peripherals/serial/)

## 5. API参考

### 5.1. 用户数据结构

#### FDdmaConfig

- DDMA 实例配置

```c
typedef struct
{
    u32 id;                     /* DDMA ctrl id */
    uintptr base_addr;          /* DDMA ctrl base address */
    u32 irq_num;                /* DDMA ctrl interrupt id */
    u32 irq_prority;            /* DDMA ctrl interrupt priority */
    u32 caps;
} FDdmaConfig; /* DDMA instance configuration */
```

#### FDdmaChanConfig

- DDMA 通道配置

```c
typedef struct
{
    u32 slave_id;               /* Perpherial slave id for DDMA */
    FDdmaChanRequst req_mode;   /* DDMA transfer direction */
    uintptr ddr_addr;           /* DDMA channel DDR address, could be source or destination, physical address */
    u32 dev_addr;               /* DDMA channel Perpherial base address, could be source or destination */
    u32 trans_len;              /* DDMA channel transfer length */
#define FDDMA_MIN_TRANSFER_LEN      4  /* min bytes in transfer */
    u32 timeout;                /* timeout = 0 means no use DMA timeout */
    /* BDL模式，目前只针对I2S */
    uintptr  first_desc_addr;   /* BDL描述符列表首地址-物理地址 */
    u32      valid_desc_num;    /* 需要使用的BDL描述符个数，从BDL描述符列表第一个描述符开始计数 */
} FDdmaChanConfig;  /* DDMA channel instance */
```

#### FDdmaChanIrq

- DDMA通道回调信息

```c
typedef struct _FDdmaChanIrq
{
	FDdmaChanIndex      channel_id;                               /* 信息所属DDMA通道的ID */
    FDdma               *ddma_instance_p;                         /* 信息所属DDMA通道所属的DDMA控制器实例 */
    void                *evt_handler_args[FDDMA_NUM_OF_CHAN_EVT]; /* DDMA通道事件回调函数输入参数 */
    FDdmaChanEvtHandler evt_handlers[FDDMA_NUM_OF_CHAN_EVT];      /* DDMA通道事件回调函数 */
} FDdmaChanIrq; /* DDMA通道中断回调信息 */
```

#### FDdma

- DDMA 控制器实例

```c
typedef struct _FDdma
{
    FDdmaConfig config;  /* DDMA控制器配置 */
    u32 is_ready;        /* DDMA控制器初始化是否完成 */
    u32 bind_status;     /* DDMA通道绑定标志位，第几位表示第几通道已经被外设绑定并进行过相关配置 */
    FDdmaChanIrq chan_irq_info[FDDMA_NUM_OF_CHAN]; /* DDMA通道事件回调信息集合 */
} FDdma; /* DDMA instance */
```

### 5.2  错误码定义

- FDDMA_SUCCESS                   : 成功  
- FDDMA_ERR_NOT_INIT              ：驱动未初始化
- FFDDMA_ERR_IS_USED              ：通道已被占用
- FDDMA_ERR_INVALID_INPUT         : 非法输入
- FDDMA_ERR_WAIT_TIMEOUT          : DMA等待超时

### 5.3. 用户API接口

#### FDdmaLookupConfig

```c
const FDdmaConfig *FDdmaLookupConfig(u32 instance_id);
```

Note:

- 获取DDMA实例默认配置 

Input:

- {u32} instance_id, DDMA实例号

Return:

- {const FDdmaConfig *} DDMA控制器默认配置

#### FDdmaCfgInitialize

```c
FError FDdmaCfgInitialize(FDdma *const instance, const FDdmaConfig *controller_config);
```

Note:

- 初始化DDMA控制器

Input:

- {FDdma} *instance, DDMA控制器实例
- {FDdmaConfig} *controller_config, DDMA控制器配置

Return:

- {FError} FDDMA_SUCCESS表示初始化成功，其它返回值表示初始化失败

#### FDdmaDeInitialize

```c
void FDdmaDeInitialize(FDdma *const instance_p);
```

Note:

- 去初始化DDMA控制器

Input:

- {FDdma} *instance_p, DDMA控制器实例

Return:

- 无

#### FDdmaChanConfigure

```c
FError FDdmaChanConfigure(FDdma *const instance_p, FDdmaChanIndex channel_id , const FDdmaChanConfig *channel_config);
```

Note:

- 按照配置分配并使能DDMA通道

Input:

- {FDdma} *instance_p, DDMA控制器实例
- {FDdmaChanIndex} channel_id, 通道号
- {FDdmaChanConfig} *channel_config, DDMA通道配置

Return:

- {FError} FDDMA_SUCCESS表示配置成功，其它返回值表示配置失败

#### FDdmaChanDeconfigure

```c
FError FDdmaChanDeconfigure(FDdma *const instance_p, FDdmaChanIndex channel_id);
```

Note:

- 释放之前分配的DDMA通道

Input:

- {FDdma} *instance_p, DDMA控制器实例
- {FDdmaChanIndex} channel_id, 通道号

Return:

- {FError} FDDMA_SUCCESS表示释放成功，其它返回值表示释放失败

#### FDdmaChanActive

```c
void FDdmaChanActive(FDdma *const instance_p, FDdmaChanIndex channel_id);
```

Note:

- 使能指定的DDMA通道

Input:

- {FDdma} *instance_p, DDMA控制器实例
- {FDdmaChanIndex} channel_id, 通道号

Return:

#### FDdmaChanDeactive

```c
void FDdmaChanDeactive(FDdma *const instance_p, FDdmaChanIndex channel_id);
```

Note:

- 去使能指定的DDMA通道

Input:

- {FDdma} *instance_p, DDMA控制器实例
- {FDdmaChanIndex} channel_id, 通道号

Return:

#### FDdmaStart

```c
void FDdmaStart(FDdma *const instance_p);
```

Note:

- 启动DDMA控制器并开始传输，请确保FDdmaChanConfigure()与FDdmaChanActive()步骤已完成

Input:

- {FDdma} *instance_p, DDMA控制器实例

Return:


#### FDdmaStop

```c
void FDdmaStop(FDdma *const instance_p);
```

Note:

- 停止DDMA控制器

Input:

- {FDdma} *instance_p, DDMA控制器实例

Return:

#### FDdmaIrqHandler

```c
void FDdmaIrqHandler(s32 vector, void *args)
```

Note:

- DDMA中断处理函数 

Input:

- {s32} vector
- {void} *param, 输入参数

Return:

- 无

#### FDdmaRegisterChanEvtHandler

```c
void FDdmaRegisterChanEvtHandler(FDdma *const instance_p,
								 FDdmaChanIndex channel_id,
								 FDdmaChanEvt evt,
								 FDdmaChanEvtHandler handler,
								 void *handler_arg);
```

Note:

- 注册DDMA通道中断响应事件函数 

Input:

- {FDdma} *instance_p, DDMA控制器实例
- {FDdmaChanIndex} channel_id, 通道号
- {FDdmaChanEvt} evt, 中断事件
- {FDdmaChanEvtHandler} handler, 中断响应事件函数
- {void} *handler_arg, 中断响应事件函数输入参数

Return:

- 无

#### FDdmaBDLSetDesc

```c
FError FDdmaBDLSetDesc(FDdmaBdlDesc *const first_desc_addr_p, FDdmaBdlDescConfig const *bdl_desc_config_p);
```

Note:

- 停止DDMA控制器

Input:

- {DdmaBdlDesc} *first_desc_addr_p, BDL描述符列表首地址
- {FGdmaBdlDescConfig} *first_desc_addr_p, BDL描述符配置

Return:

- {FError} FDDMA_SUCCESS表示配置成功，其它返回值表示配置失败

#### FDdmaChanBdlConfigure

```c
FError FDdmaChanBdlConfigure(FDdma *const instance, FDdmaChanIndex channel_id, const FDdmaChanConfig *channel_config_p);
```

Note:

- 检查并设置用于DDMA BDL模式的通道

Input:

- {FDdma} *instance_p, DDMA控制器实例
- {FDdmaChanIndex} channel_id, 通道号
- {FDdmaChanConfig} *channel_config, DDMA通道配置

Return:

- {FError} FDDMA_SUCCESS表示配置成功，其它返回值表示配置失败