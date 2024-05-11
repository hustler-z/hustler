#!/bin/bash

OPT=$1
DEBUG=$2
OUT=build
ELF=$OUT/hyperm.bin
DISK=$OUT/disk.img
PLAT=$(uname -m)

ccsetup() {
    if [ "$PLAT" == "x86_64" ];then
        export CROSS_COMPILE=/bsp/pro/toolchains/armcc-64/bin/aarch64-none-linux-gnu-
    fi

    echo "---------------------------------------------------------------------"
    echo "Host:    $PLAT"
    echo "Date:    $(date +%D)"
    echo "Tool:    $CROSS_COMPILE"
    echo "---------------------------------------------------------------------"
}

usage() {
printf "usage: ./cook.sh [options]
options:
    config - set up configuration for hyperm
    rmall  - remove all built objects
    build  - build hyperm
    clean  - clean built objects
    debug  - QEMU emulation [on debug mode]
"
}

create_disk_image() {
    mkdir -p ./build/disk/tmp
    mkdir -p ./build/disk/system
    cp -f ./docs/banner/hyperm.txt ./build/disk/system/banner.txt

    mkdir -p ./build/disk/images/arm64/virt-v8
    dtc -q -I dts -O dtb -o ./build/disk/images/arm64/virt-v8-guest.dtb ./tests/arm64/virt-v8/virt-v8-guest.dts
    make -j$(nproc) -C tests/arm64/virt-v8/basic

    if [ -f "build/tests/arm64/virt-v8/basic/firmware.bin" ];then
        echo "Now start building disk image ..."
        echo "---------------------------------------------------------------------"
        cp -f ./build/tests/arm64/virt-v8/basic/firmware.bin ./build/disk/images/arm64/virt-v8/firmware.bin
        cp -f ./tests/arm64/virt-v8/linux/nor_flash.list ./build/disk/images/arm64/virt-v8/nor_flash.list
        cp -f ./tests/arm64/virt-v8/xscript/one_guest_virt-v8.xscript ./build/disk/boot.xscript
        genext2fs -B 1024 -b 32768 -d ./build/disk ./build/disk.img
        echo "---------------------------------------------------------------------"
    fi
}

ubootable() {
    if [ -f "build/hyperm.bin" ];then
        mkimage -A arm64 -O linux -T kernel \
            -C none -a 0x3cebc000 -e 0x3cebc000 \
            -n "Hyperm VMM" -d build/hyperm.bin build/uhyperm.bin
    else
        echo "Build hyperm.bin first!!"
    fi

    echo "---------------------------------------------------------------------"

    if [ -f "build/disk.img" ];then
        mkimage -A arm64 -O linux -T ramdisk \
            -a 0x00000000 -n "Hyperm Ramdisk" \
            -d build/disk.img build/udisk.img
    else
        echo "Build disk.img first!!"
    fi
}

build() {
    make -j$(nproc) O=$OUT V=1
    create_disk_image
    ubootable
}

config() {
    make -j$(nproc) menuconfig O=$OUT V=1
}

clean() {
    make -j$(nproc) clean
}

cleanall() {
    # remove openconf for menuconfig to work
    rm -rf build
    make -j$(nproc) -C tests/arm64/virt-v8/basic clean
    cd tools/openconf && make clean
}

debug() {
    case $DEBUG in
        on)
            qemu-system-aarch64 \
                -machine virt,virtualization=on \
                -cpu cortex-a53 \
                -m 512M \
                -initrd $DISK \
                -kernel $ELF \
                -append 'vmm.bootcmd="vfs mount initrd /;vfs run /boot.xscript;vfs cat /system/banner.txt"' \
                -nographic \
                -d in_asm
            ;;
        off)
            qemu-system-aarch64 \
                -machine virt,virtualization=on \
                -cpu cortex-a53 \
                -m 512M \
                -initrd $DISK \
                -kernel $ELF \
                -append 'vmm.bootcmd="vfs mount initrd /;vfs run /boot.xscript;vfs cat /system/banner.txt"' \
                -nographic
            ;;
        *)
            usage
            ;;
    esac
}

main() {


    case $OPT in
        build)
            # set CROSS_COMPILE toolchains if necessary
            ccsetup
            echo "Start Compiling ..."
            echo "---------------------------------------------------------------------"
            begin=$(date +%s)
            build
            end=$(date +%s)
            tts=$(($end-$begin))
            echo "---------------------------------------------------------------------"
            echo "Done building hyperm within $(($tts/60)) min $(($tts%60)) sec"
            echo "---------------------------------------------------------------------"
            ;;
        config)
            config
            ;;
        clean)
            clean
            ;;
        rmall)
            cleanall
            ;;
        debug)
            debug
            ;;
        *)
            usage
            ;;
    esac
}

main $1 $2
