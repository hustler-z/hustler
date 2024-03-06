# FGDMA 驱动程序

## 1. 概述

GDMA(Generic Direct Memory Access)，提供多个DMA通道，多个通道可以同时工作，独立配置给不同内存数据搬运使用


## 2. 功能

FGDMA 驱动程序主要完成 GDMA 模块的初始化，GDMA 通道的分配与释放，
相关源文件为：
```
fgdma
    .
    ├── fgdma.c
    ├── fgdma.h
    ├── fgdma_g.c
    ├── fgdma_hw.h
    ├── fgdma_intr.c
    ├── fgdma_selftest.c
    └── fgdma_sinit.c
```

## 3. 配置方法

以下部分将指导您完成 FGDMA 驱动的软件配置:

- 初始化 GDMA 控制器
- 配置 GDMA 通道，使用直接模式或者 BDL 模式进行操作
- 启动 GDMA 通道

## 4. 应用示例

### [通过GDMA拷贝内存数据](../../../example/peripherals/dma/gdma/)

## 5. API参考

### 5.1. 用户数据结构

#### FGdmaConfig

- GDMA控制器配置

```c
typedef struct
{
    u32                 gdma_id;                /* GDMA控制器ID */
    u32                 irq_num[FGDMA_NUM_OF_CHAN]; /* GDMA控制器中断号 */
    u32                 irq_prority;                /* GDMA控制器中断优先级 */
    u32                 caps;                       /* GDMA控制器能力：标志传输前是否reset与通道中断是否共享等 */
    volatile uintptr_t  base_addr;                  /* GDMA控制器基地址 */
    FGdmaOperPriority   rd_qos;                     /* GDMA控制器读操作Qos优先级，表示GDMA控制器与别的控制器之间优先级关系 */
    FGdmaOperPriority   wr_qos;                     /* GDMA控制器写操作Qos优先级，表示GDMA控制器与别的控制器之间优先级关系 */
    FGdmaWaitEnd        wait_mode;                  /* GDMA控制器结果响应模式 */
} FGdmaConfig;                                      /* GDMA控制器配置 */
```

#### FGdmaChanConfig

- DMA通道配置

```c
typedef struct
{
    FGdmaOperMode     trans_mode;       /* GDMA通道的操作模式，直接模式或者BDL模式 */
    FGdmaWaitEnd      wait_mode;        /* GDMA通道结果响应模式 */
    FGdmaOperPriority rd_qos;           /* GDMA通道读操作Qos优先级，表示控制器内部不同通道之间的优先级关系 */
    FGdmaOperPriority wr_qos;           /* GDMA通道写操作Qos优先级，表示控制器内部不同通道之间的优先级关系*/
    /* Direct模式有效 */
    uintptr           src_addr;         /* GDMA传输源地址-物理地址 */
    uintptr           dst_addr;         /* GDMA传输目的地址-物理地址 */
    fsize_t           trans_length;     /* GDMA传输总数据量 */
    FGdmaBurstType    rd_type;          /* GDMA通道读操作burst type */
    FGdmaBurstType    wr_type;          /* GDMA通道写操作burst type */    
    FGdmaBurstSize    rd_size;          /* GDMA通道读操作burst size */
    FGdmaBurstSize    wr_size;          /* GDMA通道写操作burst size */
    FGdmaBurstLength  rd_length;        /* GDMA通道读操作burst length */
    FGdmaBurstLength  wr_length;        /* GDMA通道写操作burst length */
    /* BDL模式有效 */
    FGdmaBdlDesc      *first_desc_addr; /* BDL描述符列表首地址-物理地址 */
    fsize_t           valid_desc_num;   /* 需要使用的BDL描述符个数，从BDL描述符列表第一个描述符开始计数 */
    boolean           roll_back;        /* GDMA通道循环模式，TRUE: 当前BDL描述符列表传输完成后，再次从第一个BDL描述符重新开始传输 */
} FGdmaChanConfig; /* GDMA通道配置 */
```

#### FGdma

- GDMA控制器实例

```c
typedef struct _FGdma
{
    u32                gdma_id;                /* GDMA控制器ID */
    u32                is_ready;                   /* GDMA控制器初始化是否完成 */
    volatile uintptr_t base_addr;                  /* GDMA控制器基地址 */
    u32                caps;                       /* GDMA控制器能力：标志传输前是否reset与通道中断是否共享等 */
    FGdmaChan          *chans[FGDMA_NUM_OF_CHAN];  /* GDMA通道实例，如果通道没有分配，值为NULL */
} FGdma;                                           /* GDMA控制器实例 */
```

