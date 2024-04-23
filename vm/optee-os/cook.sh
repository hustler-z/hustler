#!/bin/bash

OPT=$1
PLAT=$2

usage() {
printf "usage: ./cook.sh [options]
options:
    build [PLATFORM] - build optee-os
           rockchip    rk3399
           vexpress    ARM board for QEMU
    clean            - clean built objects
"
}

build_rockchip() {
    make -j$(nproc) CFG_TEE_BENCHMARK=n \
        CFG_TEE_CORE_LOG_LEVEL=3 \
        DEBUG=1 PLATFORM=rockchip \
        PLATFORM_FLAVOR=rk3399
}

build_vexpress() {
    make -j$(nproc) \
        CFG_ARM64_core=y \
        CFG_TEE_BENCHMARK=n \
        CFG_TEE_CORE_LOG_LEVEL=3 \
        CROSS_COMPILE= \
        CROSS_COMPILE_core= \
        CROSS_COMPILE_ta_arm32=/bsp/pro/toolchains/in-use/armcc-lx32-v13.2/bin/arm-none-linux-gnueabihf- \
        CROSS_COMPILE_ta_arm64= \
        DEBUG=1 \
        PLATFORM=vexpress-qemu_armv8a
}

build() {
    # while CONFIG_SYS_TEXT_BASE=0x60000000
    echo "---------------------------------------------------------------------"
    begin=$(date +%s)
    case $PLAT in
        rockchip)
            build_rockchip
            ;;
        vexpress)
            build_vexpress
            ;;
        *)
            usage
            ;;
    esac
    end=$(date +%s)
    tts=$(($end-$begin))
    echo "Done build optee-os in $(($tts/60)) min $(($tts%60)) sec"
    echo "---------------------------------------------------------------------"
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
        *)
            usage
            ;;
    esac
}

main $1 $2
