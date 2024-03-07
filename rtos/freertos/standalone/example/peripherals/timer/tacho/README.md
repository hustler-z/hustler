# TACHO 测试例程

## 1. 例程介绍

><font size="1">介绍例程的用途，使用场景，相关基本概念，描述用户可以使用例程完成哪些工作</font><br />

注：本测试例程使用的tacho id = 12，可进行修改
tacho获取风扇转速测试例程 (tacho_get_fun_rpm_example.c)
- 初始化Tacho,配置为tachometer模式，打开tacho低于转速中断和超转速中断
- 使用示波器输出1KHz方波模拟风扇，接入pwm12_data_in
- 假设风扇每转一周产生两个方波，风扇转速(rpm)理论值应为29999
- 获取计数值与计数周期值，计算可得风扇转速，并打印，可以与理论值比较判断结果是否准确
- 关闭所有中断，去初始化Tacho

tacho脉冲捕获计数测试例程 (tacho_pulse_capture_count_example.c)
- 初始化tacho，配置为capture模式，打开tacho输入捕获中断
- 使用示波器输出1KHz方波，接入pwm12_data_in
- 间隔20ms，40ms，60ms，80ms获取Capture计数值，因接入1kHz方波，理论每次Capture计数值应该相差20
- 结合打印内容，成功触发中断，并对比理论值判断测试是否通过
- 关闭所有中断，去初始化Tacho

## 2. 如何使用例程

><font size="1">描述开发平台准备，使用例程配置，构建和下载镜像的过程</font><br />

本例程需要以下硬件，

- E2000D/Q Demo 板
- 串口线和串口上位机
- 示波器，杜邦线

### 2.1 硬件配置方法

><font size="1">哪些硬件平台是支持的，需要哪些外设，例程与开发板哪些IO口相关等（建议附录开发板照片，展示哪些IO口被引出）</font><br />
- 本测试可使用示波器的方波作为pwm输入，来验证功能的正确性
- 如下图所示，右下角为示波器方波输出端口
 ![oscilloscope](./fig/oscilloscope.jpg)
- 本测试默认使用pwm12_data_in引脚作为输入接口，以E2000D demo开发板为例，对应于J30的引脚5，如下图所示
 ![pwm12_data_in](./fig/pwm12_data_in.jpg)
- 测试前使用杜邦线将两者连接即可
### 2.2 SDK配置方法

><font size="1">依赖哪些驱动、库和第三方组件，如何完成配置（列出需要使能的关键配置项）</font><br />
- Letter Shell组件，依赖 USE_LETTER_SHELL
- Timer组件，依赖CONFIG_USE_TIMER

对应的配置项是，
- Use Timer
- Use timer_tacho

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
$ make load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_tacho
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

#### 2.4.1 tacho获取风扇转速测试例程
```
$ tacho getfunrpm
```
![tacho_getfunrpm_test](./fig/tacho_getfunrpm_test.png)

#### 2.4.2 tacho脉冲捕获计数测试例程
```
$ tacho capture
```
![tacho_capture_test](./fig/tacho_capture_test.png)

## 3. 如何解决问题

><font size="1">主要记录使用例程中可能会遇到的问题，给出相应的解决方案</font><br />

## 4. 修改历史记录

><font size="1">记录例程的重大修改记录，标明修改发生的版本号 </font><br />