#### FGdmaChanConfig

- BDL描述符配置

```c
typedef struct
{
    fsize_t           current_desc_num; /* 表示当前操作的是所在描述符列表中的第几个描述符 */
    uintptr           src_addr;         /* GDMA传输源地址-物理地址 */
    uintptr           dst_addr;         /* GDMA传输目的地址-物理地址 */
    fsize_t           trans_length;     /* 单个描述符所负责的传输数据量 */
    FGdmaBurstType    rd_type;          /* GDMA通道读操作burst type */
    FGdmaBurstType    wr_type;          /* GDMA通道写操作burst type */    
    FGdmaBurstSize    rd_size;          /* GDMA通道读操作burst size */
    FGdmaBurstSize    wr_size;          /* GDMA通道写操作burst size */
    FGdmaBurstLength  rd_length;        /* GDMA通道读操作burst length */
    FGdmaBurstLength  wr_length;        /* GDMA通道写操作burst length */
    boolean           ioc;              /* TRUE：该描述符传输完成会单独输出一个中断和置位状态位（DMA_Cx_STATE的[2]位）；FLASE：不单独输出 */
} FGdmaBdlDescConfig;  /* BDL描述符配置，同一个描述符列表中的不同描述符可以单独配置为不同的burst type、burst size、burst legth */
```

#### FGdmaBdlDesc

- BDL描述符

```c
typedef struct
{
    u32 src_addr_l;                         /* 0x0, 数据源地址低32位 */
    u32 src_addr_h;                         /* 0x4, 数据源地址高32位 */
    u32 dst_addr_l;                         /* 0x8, 数据目的地址低32位 */
    u32 dst_addr_h;                         /* 0xc, 数据目的地址高32位 */
#define FGDMA_SRC_TC_BDL_BURST_SET(x)      SET_REG32_BITS((x), 1U, 0U)
#define FGDMA_SRC_TC_BDL_SIZE_SET(x)       SET_REG32_BITS((x), 6U, 4U)
#define FGDMA_SRC_TC_BDL_LEN_SET(x)        SET_REG32_BITS((x), 15U, 8U)
    u32 src_tc;                             /* 0x10, 源传输控制位 */
#define FGDMA_DST_TC_BDL_BURST_SET(x)      SET_REG32_BITS((x), 1U, 0U)
#define FGDMA_DST_TC_BDL_SIZE_SET(x)       SET_REG32_BITS((x), 6U, 4U)
#define FGDMA_DST_TC_BDL_LEN_SET(x)        SET_REG32_BITS((x), 15U, 8U)
    u32 dst_tc;                             /* 0x14, 目的传输控制 */
    u32 total_bytes;                        /* 0x18, 传输数据总量，以Byte为单位  */
    u32 ioc;                                /* 0x1c, 该条目传输完成中断产生控制位  */
} __attribute__((__packed__)) FGdmaBdlDesc; /* BDL描述符定义 */
```

```c
/* gdma capacity mask  */

#define FGDMA_IRQ1_MASK BIT(0)   /* All Gdma channel share a single interrupt */
#define FGDMA_IRQ2_MASK BIT(1)   /* Each gdma channel owns an independent interrupt */
#define FGDMA_TRANS_NEED_RESET_MASK BIT(2) /* Gdma needs to be reset before transmission */
```

### 5.2  错误码定义

#define FGDMA_SUCCESS            : 成功     
#define FGDMA_ERR_COMMON         : 常规报错
#define FGDMA_ERR_NOT_INIT       : 驱动未初始化
#define FGDMA_ERR_CHAN_IN_USE    : 通道已经绑定无法分配
#define FGDMA_ERR_CHAN_NOT_INIT  : 通道未初始化
#define FGDMA_ERR_INVALID_ADDR   : 传输地址非法
#define FGDMA_ERR_INVALID_SIZE   : 传输字节数非法
#define FGDMA_ERR_BDL_NOT_ENOUGH : BDL已经使用完

### 5.3. 用户API接口

#### FGdmaLookupConfig

```c
const FGdmaConfig *FGdmaLookupConfig(u32 gdma_id)
```

Note:

- 获取GDMA控制器默认配置

Input:

- {u32} gdma_id, GDMA控制器ID

Return:

