#!/bin/bash

OPT=$1

build() {
    make -j$(nproc) O=out V=1
}

config() {
    make -j$(nproc) menuconfig O=out
}

clean() {
    make -j$(nproc) clean
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
        *)
            echo "Usage: ./cook.sh [build/config/clean]"
            ;;
    esac
}

main $1
