# FPMU 驱动程序

## 1. 概述

性能监视器的基本形式如下：

- 具有64位循环计数器
- 一些64位或32位事件计数器。
- 每个事件计数器计算的事件是可编程的。体系结构为最多提供31个事件计数器的空间。实际的事件计数器数量是实现定义的，并且规范包括一个识别机制。

## 2. 驱动功能

组成由以下所示

.
├── fpmu_perf.c
└── fpmu_perf.h

本驱动为开发者提供了特性：

1. pmu 支持特性检查
2. 提供counter 编程功能
3. 支持counter 溢出中断功能

## 3. 错误码定义

```
#define FPMU_SUCCESS               FT_SUCCESS
#define FPMU_SOC_NOT_SURPPORT      FT_MAKE_ERRCODE(ErrorModGeneral, ErrPmu, 1) /* 错误选择CTLR 寄存器 */
#define FPMU_EVENT_ID_NOT_SUPPORT  FT_MAKE_ERRCODE(ErrorModGeneral, ErrPmu, 2)
#define FPMU_COUNTER_NOT_SUPPORT   FT_MAKE_ERRCODE(ErrorModGeneral, ErrPmu, 3)
#define FPMU_COUNTER_HAS_USED      FT_MAKE_ERRCODE(ErrorModGeneral, ErrPmu, 4)
#define FPMU_COUNTER_NOT_ENABLE    FT_MAKE_ERRCODE(ErrorModGeneral, ErrPmu, 5)

```

## 4. 应用示例

/example/system/arch/armv8/pmu 

## 5. API参考

### 5.1 用户数据结构

- pmu 实例

```c
typedef struct
{
    u32 is_ready;  /* 表示FPmu实例是否已准备好。如果实例已经初始化并且准备就绪，则该值为非零。*/

    FBitPerWordType event_id_bitmap;  /* 用于表示可用事件ID的位图。每个位代表一个事件ID，如果该事件ID可用，则对应位设置为1。*/
    FBitPerWordType event_ext_id_bitmap; /* 用于表示额外事件ID的位图。这用于处理超出标准事件ID范围的事件。*/

    u32 counter_num;  /* PMU中可用的计数器数量。这个值表示FPmu实例能够处理的性能计数器的总数。*/
    FBitPerWordType counter_used_bitmap;  /* 表示当前正在使用的计数器的位图。每个位对应一个计数器，如果计数器正在使用，则设置为1。*/

    struct FPmuCounter counter[FPMU_EVENT_MAX_NUM];  /* 定义了一个FPmuCounter类型的数组，用于存储每个性能计数器的配置和状态。数组大小由FPMU_EVENT_MAX_NUM定义，它表示实例可以监控的最大事件数。*/
} FPmu;
```

```c
struct FPmuCounter
{
    u32 event_id;           /* 用于标识性能监控事件的唯一ID。这个ID用于指定FPmu实例中应该监控的具体性能事件。*/

    u32 event_type_config;  /* 用于配置性能事件类型的参数。这个配置决定了如何监测和记录指定的性能事件。*/

    FPmuEventCB event_cb;   /* 指向事件回调函数的指针。当监控到指定的性能事件时，这个回调函数将被调用。*/

    void *args;             /* 指向传递给事件回调函数的参数的指针。这些参数提供了额外的信息或上下文，用于回调函数的执行。*/
} ;
```

## 5.2 用户API接口

### 1. FPmuCfgInitialize

- 此函数用于初始化FPmu配置，检查功能，重置PMU控制器，并将组件设置为准备就绪。

```c
FError FPmuCfgInitialize(FPmu *instance_p);
```

    Note:
        该函数首先清空FPmu实例的内存，然后进行特性探测。如果特性探测成功，它将重置PMU控制器并将实例状态设置为准备就绪。

    Input:
    - instance_p，指向要初始化的FPmu实例的指针。

    Return:
    - FError，表示初始化过程的状态。成功返回FPMU_SUCCESS，失败返回相应的错误码。

### 2. FPmuCounterConfig

- 此函数用于在FPmu实例中为特定事件ID和周期计数配置一个计数器。

```c
FError FPmuCounterConfig(FPmu *instance_p, u32 counter_id, u32 event_id, FPmuEventCB irq_cb, void *args);
```

    Note:
        在配置计数器前，函数会检查实例指针非空且实例已准备好。它还会验证计数器ID是否有效以及事件ID是否在实例的事件ID位图中。

    Input:
    - instance_p，指向要配置的FPmu实例的指针。
    - counter_id，要配置的计数器的ID。
    - event_id，与计数器关联的事件ID。
    - irq_cb，事件发生时调用的中断回调函数。
    - args，传递给回调函数的参数。

    Return:
    - FError，表示计数器配置操作的状态。成功返回FPMU_SUCCESS，失败返回相应的错误码。

### 3. FPmuCounterEnable

- 此函数用于启用FPmu实例中已配置的计数器。

```c
FError FPmuCounterEnable(FPmu *instance_p, u32 counter_id);
```

    Note:
        在启用计数器之前，函数确保实例指针非空且实例已准备就绪。同时检查计数器ID是否有效且未被使用。函数首先禁用指定的计数器，然后根据需要设置事件类型，最后启用计数器及其中断。

    Input:
    - instance_p，指向FPmu实例的指针。
    - counter_id，要启用的计数器的ID。

    Return:
    - FError，表示计数器启用操作的状态。成功返回FPMU_SUCCESS，失败返回相应的错误码。

### 4. FPmuReadCounter

- 此函数用于从FPmu实例中读取特定计数器的值。

