# Letter Shell

## 1. 例程介绍

>介绍例程的用途，使用场景，相关基本概念，描述用户可以使用例程完成哪些工作

letter-shell是一个功能强大的嵌入式shell，本例程示范了baremetal环境中的letter-shell移植，通过letter-shell，用户可以很方便地通过定义和导出命令（cmd），与baremetal环境进行交互，便利了增加新功能。

本例程移植的letter-shell是3.0版本，可以通过[Gitee](https://gitee.com/smallqing/letter-shell)访问获取相关源码

本例程支持的cmd包括
- md，读取一段内存的值
- mw，修改一段内存的值
- reboot, 重启baremetal运行环境
- test, 展示如何通过shell获取用户输入参数

## 2. 如何使用例程

><font size="1">描述开发平台准备，使用例程配置，构建和下载镜像的过程</font><br />

本例程需要用到
- Phytium开发板（FT2000-4/D2000/E2000Q/D）
- [Phytium Standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)

### 2.1 硬件配置方法

><font size="1">哪些硬件平台是支持的，需要哪些外设，例程与开发板哪些IO口相关等（建议附录开发板照片，展示哪些IO口被引出）</font><br />

本例程支持的硬件平台包括

- FT2000-4 AARCH32/AARCH64
- D2000 AARCH32/AARCH64
- E2000Q/D AARCH32/AARCH64
- PhytiumPi AARCH32/AARCH64
对应的配置项是，

- CONFIG_TARGET_FT2004
- CONFIG_TARGET_D2000
- CONFIG_TARGET_E2000Q
- CONFIG_TARGET_E2000D

### 2.2 SDK配置方法

><font size="1">依赖哪些驱动、库和第三方组件，如何完成配置（列出需要使能的关键配置项）</font><br />

本例程需要，

- 使能Letter-shell
- 使能串口驱动

对应的配置项是，

- CONFIG_USE_LETTER_SHELL 
- CONFIG_LS_PL011_UART

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
$ make load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_shell
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

- 启动进入letter shell界面，按TAB键打印Command list

![输入图片说明](./fig/ls.png)

#### 2.4.1 使用md/mw命令读写内存

>注意要找一段不用的内存进行修改，内存布局参考make文件夹下的ld文件

- mw -b 0x81001000 0x67 -c 2
- md -b 0x81001000 -c 0x2

![输入图片说明](./fig/mw_md.PNG)

#### 2.4.2 使用reboot命令重启

>使用RAM启动的baremetal环境没有固化（使用aarch32_ram.ld或者aarch64_ram.ld链接的baremetal程序），重启后会跳转到u-boot界面
>以下图片为FT2004 reboot 执行结果  （D2000等其余板子显示可能会有所区别）

![输入图片说明](./fig/power_on.PNG)

## 3. 如何解决问题

><font size="1">主要记录使用例程中可能会遇到的问题，给出相应的解决方案</font><br />

## 4. 修改历史记录

><font size="1">记录例程的重大修改记录，标明修改发生的版本号 </font><br />

- 2021-09-07 ：v0.1.0 添加example
- 2021-10-29 : v0.1.8 修改makefile
- 2022-11-02 : v0.4.0 增加E2000Q/D支持

