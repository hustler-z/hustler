#!/bin/bash

OPT=$1

usage() {
printf "usage: ./cook.sh [options]
options:
    build             - build kvmtool
    clean             - clean built objects
"
}

build_kvmtool() {
    make -j$(nproc)
}

build() {
    echo "---------------------------------------------------------------------"
    begin=$(date +%s)
    build_kvmtool
    end=$(date +%s)
    tts=$(($end-$begin))
    echo "---------------------------------------------------------------------"
    echo "Done building kvmtool in $(($tts/60)) min $(($tts%60)) sec"
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

main $1