```c
FError FPmuReadCounter(FPmu *instance_p, u32 counter_id, u64 *value);
```

    Note:
        在读取计数器之前，函数确保实例指针非空且实例已准备就绪。它还检查指定的计数器ID是否有效且已启用。根据计数器ID的不同，函数将从相应的计数器读取值。

    Input:
    - instance_p，指向FPmu实例的指针。
    - counter_id，要读取的计数器的ID。
    - value，用于存储读取的计数器值的指针。

    Return:
    - FError，表示计数器读取操作的状态。成功返回FPMU_SUCCESS，失败返回相应的错误码。

### 5. FPmuWriteCounter

- 此函数用于向FPmu实例中的特定计数器写入值。

```c
FError FPmuWriteCounter(FPmu *instance_p, u32 counter_id, u64 value);
```

    Note:
        在写入计数器之前，函数确保实例指针非空且实例已准备就绪。同时检查计数器ID是否有效且已启用。对于AArch32，当使用FPmuWriteCounter接口且计数器ID不是FPMU_CYCLE_COUNT_IDX时，写入的值应为32位数据。

    Input:
    - instance_p，指向FPmu实例的指针。
    - counter_id，要写入的计数器的ID。
    - value，要写入计数器的值。

    Return:
    - FError，表示计数器写入操作的状态。成功返回FPMU_SUCCESS，失败返回相应的错误码。

### 6. FPmuCounterDisable

- 此函数用于禁用FPmu实例中的计数器，以停止计数事件。

```c
FError FPmuCounterDisable(FPmu *instance_p, u32 counter_id);
```

    Note:
        在禁用计数器之前，函数确保实例指针非空且实例已准备就绪。同时检查计数器ID是否有效且已被使用。函数首先禁用指定计数器的计数功能，然后禁用与该计数器相关联的中断。

    Input:
    - instance_p，指向FPmu实例的指针。
    - counter_id，要禁用的计数器的ID。

    Return:
    - FError，表示计数器禁用操作的状态。成功返回FPMU_SUCCESS，失败返回相应的错误码。

### 7. FPmuStart

- 此函数用于启动性能监控单元，通过启用所有被配置好的计数器。

```c
void FPmuStart(void);
```

    Note:
        该函数通过设置PMU控制寄存器中的启用位来启动性能监控单元。此操作将激活所有之前配置的计数器开始计数。

    Input:
    - 无需输入参数。

    Return:
    - 该函数没有返回值。

### 8. FPmuStop

- 此函数用于停止性能监控单元，通过禁用所有计数器。

```c
void FPmuStop(void);
```

    Note:
        该函数通过清除PMU控制寄存器中的启用位来停止性能监控单元。此操作将停止所有计数器的计数活动。

    Input:
    - 无需输入参数。

    Return:
    - 该函数没有返回值。

### 9. FPmuIrqHandler

- 此函数用于处理性能监控单元的中断，检查计数器溢出，并调用回调函数。

```c
void FPmuIrqHandler(s32 vector, void *args);
```

    Note:
        该函数首先检查并重置性能监控单元的中断标志。如果发生计数器溢出，它将停止PMU，处理溢出情况，然后重启PMU。对于每个溢出的计数器，如果配置了回调函数，该函数将被调用。

    Input:
    - vector，中断向量号。
    - args，指向用户定义数据的指针，预期为FPmu实例。

    Return:
    - 该函数没有返回值。

### 10. FPmuDebugCounterIncrease

- 此函数用于增加给定PMU实例的指定性能计数器的值。在尝试增加计数器之前，它会验证PMU实例和计数器索引的有效性。

```c
FError FPmuDebugCounterIncrease(FPmu *instance_p, u32 counter_id);
```

    Note:
        该函数首先确保PMU实例非空且准备就绪。然后检查提供的计数器ID是否在有效范围内且未被使用。若满足条件，函数将增加相应计数器的值。

    Input:
    - instance_p，指向需要增加计数器的FPmu实例的指针。
    - counter_id，要增加的计数器ID。此ID应在PMU实例可用计数器的范围内，或者是特殊的循环计数索引。

    Return:
    - FError，如果实例指针为NULL、组件未准备好、计数器ID不受支持或计数器已被使用，则返回错误码。成功时返回FPMU_SUCCESS。

### 11. `FPmuFeatureProbeDebugPrint`

- 此函数用于打印FPmu实例的调试信息，包括PMU版本、计数器数量和事件ID位图。

```c
void FPmuFeatureProbeDebugPrint(const FPmu *instance_p);
```

    Note:
        该函数首先检查FPmu实例是否为NULL。如果实例有效，它将打印出PMU版本、实例中的计数器数量、原始事件ID位图和扩展事件ID位图。此外，该函数还调用`FPmuPrintEventBitmap` 来打印事件ID位图的详细信息。

    Input:
    - instance_p，指向要进行调试打印的FPmu实例的指针。

    Return:
    - 该函数没有返回值。

### 12. FPmuCheckCounterEventId

- 此函数用于检查FPmu实例中指定计数器的事件ID。

```c
u32 FPmuCheckCounterEventId(FPmu *instance_p, u32 counter_id);
```

    Note:
        该函数首先确保实例非空且已准备就绪。然后验证提供的计数器ID是否有效且已被使用。根据计数器ID的不同，函数将返回不同的事件类型。对于特殊的循环计数器索引，函数将使用不同的方法获取事件类型。

    Input:
    - instance_p，指向FPmu实例的指针。
    - counter_id，要检查事件ID的计数器ID。

    Return:
    - 返回类型u32，表示检查到的事件类型。如果计数器ID不受支持或未使用，将返回错误码。
