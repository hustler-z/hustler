#!/bin/bash

OUT=build
ELF=$OUT/hyperm.img
DISK=$OUT/udisk.img
PLAT=$(uname -m)
KADDR=0x00200000
in_asm=

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
    -h               help information
    -C               set up configuration for hyperm
    -r               remove all built objects
    -b               build all images
    -c               clean built objects
    -q [on/off]      QEMU emulation [on debug mode]
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
        genext2fs -B 1024 -b 24576 -d ./build/disk ./build/disk.img
        echo "---------------------------------------------------------------------"
    fi
}

ubootable() {
    if [ -f "build/hyperm.bin" ];then
        mkimage -A arm64 -O linux -T kernel \
            -C none -a $KADDR -e $KADDR \
            -n "Hyperm VMM" -d build/hyperm.bin build/hyperm.img
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

# foundation_v8() {
#     /bsp/pro/toolchains/armcc-64/bin/aarch64-none-linux-gnu-gcc -nostdlib -nostdinc -e _start -Wl,--build-id=none -Wl,-Ttext=0x80000000 \
#         -DGENTIMER_FREQ=100000000 -DUART_PL011 -DUART_PL011_BASE=0x1c090000 -DGICv2 \
#         -DGIC_DIST_BASE=0x2c001000 -DGIC_CPU_BASE=0x2c002000 -DSPIN_LOOP_ADDR=0x8000fff8 \
#         -DIMAGE=./build/hyperm.bin -DDTB=./build/arch/arm/dts/arm/foundation-v8-gicv2.dtb \
#         -DINITRD=./build/disk.img ./docs/arm/foundation_v8_boot.S -o ./build/foundation_v8_boot.axf
#     ../../../Foundation_Platformpkg/models/Linux64_GCC-9.3/Foundation_Platform --no-gicv3 --no-sve --image ./build/foundation_v8_boot.axf --network=nat
# }

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


build_all_images() {
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
}

main() {
    if [ -z $in_asm ];then
        usage
        exit
    fi

    if [ ! -z $in_asm ];then
        case $in_asm in
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
    fi
}

while getopts 'q:hbCrcf' OPT; do
    case $OPT in
        'b')
            build_all_images
            exit
            ;;
        'C')
            config
            exit
            ;;
        'c')
            clean
            exit
            ;;
        'r')
            cleanall
            exit
            ;;
        'q')
            in_asm=$OPTARG
            ;;
        # 'f')
        #     foundation_v8
        #     exit
        #     ;;
        'h')
            usage
            exit
            ;;
        *)
            usage
            exit
            ;;
    esac
done

main