- {const FGdmaConfig *} 控制器默认配置

#### FGdmaCfgInitialize

```c
FError FGdmaCfgInitialize(FGdma *const instance_p, const FGdmaConfig *gdama_config_p)
```

Note:

- 初始化GDMA控制器实例

Input:

- FGdma *const instance_p, GDMA控制器实例
- const FGdmaConfig *input_config, GDMA控制器配置

Return:

- {FError} 返回FGDMA_SUCCESS表示初始化成功，返回其它表示失败

#### FGdmaDeInitialize

```c
void FGdmaDeInitialize(FGdma *const instance_p)
```

Note:

- 去初始化GDMA控制器实例

Input:

- FGdma *const instance_p, GDMA控制器实例

Return:

- 无

#### FGdmaChanConfigure

```c
FError FGdmaChanConfigure(FGdma *const instance_p, FGdmaChanIndex channel_id, FGdmaChanConfig const *channel_config_p)
```

Note:

- 配置GDMA通道
- 这一步还不会开始传输，需要在FGdmaStart前使用

Input:

- FGdma *const instance_p, GDMA控制器实例
- FGdmaChanIndex channel_id 操作的GDMA通道的ID
- FGdmaChanConfig *const dma_chan_config, GDMA通道配置

Return:

- {FError} FGDMA_SUCCESS表示配置成功，返回其它值表示配置失败

#### FGdmaChanDeconfigure

```c
FError FGdmaChanDeconfigure(FGdma *const instance_p, FGdmaChanIndex channel_id)
```

Note:

- 释放GDMA通道

Input:

- FGdma *const instance_p GDMA控制器实例
- FGdmaChanIndex channel_id 操作的GDMA通道的ID

Return:

- {FError} FGDMA_SUCCESS表示处理成功

#### FGdmaChanStartTrans

```c
FError FGdmaChanStartTrans(FGdma *const instance_p, FGdmaChanIndex channel_id)
```

Note:

- 启动某个GDMA通道传输

Input:

- FGdma *const instance_p, GDMA控制器实例
- FGdmaChanIndex channel_id 操作的GDMA通道的ID

Return:

- {FError} FGDMA_SUCCESS表示启动成功

#### FGdmaGlobalStartTrans

```c
FError FGdmaGlobalStartTrans(FGdma *const instance_p, FGdmaChanIndex channel_id)
```

Note:

- 全局启动GDMA传输，被配置的GDMA通道将同时开始传输

Input:

- FGdma *const instance_p, GDMA控制器实例

Return:

- {FError} FGDMA_SUCCESS表示启动成功

#### FGdmaChanAbort

```c
FError FGdmaChanAbort(FGdmaChan *const chan_instance_p)
```

Note:

- GDMA通道Abort操作，可以暂停某个通道的输出，也可以作为单通道传输完毕后的通道停止操作

Input:

- FGdma *const instance_p, GDMA控制器实例
- FGdmaChanIndex channel_id 操作的GDMA通道的ID

Return:

- {FError} FGDMA_SUCCESS表示处理成功

#### FGdmaIrqHandler

```c
void FGdmaIrqHandler(s32 vector, void *args)
```

Note:

- 当 FGdmaConfig.caps 为FGDMA_IRQ1_MASK 特性时，各通道统一上报至一个中断，选择使用此函数作为中断处理函数

Input:

- {s32} vector, 中断号
- {void} *args, 中断参数

Return:

- 无

#### FGdmaIrqHandlerPrivateChannel
```c
void FGdmaIrqHandlerPrivateChannel(s32 vector, void *args)
```

Note:

- 当 FGdmaConfig.caps 为FGDMA_IRQ2_MASK 特性时，各通道独立上报中断，选择使用此函数作为中断处理函数

Input:

- {s32} vector, 中断号
- {void} *args, 中断参数

Return:

- 无

#### FGdmaChanRegisterEvtHandler

```c
void FGdmaChanRegisterEvtHandler(FGdmaChan *const chan_p, FGdmaChanEvtType evt, 
                                 FGdmaChanEvtHandler handler, void *handler_arg)
```

Note:

- 注册GDMA通道事件回调函数

Input:

- {FGdmaChan} *chan_p, GDMA通道实例
- {FGdmaChanEvtType} evt, 通道事件
- {FGdmaChanEvtHandler} handler, 事件回调函数
- {void} *handler_arg, 事件回调函数输入参数

Return:

- 无
