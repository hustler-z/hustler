#!/bin/bash

OPT=$1
ELF=out/vhustler.bin
DEBUG=$2
# Compilation function

build() {
    echo "======================================="
    make -j$(nproc)
    echo "======================================="
}

clean() {
    echo "======================================="
    make clean
    echo "Done cleaning built objects"
    echo "======================================="
}

qemu_dbg() {
    echo "============= QEMU DEBUG =============="
    case $DEBUG in
        on)
            # show target assembly code for each compiled TB
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
            build
            ;;
        clean)
            clean
            ;;
        debug)
            qemu_dbg
            ;;
        *)
            echo "Usage: ./cook.sh build/clean/debug"
            ;;
    esac
}

main $1 $2
