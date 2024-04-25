#!/bin/bash

OPT=$1

usage() {
printf "usage: ./cook.sh [options]
options:
    config            - configuration for LiteOS
    build             - build LiteOS
    clean             - clean built objects
"
}

build_liteos() {
    make -j$(nproc)
}

build() {
    echo "---------------------------------------------------------------------"
    begin=$(date +%s)
    build_liteos
    end=$(date +%s)
    tts=$(($end-$begin))
    echo "---------------------------------------------------------------------"
    echo "Done building LiteOS in $(($tts/60)) min $(($tts%60)) sec"
    echo "---------------------------------------------------------------------"
}

config() {
    make -j$(nproc) menuconfig
}

clean() {
    echo "---------------------------------------------------------------------"
    make clean
    echo "---------------------------------------------------------------------"
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
