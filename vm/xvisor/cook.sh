#!/bin/bash

OPT=$1
DEBUG=$2
OUT=build
ELF=$OUT/vmm.bin

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
            echo "Usage: ./cook.sh debug [on/off]"
            ;;
    esac
}

main() {
    case $OPT in
        build)
            echo "======================================"
            build
            echo "======================================"
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
            echo "Usage: ./cook.sh [build/config/clean/debug]"
            ;;
    esac
}

main $1 $2
