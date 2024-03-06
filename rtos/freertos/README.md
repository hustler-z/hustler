# Phytium-FreeRTOS-SDK

## 1. 介绍

本项目发布了Phytium系列CPU的FreeRTOS源代码，参考例程以及配置构建工具

代码仓库整体共分为两个分支： 
- master 分支：开发分支，用于保存最新的协作开发代码以及bug修复后的代码。其只要求保障新功能基本正确并且能够满足基本的使用需求，并没有经过系统性和复杂条件下的测试。 
- release 分支：发布分支，包含核心启动代码、芯片外设驱动、用户使用例程和构建的脚本工具。用于保存经过系统性测试的代码并对外发布版本，默认下载此分支的代码。

---

## 2. 快速入门

- 目前支持在Windows和Linux上使用SDK，支持在x86_64和arm aarch64设备上完成交叉编译

![windows](./docs/fig/windows.png)![linux](./docs/fig/linux.png)![输入图片说明](./docs/fig/kylin.png)

- 参考如下说明搭建Phytium FreeRTOS SDK的软件环境

    [Windows10 快速入门](./docs/reference/usr/install_windows.md)

    [Linux x86_64 快速入门](./docs/reference/usr/install_linux_x86_64.md)

    [Linux arm aarch64 快速入门](./docs/reference/usr/install_linux_aarch64.md)


- 参考[使用说明](./docs/reference/usr/usage.md), 新建Phytium FreeRTOS SDK的应用工程，与开发板建立连接
- 参考[例程](./example/template/), 新建Phytium FreeRTOS SDK的例程，在开发板上运行

---

## 3. 硬件参考

### 3.1 FT2000-4

FT-2000/4 是一款面向桌面应用的高性能通用 4 核处理器。每 2 个核构成 1 个处理器核簇（Cluster），并共享 L2 Cache。主要技术特征如下：

- 兼容 ARM v8 64 位指令系统，兼容 32 位指令
- 支持单精度、双精度浮点运算指令
- 支持 ASIMD 处理指令
- 集成 2 个 DDR4 通道，可对 DDR 存储数据进行实时加密
- 集成 34 Lane PCIE3.0 接口：2 个 X16（每个可拆分成 2 个 X8），2 个 X1
- 集成 2 个 GMAC，RGMII 接口，支持 10/100/1000 自适应
- 集成 1 个 SD 卡控制器，兼容 SD 2.0 规范
- 集成 加密计算单元
- 集成 4 个 UART，32 个 GPIO，4 个 I2C，1 个 QSPI，2 个通 用 SPI，2 个 WDT，16 个外部中断（和 GPIO 共用 IO）
- 集成温度传感器

### 3.2 D2000

D2000 是一款面向桌面应用的高性能通用 8 核处理器。每 2 个核构成 1 个处理器核簇（Cluster），并共享 L2 Cache。存储系统包含 Cache 子系统和 DDR，I/O 系统包含 PCIe、高速 IO 子系统、千兆位以太网 GMAC 和低速 IO 子系统，主要技术特征如下，

- 兼容 ARM v8 64 位指令系统，兼容 32 位指令
- 支持单精度、双精度浮点运算指令
- 支持 ASIMD 处理指令
- 集成 2 个 DDR 通道，支持 DDR4 和 LPDDR4，可对 DDR 存储数据进行实时加密
- 集成 34 Lane PCIE3.0 接口：2 个 X16（每个可拆分成 2 个 X8），2 个 X1
- 集成 2 个 GMAC，RGMII 接口，支持 10/100/1000 自适应
- 集成 1 个 SD 卡控制器，兼容 SD 2.0 规范
- 集成 1 个 HDAudio，支持音频输出，可同时支持最多 4 个 Codec
- 集成 SM2、SM3、SM4、SM9 模块
- 集成 4 个 UART，32 个 GPIO，4 个 I2C，1 个 QSPI，2 个通用 SPI，2 个 WDT，16 个外部中断（和 GPIO 共用 IO）
- 集成 2 个温度传感器

### 3.3 E2000Q

- E2000Q 集成2个FTC664核和2个FTC310核。主要技术特征如下：

