# Atomic Example

## 1. 例程介绍

><font size="1">介绍例程的用途，使用场景，相关基本概念，描述用户可以使用例程完成哪些工作</font><br />

GCC 4.1.2版本之后，提供了许多原子函数（Atomic Built-in Functions），这些函数主要用于实现线程安全的共享内存访问，可以保证共享变量的原子性、顺序性和正确性。
原子函数在多线程或多进程编程中广泛应用，特别是在实现同步机制、锁和条件变量等方面具有重要作用。

本例程只要针对原子操作的有效性进行相关测试。

## 2. 如何使用例程

><font size="1">描述开发平台准备，使用例程配置，构建和下载镜像的过程</font><br />

本例程需要以下硬件，

- E2000 demo，FT2000/4，D2000
- 串口线和串口上位机

### 2.1 硬件配置方法

><font size="1">哪些硬件平台是支持的，需要哪些外设，例程与开发板哪些IO口相关等（建议附录开发板照片，展示哪些IO口被引出）</font><br />

- 使用E2000 demo板进行测试

### 2.2 SDK配置方法

><font size="1">依赖哪些驱动、库和第三方组件，如何完成配置（列出需要使能的关键配置项）</font><br />

本例程需要，

- 使能Letter Shell `CONFIG_USE_LETTER_SHELL`, 然后选择UART驱动 `CONFIG_LS_PL011_UART`

对应的配置项是，

- Use Letter Shell

本例子已经提供好具体的编译指令，以下进行介绍:
- make all 将目录下的工程进行编译
- make clean  将目录下的工程进行清理
- make image   将目录下的工程进行清理后编译，并将生成的elf 复制到目标地址
- make list_kconfig 当前工程支持哪些配置文件
- make load_kconfig LOAD_CONFIG_NAME=<kconfig configuration files>  将预设配置加载至工程中
- make menuconfig   自行配置目录下的参数变量
- make backup_kconfig 将目录下自行配置的sdkconfig 备份到./configs下

具体使用方法为：
- 在当前目录下
- 执行以上指令

### 2.3 构建和下载

><font size="1">描述构建、烧录下载镜像的过程，列出相关的命令</font><br />

- 在host侧完成配置

>配置成E2000D，对于其它平台，使用对应的默认配置，如E2000d 32位:
```
$ make load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_atomic
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

依次执行2.4.1,2.4.2命令

### 2.4.1 进行原子操作测试

```
$ atomic start
```
![atomic_start](./fig/atomic_start.png)

## 3. 如何解决问题

><font size="1">主要记录使用例程中可能会遇到的问题，给出相应的解决方案</font><br />

## 4. 修改历史记录

><font size="1">记录例程的重大修改记录，标明修改发生的版本号 </font><br />



