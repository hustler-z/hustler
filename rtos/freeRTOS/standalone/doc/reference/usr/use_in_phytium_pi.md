# 飞腾派开发

> 本文主要介绍如何在飞腾派中进行裸机程序的开发和固化

- 飞腾派开发板是一款面向广大工程师和爱好者的开源硬件。主板处理器采用飞腾四核处理器,兼容
ARM v8 指令集,主频最高可达 1.8GHz
- 由于默认系统需要 16G 的空间，推荐使用 32G 的 SD 卡开发裸机程序

## 开发和调试程序

- 在开发和调试程序的阶段可以参考 [Usage](./usage.md) 编译裸机程序，通过网络串口等方式下载到飞腾派的 RAM 中直接运行

## 使用飞腾派裸机镜像

- 下载 [镜像](https://pan.baidu.com/s/1CTP3eWvK-H3-Mb72xs__VQ#list/path=%2F)，提取码 tei2 

- 下载之后解压，使用 sdcard.img-rtos-4g，注意 2G 版本的飞腾派只能使用 2G 版本的镜像 sdcard.img-rtos-2g
- Windows 上使用 balenaEtcher 工具烧入一张 SD 卡（>= 32G），镜像的格式如下图所示，由3个分区组成，前 64MB 是二进制无格式的启动镜像，然后 16G 是 Phytium Pi OS 的根文件系统，格式为 ext4，最后 4G 是裸机文件系统，格式为 fat32，

    ![balena_etcher](./image/balena_etcher_flash.png)

    ```                            
    -----------------------------------------------------------------------------------
    |                 |                                   |                           |
    |  64MB (系统镜像) |   16G (Phytium Pi OS 根文件系统)    |    4G (裸机文件系统)        |
    |     （无格式）    |        （ext4格式）                |     (fat32格式）           |
    ----------------------------------------------------------------------------------
    ```

- 在 linux 系统上，可以使用 dd 命令将镜像写入 SD 卡 (/dev/sdd)

    ```
    sudo dd if=./sdcard.img-rtos-4g of=/dev/sdd bs=1M count=20000 status=progress
    ```

- SD 卡烧入完成之后插入飞腾派 SD 卡槽，重启飞腾派就会自动进入裸机程序，

    ```
    N: runtime setup 
    N: 5: c0000000; 6: c0000000; 7: c0000003; 4: 80000002
    N: wait for scp ready
    N: 5: c0000000; 6: c0000000; 7: c0000003; 4: 80000002
    N: enable scp->ap os chanel interrupt. 
    N: [os chanel] = 1 
    N: enable ap->scp os chanle interrupt. 
    N: [os chanel] = 1 
    N: scmi_core_reset_addr_set. 

    _____    _               _     _                     
    |  __ \  | |             | |   (_)                    
    | |__) | | |__    _   _  | |_   _   _   _   _ __ ___  
    |  ___/  | '_ \  | | | | | __| | | | | | | | '_ ` _ \ 
    | |      | | | | | |_| | | |_  | | | |_| | | | | | | |
    |_|      |_| |_| \ __, | \__|  |_|  \__,_| |_| |_| |_|
                    __/ |                              
                    |___/                               
    ================ Baremetal Letter Shell================
    Build:       Sep  5 2023 13:29:04
    Version:     3.0.5
    Platform:    PhytiumPI  Armv8-aarch64   
    =======================================================

    phytium:/$ 
    ```

## 更新飞腾派裸机程序

- 有两种方式可以更新 SD 卡第三个分区中的裸机程序
- 1. 将 SD 卡插入一台能识别第三个分区的电脑 (Ubuntu 系统能识别，Windows 可能不能识别)，直接进裸机程序复制入 SD 卡
- 2. 可以通过 u-boot 上传裸机程序，然后保存在 SD 卡第三个分区中，注意保存文件的大小 (0xb0000)，要超过 tftpboot 加载的文件大小

    ```
    Phytium-Pi#setenv ipaddr 10.31.94.219;setenv serverip 10.31.94.103;setenv gatewayip 10.31.94.254;  
    Phytium-Pi#tftpboot 0x90100000 baremetal.elf  
    ethernet@3200c000: PHY present at 0
    ethernet@3200c000: Starting autonegotiation...
    ethernet@3200c000: Autonegotiation complete
    ethernet@3200c000: link up, 1000Mbps full-duplex (lpa: 0x6800)
    ft sgmii speed 1000M!
    Using ethernet@3200c000 device
    TFTP from server 10.31.94.103; our IP address is 10.31.94.219
    Filename 'baremetal.elf'.
    Load address: 0x90100000
    Loading: ##############################################
            127.9 KiB/s
    done
    Bytes transferred = 669448 (a3708 hex)
    ```

    ```
    Phytium-Pi#fatls mmc 0:2
            6   hello.txt

    1 file(s), 0 dir(s)
    Phytium-Pi#fatwrite mmc 0:2 0x90100000 standalone-images/baremetal.elf 0xb0000
    65536 bytes written in 29 ms (2.2 MiB/s)
    Phytium-Pi#fatls mmc 0:2
            6   hello.txt
        65536   baremetal.elf

    2 file(s), 0 dir(s)
    ```

## 修改自启动的裸机程序

- 通过在 u-boot 控制台修改 bootcmd，可以指定不同的裸机程序自启动，如下所示，指定启动 `phytiumpi_aarch64_firefly_shell.elf`

    ```
    Phytium-Pi#printenv bootcmd
    bootcmd=mw 0x32b301a8 0x275;mmc dev 0;mmc read 0x90000000 0x2000 0x10000;bootm 0x90000000#phytium
    Phytium-Pi#setenv bootcmd "fatload mmc 0:2 0xa0100000 standalone-images/phytiumpi_aarch64_firefly_shell.elf;dcache flush;bootelf -p 0xa0100000;"
    Phytium-Pi#saveenv
    ```

## 切换成 linux 开发模式

- 本文提供的裸机开发镜像中，有一个 linux 系统，需要的时候可以修改 bootcmd，切换成自启动 linux 系统

    ```
    Phytium-Pi#setenv bootcmd "mw 0x32b301a8 0x275;mmc dev 0;mmc read 0x90000000 0x2000 0x10000;bootm 0x90000000#phytium"
    Phytium-Pi#saveenv
    Saving Environment to MMC... Writing to MMC(0)... OK
    ```

## 在裸机应用中访问 SD 卡分区

- 飞腾派中可以参考[fatfs例程](../../../example/storage/fatfs/README.md)，使用 SD 卡中的分区