- 兼容ARM v8 64 位指令系统，兼容32 位指令
- 集成 1 路 16 通道 General DMA 和 1 路 8 通道 Device DMA
- 支持单精度、双精度浮点运算指令
- 两个 FTC664 核各包含 1MB 私有 L2 Cache,由两个 FTC310 核组成的Cluster 内含 256KB 共享的 L2 Cache
- 集成1个DDR4 通道
- 集成6Lane PCIE3.0 接口（X4+2*X1 、X2+4*X2、6*X1）
- 集成4个1000M以太网控制器，支持2路SGMII接口和2路SGMII/RGMII接口
- 集成3路USB2.0(OTG)和2路USB3.0(兼容 2.0)
- 集成2路SATA3.0模块
- 2路 DisplayPort1.4 接口
- 集成常用低速接口：WDT、QSPI、PWM、Nand、SD/SDIO/eMMC 、SPI_M、UART、I2C、I2S、MIO、CAN-FD、GPIO、LocalBus、Timer

### 3.4 E2000D

- E2000D 集成 2 个 FTC310 核。主要技术特征如下：

- 兼容ARM v8 64 位指令系统，兼容32 位指令
- 集成 1 路 16 通道 General DMA 和 1 路 8 通道 Device DMA
- 支持单精度、双精度浮点运算指令
- L2 Cache 有256KB
- 集成1个DDR4 通道
- 集成4 Lane PCIE3.0 接口（4X1）
- 集成4个1000M以太网控制器，支持 2 路 SGMII 接口和 2 路 SGMII/RGMII 接口
- 集成3路USB2.0(OTG)和2路USB3.0(兼容 2.0)
- 集成2路SATA3.0模块
- 2路 DisplayPort1.4 接口
- 集成常用低速接口：WDT，QSPI，PWM，Nand，SD/SDIO/eMMC ，SPI_M，UART，I2C，MIO，CAN-FD，GPIO，LocalBus，Timer

### 3.5 E2000S

- E2000S 集成 1 个 FTC310 核，单核结构。主要技术特征如下：

- 兼容ARM v8 64 位指令系统，兼容32 位指令
- 集成 1 路 16 通道 General DMA 和 1 路 8 通道 Device DMA
- 支持单精度、双精度浮点运算指令
- L2 Cache 有256KB
- 集成1个DDR4 通道
- 集成2 Lane PCIE3.0 接口（2X1）
- 集成3个1000M以太网控制器，支持1路SGMII接口和2路RGMII/RMII接口
- 集成1路USB2.0(Device)和2路USB2.0(OTG)
- 2路 DisplayPort1.4 接口
- 集成常用低速接口：WDT、DMAC、PWM、QSPI、SD/SDIO/eMMC、SPI Master、UART、I2C、MIO、I3C、PMBUS、GPIO、SGPIO、One-Wire、Timer、One-Wire

### 3.6 PHYTIUMPI

- PHYTIUMPI 集成2个FTC664核和2个FTC310核。主要技术特征如下：
- FTC664 核主频可达 1.8GHz，FTC310 核主频可达 1.5GHz
- 兼容ARM v8 64 位指令系统，兼容32 位指令
- 集成 1 路 16 通道 General DMA 和 1 路 8 通道 Device DMA
- 支持单精度、双精度浮点运算指令
- 集成1个DDR4 通道
- 集成1路 Mini-PCIe，支持 AI、5G\4G 等模组
- 集成2个1000M以太网控制器，支持2路SGMII接口和2路SGMII/RGMII接口
- 集成3路USB2.0(OTG)和2路USB3.0(兼容 2.0)
- 1路 HDMI 接口
- 集成常用低速接口：WDT、QSPI、PWM、SD/SDIO/eMMC 、SPI_M、UART、I2C、I2S、MIO、CAN-FD、GPIO、LocalBus、Timer

---

## 4. 例程支持情况

| Feature            | Platform Supported                        | Platform Developing                  | Component              |
| -------------------| ------------------------------------------| ------------------------------------ | ---------------------- |
| EVENTGROUP         | FT2000/4 <br>D2000 <br>E2000 <br>PHYTIUMPI|                                      | eventgroup           |
| INTERRUPT          | FT2000/4 <br>E2000 <br>D2000 <br>PHYTIUMPI|                                      | interrupt                |
| QUEUE              | FT2000/4 <br>D2000 <br>E2000 <br>PHYTIUMPI|                                      | queue           |
| RESOURCE           | FT2000/4 <br>E2000 <br>D2000 <br>PHYTIUMPI|                                      | resource                |
| SOFTWARE_TIMER     | FT2000/4 <br>D2000 <br>E2000 <br>PHYTIUMPI|                                      | software_timer           |
| TASK               | FT2000/4 <br>E2000 <br>D2000 <br>PHYTIUMPI|                                      | task                |
| TASK_NOTIFY        | FT2000/4 <br>D2000 <br>E2000 <br>PHYTIUMPI|                                      | task_notify           |

