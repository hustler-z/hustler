#!/bin/bash

OPT=$1
PLAT=$2

usage() {
printf "usage: ./cook.sh [options]
options:
    config            - set up configuration of uboot
    build [platforms] - build uboot
            qemu        uboot for QEMU
            sandbox     uboot with trace enabled
    clean             - clean built objects
"
}

build_qemu() {
    make -j$(nproc) qemu_arm64_defconfig
    make -j$(nproc)
}

build_sandbox() {
    make -j$(nproc) FTRACE=1 sandbox64_defconfig
    make -j$(nproc) FTRACE=1
}

build() {
    # while CONFIG_SYS_TEXT_BASE=0x60000000
    echo "---------------------------------------------------------------------"
    begin=$(date +%s)
    case $PLAT in
        qemu)
            build_qemu
            ;;
        sandbox)
            build_sandbox
            ;;
        *)
            usage
            echo "---------------------------------------------------------------------"
            exit
            ;;
    esac
    end=$(date +%s)
    tts=$(($end-$begin))
    echo "Done build uboot in $(($tts/60)) min $(($tts%60)) sec"
    echo "---------------------------------------------------------------------"
}

config() {
    make -j$(nproc) menuconfig
}

clean() {
    make -j$(nproc) clean
}

main() {
    case $OPT in
        build)
            build
            ;;
        clean)
            clean
            ;;
        config)
            config
            ;;
        *)
            usage
            ;;
    esac
}

main $1 $2
