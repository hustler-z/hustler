<!--
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: usage.md
 * Date: 2022-02-22 16:26:16
 * LastEditTime: 2022-02-22 16:26:16
 * Description:  This file is for c library compile
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
-->

# C库编译方法 

## 1. 概述

- Libc（C标准库）是C语言中最常用的标准库之一。它提供了许多常见的函数和类型，用于字符串处理、文件操作、内存管理、数学运算和系统调用等。

- Newlib 是一个用于嵌入式系统的C标准库（libc）的替代实现。它提供了一套轻量级的标准C库函数，适用于资源有限的嵌入式设备和嵌入式操作系统。与其他标准库实现（如GNU C Library）相比，Newlib 的特点主要体现在以下几个方面：
1. 轻量级：Newlib 的设计目标是保持最小的内存消耗和可执行文件大小。它在功能上比较精简，只包含了标准C库中最常用的函数，因此适用于嵌入式系统中有限的资源环境。
2. 可裁剪性：Newlib 具有很高的可定制性。可以根据具体应用的需求进行裁减，只包含所需的功能，从而进一步减小库的体积。
3. 平台无关性：Newlib 不依赖于特定的硬件平台和操作系统，因此可以很容易地移植到不同的嵌入式系统中。

- 本文档主要介绍基于newlib-4.1.0版本的C库编译方法。

## 2. 功能

- newlib包含常用的 C标准库（libc）和数学库（libm）。

## 3. 配置方法

- 目前在lib\newlib目录下已经生成了aarch32和aarch64版本的libc.a和libm.a，在编译工程时会自动引用对应的库文件。
- 如果有自行编译的需要，请按以下步骤进行编译：
1. 进入example/system/newlib_test例程目录

2. 使用`make menuconfig`配置aarch32/aarch64模式，注意是aarch32，还需要配置-mfloat-abi是使用softfp还是hard
![](./figs/aarch_select.png)
![](./figs/mfloat_abi.png)

3. lib库选择`Using newlib library`
![](./figs/lib_select.png)

4. 执行 `make newlibc.a` 和`make newlibm.a`完成编译，生成的库文件会自动拷贝至lib\newlib的相应目录，具体的命令执行见[c库的编译命令](../../../lib/newlib/makelib.mk)，库文件生成后，再进行应用工程的正常编译即可。

