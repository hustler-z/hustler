# can base on freertos

## 1. 例程介绍

本例程示范了freertos环境下的can的使用，包括can的初始化、can周期发送，接收操作；
程序启动后，创建can初始化任务，设置can波特率，中断函数、id mask等；
创建can定时发送任务FFreeRTOSCanSendTask，用于定时发送can报文；
创建can接收任务FFreeRTOSCanRecvTask，用于接收can报文；
使用B板进行测试，选择can0和can1回环；

CAN中断模式回环测试例程 (can_intr_loopback_mode_example.c)

- 初始化CAN0，CAN1基本配置,仲裁域波特率和数据域波特率配置为 1M/S+1M/S,设置滤波器，初始化中断函数
- 中断触发CAN0发送，CAN1接收，循环收发标准帧。并比对发送帧和接收帧是否相同
- 中断触发CAN1发送，CAN0接收，循环收发标准帧。并比对发送帧和接收帧是否相同
- 中断触发CAN0发送，CAN1接收，循环收发扩展帧。并比对发送帧和接收帧是否相同
- 中断触发CAN1发送，CAN0接收，循环收发扩展帧。并比对发送帧和接收帧是否相同
- 以上收发测试完成后，去初始化CAN0,CAN1，删除发送接收任务

CAN轮询模式回环测试例程 (can_polled_loopback_mode_example.c)

- 初始化CAN0，CAN1基本配置,仲裁域波特率和数据域波特率配置为 1M/S+1M/S,设置滤波器
- CAN0发送，CAN1接收，循环收发标准帧。并比对发送帧和接收帧是否相同
- CAN1发送，CAN0接收，循环收发标准帧。并比对发送帧和接收帧是否相同
- CAN0发送，CAN1接收，循环收发扩展帧。并比对发送帧和接收帧是否相同
- CAN1发送，CAN0接收，循环收发扩展帧。并比对发送帧和接收帧是否相同
- 以上收发测试完成后，去初始化CAN0,CAN1，删除发送接收任务

CAN过滤功能测试例程 (can_id_filter_example.c)

- 初始化CAN0,CAN1基本配置,仲裁域波特率和数据域波特率配置为 1M/S+1M/S
- 过滤模式1配置为只可接收id为0x0F的帧
- CAN0向CAN1发送id为（0x00~0x0F）的标准帧，CAN1成功接收id=0x0F的帧，表示成功过滤除0x0F以外的所有帧
- CAN1向CAN0发送id为（0x00~0x0F）的标准帧，CAN0成功接收id=0x0F的帧，表示成功过滤除0x0F以外的所有帧
- 过滤模式2配置为比较较高的两位，忽略较低的两位
- CAN0向CAN1发送id为（0x00~0x0F）的标准帧，CAN1成功接收id=0x0C、0x0D、 0x0E、0x0F的帧
- CAN1向CAN0发送id为（0x00~0x0F）的标准帧，CAN0成功接收id=0x0C、0x0D、 0x0E、0x0F的帧
- 以上收发测试完成后，去初始化CAN0,CAN1，删除发送接收任务

## 2. 如何使用例程

本例程需要用到

- Phytium开发板（E2000D/E2000Q/D2000/FT2000-4）
- [Phytium freeRTOS SDK](https://gitee.com/phytium_embedded/phytium-free-rtos-sdk)
- [Phytium standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)

### 2.1 硬件配置方法

本例程支持的硬件平台包括

- FT2000-4
- D2000
- E2000D
- E2000Q

对应的配置项是，

- CONFIG_TARGET_FT2004
- CONFIG_TARGET_D2000
- CONFIG_TARGET_E2000D
- CONFIG_TARGET_E2000Q

### 2.2 SDK配置方法

本例程需要，

- 使能Shell
- 使能Can

对应的配置项是，

- CONFIG_USE_LETTER_SHELL
- CONFIG_FREERTOS_USE_CAN
- CONFIG_USE_CAN

本例子已经提供好具体的编译指令，以下进行介绍:

- make 将目录下的工程进行编译
- make clean  将目录下的工程进行清理
- make image   将目录下的工程进行编译，并将生成的elf 复制到目标地址
- make list_kconfig 当前工程支持哪些配置文件
- make load_kconfig LOAD_CONFIG_NAME=`<kconfig configuration files>`  将预设配置加载至工程中
- make menuconfig   配置目录下的参数变量
- make backup_kconfig 将目录下的sdkconfig 备份到./configs下

具体使用方法为:

- 在当前目录下
- 执行以上指令

### 2.3 构建和下载

> `<font size="1">`描述构建、烧录下载镜像的过程，列出相关的命令 `</font><br />`

[参考 freertos 使用说明](../../../../docs/reference/usr/usage.md)

#### 2.3.1 下载过程

- host侧设置重启host侧tftp服务器

```
sudo service tftpd-hpa restart
```

- 开发板侧使用bootelf命令跳转

```
setenv ipaddr 192.168.4.20  
setenv serverip 192.168.4.50 
setenv gatewayip 192.168.4.1 
tftpboot 0x90100000 freertos.elf
bootelf -p 0x90100000
```

### 2.4 输出与实验现象

- 系统进入后，输入 ``can``查看指令说明
- 执行相应的测试例子，创建测试任务
- 测试任务能够能正常创建和删除，输入 ``ps``查看任务状态正常，即测试正常

![can](./figs/can.png)

- 输入 ``can intr``，启动can中断模式发送接收测试例子，测试完标准帧和扩展帧后自动删除任务

![intr_stid](./figs/intr_stid.png)
......
![intr_exid](./figs/intr_exid.png)

- 输入 ``can polled``，启动can轮询模式发送接收测试例子，测试完标准帧和扩展帧后自动删除任务

![polled_stid](./figs/polled_stid.png)
......
![polled_exid](./figs/polled_exid.png)

- 输入 ``can filter``，启动can id滤波功能测试例子，测试1只接收id=0x0F的帧，测试2接收帧id&mask(mask=0x03，结果为0表示比较，为1表示忽略)，比较接收id和接收id&maskid(maskid=0x0F)的结果。全部匹配则接收否则滤除。
滤波测试例子1
![filter1](./figs/filter1.png)
......
![filter1_success](./figs/filter1_success.png)
滤波测试例子2
![filter2](./figs/filter2.png)
......
![filter2_success](./figs/filter2_success.png)

## 3. 如何解决问题

- 使用can0和can1进行回环测试时，需要将can0和can1的H和L信号线分别进行短接

## 4. 修改历史记录
