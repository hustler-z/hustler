#!/bin/bash

OPT=$1
DEBUG=$2
OUT=build
ELF=$OUT/vhustle.bin

usage() {
printf "usage: ./cook.sh [options]
options:
    config - set up configuration for vhustle
    build  - build vhustle
    clean  - clean built objects
    debug  - QEMU emulation [on debug mode]
"
}

build() {
    make -j$(nproc) O=$OUT V=1
}

config() {
    make -j$(nproc) menuconfig O=$OUT
}

clean() {
    make -j$(nproc) clean
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
            echo "---------------------------------------------------------------------"
            begin=$(date +%s)
            build
            end=$(date +%s)
            tts=$(($end-$begin))
            echo "---------------------------------------------------------------------"
            echo "Done building vhustle within $(($tts/60)) min $(($tts%60)) sec"
            echo "---------------------------------------------------------------------"
            ;;
        config)
            config
            ;;
        clean)
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
