# FCAN_OS 驱动程序

## 1. 概述

CAN 是控制器局域网络(Controller Area Network)的缩写，由以研发和生产汽车电子产品著称的德国BOSCH公司开发，并最终成为国际标准(ISO 11898)，是国际上应用最广泛的现场总线之一。

## 2. 功能

CAN控制器驱动提供了CAN的控制访问方法，
- 初始化CAN控制器
- 以中断方式发送/接收数据例程
- 以任务方式发送/接收数据例程
- can id滤波测试例程

驱动相关的源文件包括，
```
.
├── fcan_os.c
└── fcan_os.h
```

## 3. 配置方法

以下部分将指导您完成 fcan_os 驱动的软件配置:

- 初始化CAN控制器
- 设置CAN的中断处理函数，包括收发中断

## 4 应用示例

### [can收发数据](../../../example/peripheral/can/can/)

## 5. API参考

### 5.1. 用户数据结构

- fcan_os控制操作
```c
enum
{
    FREERTOS_CAN_CTRL_ENABLE = 0,       /* enable can */
    FREERTOS_CAN_CTRL_DISABLE = 1,      /* disable can */
    FREERTOS_CAN_CTRL_BAUDRATE_SET,     /* set can baudrate */
    FREERTOS_CAN_CTRL_STATUS_GET,       /* get can status */
    FREERTOS_CAN_CTRL_ID_MASK_SET,      /* set can receive id mask */
    FREERTOS_CAN_CTRL_ID_MASK_ENABLE,   /* enable can receive id mask */
    FREERTOS_CAN_CTRL_INTR_SET,         /* set can interrupt handler */
    FREERTOS_CAN_CTRL_INTR_ENABLE,      /* enable can interrupt */
    FREERTOS_CAN_CTRL_FD_ENABLE,        /* set can fd enable */
    FREERTOS_CAN_CTRL_MODE_SET,         /* set can transmit mode */
    FREERTOS_CAN_CTRL_NUM
};
```

- FreeRTOS中can消息队列
```c
typedef struct
{
    u32 count;
    FFreeRTOSCan *os_can_p;
} FCanQueueData;
```

- fcan_os控制数据，FCanCtrl主要是fcan控制数据，xSemaphoreHandle是信号量句柄
```c
typedef struct
{
    FCanCtrl can_ctrl;              /* can object */
    xSemaphoreHandle can_semaphore; /* can semaphore for resource sharing */
} FFreeRTOSCan;
```

- fcan控制数据
```c
typedef struct
{
    FCanConfig config;
    u32 is_ready;       /* Device is initialized and ready */
    boolean use_canfd;  /* if use canfd function */
    
    FCanIntrEventConfig intr_event[FCAN_INTR_EVENT_NUM];/* event handler and parameters for interrupt */
} FCanCtrl;
```

- fcan配置数据，FCanConfig主要是can控制器id、基地址和中断号，FCanIntrEventConfig主要包括中断处理函数
```c
typedef struct
{
    u32 instance_id;        /* Id of device */
    uintptr base_address;   /* Can base Address */
    u32 irq_num;            /* interrupt number */
    u32 irq_prority;        /* interrupt priority*/
}FCanConfig;
```

- fcan波特率配置
```c
typedef struct
{
    FCanSegmentType segment;
    boolean auto_calc;  /* if auto calculate baudrate parameters */
    u32 baudrate;       /* baudrate */
    u32 sample_point;   /* sample point */
    u32 prop_seg;       /* Propagation segment in TQs */
    u32 phase_seg1;     /* Phase buffer segment 1 in TQs */
    u32 phase_seg2;     /* Phase buffer segment 2 in TQs */
    u32 sjw;            /* Synchronisation jump width in TQs */
    u32 brp;            /* Baudrate prescaler */
}FCanBaudrateConfig;
```

