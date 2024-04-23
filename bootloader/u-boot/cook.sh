#!/bin/bash

OPT=$1

usage() {
printf "usage: ./cook.sh [options]
options:
    config - set up configuration of uboot
    build  - build uboot
    clean  - clean built objects
"
}

build() {
    # while CONFIG_SYS_TEXT_BASE=0x60000000
    echo "---------------------------------------------------------------------"
    begin=$(date +%s)
    make -j$(nproc) qemu_arm64_defconfig
    make -j$(nproc)
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

main $1