| Network            | Platform Supported                        | Platform Developing                  | Component              |
| -------------------| ------------------------------------------| ------------------------------------ | ---------------------- |
| LWIP               | FT2000/4 <br>D2000 <br>E2000 <br>PHYTIUMPI|                                      | lwip_startup           |
| UDP                | FT2000/4 <br>E2000 <br>D2000 <br>PHYTIUMPI|                                      | sockets/udp_multicast  |

| Peripherals                    | Platform Supported                        | Platform Developing                  | Component            |
| ------------------------------ | ------------------------------------------| ------------------------------------ | -------------------- |
| ADC                            |                                           | E2000                                | adc                  |
| CAN                            | FT2000/4 <br>E2000 <br>D2000              |                                      | can/can              |
| DDMA                           | E2000 <br>PHYTIUMPI                       |                                      | dma/ddma             |
| GDMA                           | E2000 <br>PHYTIUMPI                       |                                      | dma/gdma             |
| GPIO                           | E2000 <br>PHYTIUMPI                       |                                      | gpio                 |
| I2C                            | E2000 <br>PHYTIUMPI                       | FT2004/D2000                         | i2c                  |
| MEDIA                          | E2000 <br>PHYTIUMPI                       |                                      | media                |
| QSPI (Nor Flash)               | E2000 <br>D2000 <br>FT2000/4              |                                      | qspi                 |
| SPI                            | E2000 <br>PHYTIUMPI                       |                                      | spi                  |
| TIMER & TACHO                  | E2000 <br>PHYTIUMPI                       |                                      | timer&tacho          |
| SDIO                           | E2000 <br>PHYTIUMPI                       |                                      | sd                   |
| PWM                            | E2000 <br>PHYTIUMPI                       |                                      | pwm                  |
| USB                            | E2000 <br>PHYTIUMPI                       |                                      | usb                  |
| WDT                            | FT2000/4 <br>D2000 <br>E2000 <br>PHYTIUMPI|                                      | wdt                  |

| Storage            | Platform Supported                        | Platform Developing                  | Component              |
| -------------------| ------------------------------------------| ------------------------------------ | ---------------------- |
| FATFS              | E2000                                     |                                      | fatfs           |
| QSPI_SPIFFS        | FT2000/4 <br>E2000 <br>D2000              |                                      | qspi_spiffs                |
| SPIM_SPIFFS        | E2000 <br>PHYTIUMPI                       |                                      | spim_spiffs           |

| System             | Platform Supported                        | Platform Developing                  | Component              |
| -------------------| ------------------------------------------| ------------------------------------ | ---------------------- |
| AMP                | E2000 <br>PHYTIUMPI                       | D2000 <br>FT2000/4                   | amp/openamp            |
| ATOMIC             | FT2000/4 <br>D2000 <br>E2000 <br>PHYTIUMPI|                                      | atomic            |
| EXCEPTION_DEBUG    | FT2000/4 <br>D2000 <br>E2000 <br>PHYTIUMPI|                                      | exception_debug        |
| NESTED_INTERRUPT   | FT2000/4 <br>D2000 <br>E2000 <br>PHYTIUMPI|                                      | nested_interrupt       |

---
## 5. 参考资料

- The FreeRTOS Reference Manual API Functions and Configuration Options
- Mastering the FreeRTOS Real Time Kernel A Hands-On Tutorial Guide
- FT-2000／4 软件编程手册-V1.4
- D2000 软件编程手册-V1.0
- 飞腾腾珑E2000系列处理器软件编程手册V0.8.1 
- 飞腾派软件开发手册-V1.0

---
## 6. 贡献方法

请联系飞腾嵌入式软件部

huanghe@phytium.com.cn

zhugengyu@phytium.com.cn

wangxiaodong1030@phytium.com.cn

liushengming1118@phytium.com.cn

---

## 7. 许可协议

Phytium Public License 1.0 (PPL-1.0)