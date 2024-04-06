#!/bin/bash

OPT=$1

# Compilation function

build() {
    echo "======================================="
    make -j$(nproc)
    echo "======================================="
}

clean() {
    echo "======================================="
    make clean
    echo "======================================="
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
            echo "Usage: ./cook.sh build/*"
            ;;
    esac
}

main $1
