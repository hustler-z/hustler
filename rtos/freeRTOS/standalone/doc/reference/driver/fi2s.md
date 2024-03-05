# I2S 驱动程序

## 1. 概述

- I2S 是针对数字音频设备之间的音频数据传输而制定的一种总线标准，I2S 控制器
主要实现音频数据的发送与接收。
- E2000 的 I2S接口需要配合ES8336芯片使用，在使用使请确保ES8336芯片已能利用I2C接口配置参数，在本驱动中已进行设置。
- E2000 支持的采样率主要有8k，32k,44.1k,48k,96k等，通常建议使用44.1K采样率

## 2. 驱动功能

I2S 驱动程序管理音频数据的发送与接收，该驱动程序具备以下功能：

- 音频数据的发送与接收
- DDMA功能的扩展

## 3. 使用方法

以下部分将指导您完成 FI2S 驱动的硬件配置：

- 1. I2S驱动支持 E2000 Q D ，在E2000上完成测试

以下部分将指导您完成 FI2S 驱动的软件配置：

- 1. 配置驱动程序，新建应用工程，设置I2S参数
- 2. 得到设备参数，操作设置配置项目
- 3. 进行I2S,DDMA,I2C操作配置流程

## 5. API参考

### 5.1. 用户数据结构

- drivers/i2s/fi2s/fi2s.h

```c

typedef struct
{
    FI2sConfig  config;
    u32         is_ready;
    FI2SIntrEventHandler event_handler[FI2S_INTR_EVENT_NUM];
    void *event_param[FI2S_INTR_EVENT_NUM];   /* parameters of event handler */
    FI2sData    data_config;
} FI2s;
```

```c

typedef struct
{
    u32 instance_id;
    uintptr base_addr;
    u32 irq_num;
    u32 irq_prority;
} FI2sConfig;
```

```c

typedef enum
{
    FI2S_SAMPLE_RATE_PHONE = 8000,
    FI2S_SAMPLE_RATE_DAT = 32000,
    FI2S_SAMPLE_RATE_CD = 44100,
    FI2S_SAMPLE_RATE_DV = 48000,
    FI2S_SAMPLE_RATE_DVD = 96000,
} FI2sDataSampleRate;


```

### 5.3 用户API接口

```c

const FI2sConfig *FI2sLookupConfig(u32 instance_id)
```
- 获取I2S驱动的默认配置参数

Input:

    - u32 instance_id, 当前控制的i2s控制器实例号    

Return:

    - const FI2sConfig *, 返回驱动默认参数， NULL表示失败


```c

FError FI2sCfgInitialize(FI2s *instance, const FI2sConfig *config_p);
```
- 初始化I2S控制器，使之能够使用

Input:
    - FI2s* instance, 当前控制器的实例
    - const FI2sConfig * config_p, I2S默认配置
Return:
    - @return 0:FI2S_SUCCESS,other:failed

```c

void FI2sTxRxEnable(FI2s *instance, boolean enable);
```
- 使能发送或接收模块

Input:
    - FI2s* instance_p, 当前控制器的实例
    - boolean enable, enable or disable

Return:
    - @return null

```c

void FI2sClkOutDiv(FI2s *instance_p)
```
- 使能I2S时钟输出


Input:
    - FI2s* instance_p, 当前控制器的实例

Return:
    - @return null

```c

void FI2sStopWork(FI2s *instance_p)
```

- 停止I2S

Input:
    - FI2s* instance_p, 当前控制器的实例

Return:
    - @return null

```c

void FI2sRxHwEnable(FI2s *instance);
```
-  设置I2S接收模块参数


Input:
    - FI2s* instance_p, 当前控制器的实例

Return:
    - @return null

```c

void FI2sTxHwEnable(FI2s *instance);
```
- 设置I2S发送模块参数

Input:
    - FI2s* instance_p, 当前控制器的实例

Return:
    - @return null
```c

void FI2sEnableIrq(FI2s *instance, u32 event_type);
 ```
- 中断函数使能

Input:
    - FI2s* instance_p, 当前控制器的实例
    - u32  event_type  中断类型


Return:
    - @return null
```c

void FI2sDisableIrq(FI2s *instance_p, u32 event_type)
 ```
- 去使能中断

Input:
    - FI2s* instance_p, 当前控制器的实例
    - u32  event_type  中断类型


Return:
    - @return  null
```c

 void FI2sRegisterInterruptHandler(FI2s *instance_p, FI2SIntrEventType event_type,FI2SIntrEventHandler FI2SIntrEventHandler, void *param)
 ```
- 中断注册函数

Input:
    - FI2s* instance_p, 当前控制器的实例
    - FI2SIntrEventType  event_type  中断类型
    - FI2SIntrEventHandler FI2SIntrEventHandler 中断处理函数
    - void *param   中断相关参数

Return:
    - @return null
```c
void FI2sIntrHandler(s32 vector, void *args)
 ```
 - 中断回调函数

Input:
    - {s32} vector
    - {void} *args, 输入参数，指向fadc驱动控制数据
 
Return:
    - @return null
```c
