# 飞腾派分区镜像制作方法

- 本文介绍如何从飞腾派的 Phytium Pi OS 镜像出发，制作用于 Standalone 应用开发的镜像
- 如下图所示，在飞腾派系统 SD 卡中，默认有两个分区

    ```                            
    ------------------------------------------------------
    |                 |                                   |
    |  64MB (系统镜像) |   16G (Phytium Pi OS 根文件系统)    |
    |     （无格式）    |        （ext4格式）                |
    ------------------------------------------------------
    ```

- 在默认的两个分区之后新建一个分区，大小为 4G，格式化成 FAT32，放置裸机程序

    ```                            
    -----------------------------------------------------------------------------------
    |                 |                                   |                           |
    |  64MB (系统镜像) |   16G (Phytium Pi OS 根文件系统)    |    4G (裸机文件系统)        |
    |     （无格式）    |        （ext4格式）                |     (fat32格式）           |
    ----------------------------------------------------------------------------------
    ```

## 新增分区并格式化

- 下载[Phytium PI OS镜像](https://pan.baidu.com/s/17UV4A3PDzaix9xPeK-gyRg) 提取码：6hur，注意根据板子情况选择 2G 或者 4G 的版本

- 查看飞腾派系统OS支持扩展分区，具体的方法是，检查 u-boot 的 bootargs，主要是看 SD 卡(mmcblk0设备)的设置是否支持扩展分区，如下所示

```
Phytium-Pi#printenv bootargs
bootargs=console=ttyAMA1,115200 earlycon=pl011,0x2800d000 root=/dev/mmcblk0p1 rootfstype=ext4 rootwait rw cma=256m;
```

- 重启进入 Phytium Pi OS，用 fdisk -l 查看分区信息 

```
root@phytiumpi:~# fdisk -l /dev/mmcblk0
Disk /dev/mmcblk0: 119.38 GiB, 128177930240 bytes, 250347520 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0x00000000

Device         Boot  Start      End  Sectors Size Id Type
/dev/mmcblk0p1      131072 33685503 33554432  16G 83 Linux

```

- 用 fdisk 新建分区，如下图所示，从分区 p1 结尾 33685503 开始，如 33685504 创建分区 p2 (开始位置根据实际情况，不要覆盖分区 p1)

```

root@phytiumpi:~# fdisk /dev/mmcblk0

Welcome to fdisk (util-linux 2.36.1).
Changes will remain in memory only, until you decide to write them.
Be careful before using the write command.


Command (m for help): n
Partition type
   p   primary (1 primary, 0 extended, 3 free)
   e   extended (container for logical partitions)
Select (default p): p
Partition number (2-4, default 2): 2
First sector (2048-250347519, default 2048): 33685504
Last sector, +/-sectors or +/-size{K,M,G,T,P} (33685504-250347519, default 250347519): +4G

Created a new partition 2 of type 'Linux' and of size 4 GiB.
Partition #2 contains a vfat signature.

Do you want to remove the signature? [Y]es/[N]o: Y

The signature will be removed by a write command.

Command (m for help): w
The partition table has been altered.
Syncing disks.

root@phytiumpi:~# 

```

- 创建完成，写入新分区后确认一下，然后重启系统后可以看到新建的分区，

```
root@phytiumpi:~# fdisk -l /dev/mmcblk0
Disk /dev/mmcblk0: 119.38 GiB, 128177930240 bytes, 250347520 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0x00000000

Device         Boot    Start      End  Sectors Size Id Type
/dev/mmcblk0p1        131072 33685503 33554432  16G 83 Linux
/dev/mmcblk0p2      33685504 42074111  8388608   4G 83 Linux
```

- 重启后 lsblk 查看分区, p1 分区是给 Phytium PI OS 用的，p2 分区是给 Standalone 程序用的

```
root@phytiumpi:~# lsblk
NAME        MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
mmcblk0     179:0    0 119.4G  0 disk 
├─mmcblk0p1 179:1    0    16G  0 part /
└─mmcblk0p2 179:2    0     4G  0 part 
```

- 按照需要可以把 p2 分区格式化为 fat32 格式，相应的 u-boot 中的启动命令要用 fatload，格式化完成后可以挂载在 Phytium PI OS 上简单测试下是否可读可写

```
root@phytiumpi:~# mkfs.fat -F 32 /dev/mmcblk0p2
mkfs.fat 4.2 (2021-01-31)

root@phytiumpi:~# mkdir -p /mnt/rtos
root@phytiumpi:~# mount /dev/mmcblk0p2 /mnt/rtos
root@phytiumpi:~# echo 'hello' > /mnt/rtos/hello.txt
root@phytiumpi:~# ls /mnt/rtos
hello.txt
root@phytiumpi:~# cat /mnt/rtos/hello.txt 
hello
```

## 将裸机程序固化在新增分区中

- 重启系统，然后停留在 u-boot 控制台，通过网络上传镜像保存到新增的分区中，首先，通过网络将 elf 镜像保存在 RAM 中，如 0x90100000 的位置，板子 ip 地址（10.31.94.219）和上位机的 ip 地址 （10.31.94.103）按实际情况设置，

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

- 然后将上传的文件保存到分区 p2 的文件系统中, 注意保存文件的大小 (0xb0000)，要超过 tftpboot 加载的文件大小，

```
Phytium-Pi#fatls mmc 0:2
6 hello.txt

1 file(s), 0 dir(s)
Phytium-Pi#fatwrite mmc 0:2 0x90100000 standalone-images/baremetal.elf 0xb0000
65536 bytes written in 29 ms (2.2 MiB/s)
Phytium-Pi#fatls mmc 0:2
6 hello.txt
65536 baremetal.elf
2 file(s), 0 dir(s)
```

- 之后就可以加载刚刚保存的文件启动 ELF 镜像, 这时可以在 u-boot 中手动加载 ELF 镜像

```
fatload mmc 0:2 0xa0100000 standalone-images/phytiumpi_aarch64_firefly_shell.elf
dcache flush
bootelf -p 0xa0100000
```

## 将裸机程序设置为开机自启动

- 通过在 u-boot 控制台修改 bootcmd，将刚刚保存在分区 p2 中的 ELF 镜像进行加载

```
Phytium-Pi#printenv bootcmd
bootcmd=mmc dev 0;mmc read 0x90000000 0x2000 0x10000;bootm 0x90000000#phytium
Phytium-Pi#setenv bootcmd "fatload mmc 0:2 0xa0100000 standalone-images/phytiumpi_aarch64_firefly_shell.elf;dcache flush;bootelf -p 0xa0100000;"
Phytium-Pi#printenv bootcmd
bootcmd=fatload mmc 0:2 0xa0100000 standalone-images/phytiumpi_aarch64_firefly_shell.elf;dcache flush;bootelf -p 0xa0100000;
Phytium-Pi#saveenv
Saving Environment to MMC... Writing to MMC(0)... OK
Phytium-Pi#
```

- 设置完成之后，下次重启飞腾派就会自动进入裸机程序

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

_____ _ _ _ 
| __ \ | | | | (_) 
| |__) | | |__ _ _ | |_ _ _ _ _ __ ___ 
| ___/ | '_ \ | | | | | __| | | | | | | | '_ ` _ \ 
| | | | | | | |_| | | |_ | | | |_| | | | | | | |
|_| |_| |_| \ __, | \__| |_| \__,_| |_| |_| |_|
__/ | 
|___/ 
================ Baremetal Letter Shell================
Build: Sep 5 2023 13:29:04
Version: 3.0.5
Platform: PhytiumPI Armv8-aarch64 
=======================================================
phytium:/$ 
```

## 保存 SD 中的内容为镜像文件

- 前面已经把用于裸机开发的镜像制作好了，最后一步是将它保存下来，使用读卡器将 SD 卡插在一台 linux 主机上，然后找到 SD 卡设备，比如，这里找到的设备是 /dev/sdc

```
$ lsblk
sdc      8:32   1  29.1G  0 disk 
├─sdc1   8:33   1    16G  0 part
└─sdc2   8:34   1     4G  0 part
```

- 输出镜像文件，预留 20G 的空间，16G 给 Phytium PI OS 用，4G 给裸机用

```
sudo dd if=/dev/sdc of=./sdcard.img-rtos-4g bs=1M count=20000 status=progress
```