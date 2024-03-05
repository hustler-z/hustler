# SATA CONTROLLER 测试

## 1. 例程介绍

><font size="1">介绍例程的用途，使用场景，相关基本概念，描述用户可以使用例程完成哪些工作</font><br />

注:SATA硬盘正确连接开发板后可在启动打印中查看到相关信息，参考例程中使用E2000Q Demo开发板，默认ahci_host = 1和port = 0，若连接方式或开发板不同，请根据实际情况进行修改。SATA硬盘成功连接开发板后，在开发板启动打印内容中，可以查询其信息以供参考。

SATA CONTROLLER PIO模式读写测试例程 (sata_controller_pio_example.c)
- 初始化SATA基本配置
- 在指定起始块处使用PIO模式写入字符串
- 在该块处使用PIO模式读取字符串内容，并打印
- 验证读写操作成功，去初始化SATA

SATA CONTROLLER FPDMA模式读写测试例程 (sata_controller_fpdma_example.c)
- 初始化SATA基本配置
- 在指定起始块处使用FPDMA模式写入字符串
- 在该块处使用FPDMA模式读取字符串内容，并打印
- 验证读写操作成功，去初始化SATA

SATA CONTROLLER PIO中断模式测试例程 (sata_controller_pio_intr_example.c)
- 初始化SATA基本配置，打开中断
- 在指定起始块处使用PIO模式写入字符串
- 在该块处使用PIO模式读取字符串内容，并打印
- 在以上过程中，根据打印信息，可判断是否触发中断
- 验证读写操作成功，屏蔽中断，去初始化SATA

## 2. 如何使用例程

><font size="1">描述开发平台准备，使用例程配置，构建和下载镜像的过程</font><br />

本例程需要以下硬件，

- E2000D/Q Demo 板
- 电源，SATA硬盘，SATA硬盘数据线
- 串口线和串口上位机

### 2.1 硬件配置方法

><font size="1">哪些硬件平台是支持的，需要哪些外设，例程与开发板哪些IO口相关等（建议附录开发板照片，展示哪些IO口被引出）</font><br />
- 如下图所示，SATA硬盘连接电源后，使用数据线将SATA硬盘与E2000D/Q Demo相连，开发板橙色接口为SATA接口
![sata_controller_connect](./fig/sata_controller_connect.png)

### 2.2 SDK配置方法

><font size="1">依赖哪些驱动、库和第三方组件，如何完成配置（列出需要使能的关键配置项）</font><br />

使能例程所需的配置
- Letter Shell组件，依赖 USE_LETTER_SHELL
- SATA组件，依赖CONFIG_USE_SATA

对应的配置项是，
- Use SATA
- Use FIOMUX

- 本例子已经提供好具体的编译指令，以下进行介绍：
    1. make 将目录下的工程进行编译
    2. make clean  将目录下的工程进行清理
    3. make image   将目录下的工程进行编译，并将生成的elf 复制到目标地址
    4. make list_kconfig 当前工程支持哪些配置文件
    5. make load_kconfig LOAD_CONFIG_NAME=<kconfig configuration files>  将预设配置加载至工程中
    6. make menuconfig   配置目录下的参数变量
    7. make backup_kconfig 将目录下的sdkconfig 备份到./configs下

- 具体使用方法为：
    - 在当前目录下
    - 执行以上指令

### 2.3 构建和下载

><font size="1">描述构建、烧录下载镜像的过程，列出相关的命令</font><br />

- 在host侧完成配置

>配置成E2000D，对于其它平台，使用对应的默认配置，如E2000d 32位:
```
$ make load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_sata_co
```

- 在host侧完成构建

```
$ make image
```

- host侧设置重启host侧tftp服务器

```
sudo service tftpd-hpa restart
```

- 开发板侧使用bootelf命令跳转

```
setenv ipaddr 192.168.4.20  
setenv serverip 192.168.4.50 
setenv gatewayip 192.168.4.1 
tftpboot 0x90100000 baremetal.elf
bootelf -p 0x90100000
```

### 2.4 输出与实验现象

><font size="1">描述输入输出情况，列出存在哪些输出，对应的输出是什么（建议附录相关现象图片）</font><br />

#### 2.4.1 SATA CONTROLLER PIO模式读写测试例程
```
$ fsata pio
```
![fsata_pio](./fig/fsata_pio.png)

#### 2.4.2 SATA CONTROLLER FPDMA模式读写测试例程
```
$ fsata fpdma
```
![fsata_fpdma](./fig/fsata_fpdma.png)

#### 2.4.3 SATA CONTROLLER PIO中断模式读写测试例程
```
$ fsata pio_intr
```
![fsata_pio_intr](./fig/fsata_pio_intr.png)

## 3. 如何解决问题

><font size="1">主要记录使用例程中可能会遇到的问题，给出相应的解决方案</font><br />

## 4. 修改历史记录

><font size="1">记录例程的重大修改记录，标明修改发生的版本号 </font><br />