- fcan报文
```c
typedef struct
{
    u32 canid;
    u8 candlc;
    u8 flags; /* additional flags for CAN FD */
    u8 data[FCAN_DATA_LENGTH] __attribute__((aligned(8)));
}FCanFrame;
```

- fcan中断事件类型
```c
typedef enum
{
    FCAN_INTR_EVENT_SEND = 0,    /* Handler type for frame sending interrupt */
    FCAN_INTR_EVENT_RECV = 1,    /* Handler type for frame reception interrupt */
    FCAN_INTR_EVENT_ERROR,       /* Handler type for error interrupt */
    FCAN_INTR_EVENT_NUM
} FCanIntrEventType;
```

### 5.2  错误码定义
- FCAN_SUCCESS      执行成功
- FCAN_NOT_READY    驱动未初始化
- FCAN_FAILURE      执行失败
- FCAN_INVAL_PARAM  参数无效

### 5.3. 用户API接口

#### FFreeRTOSCanInit
- 初始化FreeRTOS的can实例
```c
FFreeRTOSCan *FFreeRTOSCanInit(u32 instance_id);
```

Note:
- 获取默认配置参数，包括基地址、中断号等。创建互斥量

Input:
- {u32} instance_id，控制器id号

Return:
- {FFreeRTOSCan *} 返回can实例指针

#### FFreeRTOSCanDeinit
- 去初始化can实例
```c
FError FFreeRTOSCanDeinit(FFreeRTOSCan *os_can_p)
```

Note:
- 停止can、去初始化can、删除互斥量

Input:
- {FFreeRTOSCan} *os_can_p，can实例指针

Return:
- {FError} 驱动错误码信息，FCAN_SUCCESS表示成功，其它返回值表示失败

#### FFreeRTOSCanControl
- 控制FreeRTOS中的can实例
```c
FError FFreeRTOSCanControl(FFreeRTOSCan *os_can_p, int cmd, void *arg)
```

Note:
- 使能can、禁用can、设置can波特率、获取can状态、设置id掩码、使能id掩码、设置can中断句柄、使能can中断、使能canfd、设置can传输模式

Input:
- {FFreeRTOSCan} *os_can_p，can实例指针
- {int} cmd，控制命令
- {void} *arg，控制命令参数

Return:
- {FError} 驱动错误码信息，FCAN_SUCCESS表示成功，其它返回值表示失败

#### FFreeRTOSCanSend
- FreeRTOS中can消息发送函数
```c
FError FFreeRTOSCanSend(FFreeRTOSCan *os_can_p, FCanFrame *frame_p)
```

Note:
- 通过can实例发送can消息

Input:
- {FFreeRTOSCan} *os_can_p，can实例指针
- {FCanFrame} *frame_p，can数据

Return:
- {FError} 驱动错误码信息，FCAN_SUCCESS表示成功，其它返回值表示失败

#### FFreeRTOSCanRecv
- FreeRTOS中can消息接收函数
```c
FError FFreeRTOSCanRecv(FFreeRTOSCan *os_can_p, FCanFrame *frame_p)
```

Note:
- 通过can实例接收can消息

Input:
- {FFreeRTOSCan} *os_can_p，can实例指针
- {FCanFrame} *frame_p，can数据

Return:
- {FError} 驱动错误码信息，FCAN_SUCCESS表示成功，其它返回值表示失败

#### FCanLookupConfig
- 获取fcan控制器默认配置
```c
const FCanConfig *FCanLookupConfig(u32 instance_id);
```

Note:
- 获取默认配置参数，包括基地址、中断号等

Input:
- {u32} instance_id，控制器id号

Return:
- {const FCanConfig *} fcan默认配置，返回NULL如果找不到默认配置

#### FCanCfgInitialize
- 初始化fcan控制器, 使之可以使用
```c
FError FCanCfgInitialize(FCanCtrl *instance_p, const FCanConfig *input_config_p);
```

