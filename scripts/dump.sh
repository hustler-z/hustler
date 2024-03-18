#!/bin/bash

ARCH=$(uname -m)
TC_PATH=/os/toolchains/armcc-bm-64/bin/aarch64-none-elf
OPT=$1
EXEC=$2

do_objdump() {
    if [ $ARCH = "x86_64" ];then
        $TC_PATH-objdump -S -r -l -d $EXEC > dump.asm
    fi

    if [ $ARCH = "aarch64" ];then
        objdump -S -r -l -d $EXEC > dump.asm
    fi
}

main() {
    case $OPT in
        objdump)
            do_objdump
            ;;
        *)
            echo "Usage: ./dump.sh objdump/* [executable]"
            ;;
    esac
}

main $1 $2
