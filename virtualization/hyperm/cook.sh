#!/bin/bash

OPT=$1
DEBUG=$2
OUT=build
ELF=$OUT/hyperm.bin
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
    build  - build hyperm
    clean  - clean built objects
    debug  - QEMU emulation [on debug mode]
"
}

build() {
    make -j$(nproc) O=$OUT V=1
}

config() {
    make -j$(nproc) menuconfig O=$OUT V=1
}

clean() {
    make -j$(nproc) clean
    rm -rf build/*
    # also remove openconf for menuconfig to work
    cd tools/openconf && make clean
}

debug() {
    case $DEBUG in
        on)
            qemu-system-aarch64 \
                -machine virt,virtualization=on \
                -cpu cortex-a72 \
                -nographic \
                -kernel $ELF \
                -d in_asm,guest_errors,int
            ;;
        off)
            qemu-system-aarch64 \
                -machine virt,virtualization=on \
                -cpu cortex-a72 \
                -nographic \
                -kernel $ELF \
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
            ccsetup
            config
            ;;
        clean)
            ccsetup
            clean
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