Note:
- 输入配置通过FCanLookupConfig获取，用户按照需要修改后传入此函数

Input:
- {FCanCtrl} *instance_p fcan驱动控制数据
- {FCanConfig} *input_config_p fcan用户输入配置

Return:
- {FError} 驱动初始化的错误码信息，FCAN_SUCCESS 表示初始化成功，其它返回值表示初始化失败

#### FCanSend
- 发送can数据
```c
FError FCanSend(FCanCtrl *instance_p, FCanFrame *frame_p);
```

Note:
- 指定can控制器发送can数据

Input:
- {FCanCtrl} *instance_p，fcan驱动控制数据
- {FCanFrame} *frame_p，can数据

Return:
- {FError} 驱动初始化的错误码信息，FCAN_SUCCESS 表示初始化成功，其它返回值表示初始化失败

#### FCanRecv
- 接收can数据
```c
FError FCanRecv(FCanCtrl *instance_p, FCanFrame *frame_p);
```

Note:
- 指定can控制器接收can数据

Input:
- {FCanCtrl} *instance_p，fcan驱动控制数据
- {FCanFrame} *frame_p，can数据

Return:
- {FError} 驱动初始化的错误码信息，FCAN_SUCCESS 表示初始化成功，其它返回值表示初始化失败

#### FCanRegisterInterruptHandler
- 注册can中断事件函数
```c
void FCanRegisterInterruptHandler(FCanCtrl *instance_p, FCanIntrEventConfig *intr_event_p);
```

Note:
- 无

Input:
- {FCanCtrl} *instance_p，fcan驱动控制数据
- {FCanIntrEventConfig} *intr_event_p，中断事件类型，回调函数，回调函数参数

Return:
- 无

#### FCanIntrHandler
- can中断处理函数入口
```c
void FCanIntrHandler(s32 vector, void *args);
```

Note:
- 根据中断类型，设置对应的回调函数和参数传入

Input:
- {s32} vector
- {void} *param, 输入参数，指向fcan驱动控制数据

Return:
- 无

#### FCanIdMaskFilterSet
- can id过滤设置
```c
FError FCanIdMaskFilterSet(FCanCtrl *instance_p, FCanIdMaskConfig *id_mask_p);
```

Note:
- 设置需要过滤的 can_id 的位以及对应的 can_maskid 和 can_mask
- 将 can_id 与过滤器中的 can_maskid 进行按位与运算
- 结果为1的位则忽略此位与接收帧对应位的匹配情况

Input:
- {FCanCtrl} *instance_p, fcan驱动控制数据
- {FCanIdMaskConfig} *id_mask_p, 过滤寄存器序号，可接收帧id，可接收帧id掩码

Return:
- {FError} 驱动初始化的错误码信息，FCAN_SUCCESS 表示初始化成功，其它返回值表示初始化失败

#### FCanCalcBittiming
- 根据目标波特率和采样点要求，通过计算得到最佳的定时器参数配置
```c
static FError FCanCalcBittiming(FCanBaudrateConfig *bt_p, u32 target_baudrate, u32 target_sample_point, FCanSegmentType target_segment);
```

Note:
- 根据目标波特率选择合适的标准采样点
- 通过遍历不同的 tseg（段总和值），计算波特率预分频器（brp）和波特率的误差，并根据误差找出最佳的 tseg 和 brp 值
- 计算出最佳的 tseg 值后，更新采样点值
- 对 sjw（同步跳转宽度）进行检查，并根据 tseg2 的值进行限制
- 存储最终计算得到的参数值到 bt 结构体中

Input:
- {FCanBaudrateConfig} *bt_p, 波特率计算参数
- {u32} target_baudrate, 目标波特率
- {u32} target_sample_point, 目标采样点
- {FCanSegmentType} target_segment, 目标段类型

Return:
- {FError} 驱动初始化的错误码信息，FCAN_SUCCESS 表示初始化成功，其它返回值表示初始化失败