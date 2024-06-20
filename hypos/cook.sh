#!/bin/sh

TC_PREFIX=/bsp/pro/toolchains/armcc-bm-64/bin/aarch64-none-elf
ELF=out/hypos.elf
FADDR=
CMD=

usage() {
printf "Quick Debug Tool
usage: ./cook.sh [options]
options:
    -h                   help information
    -c [cmd]             debug command
    -a [address]         debug address
cmd:
    addr2line            addr2line in arm toolchain
    build                build hypos
"
}

addr2line() {
    if [ -z $FADDR ];then
        echo "./cook.sh -c addr2line -a [address]"
        exit
    fi

    if [ ! -f $ELF ];then
        echo "[error] $ELF ain't exist."
        exit
    fi

    $TC_PREFIX-addr2line -e $ELF $FADDR
}

build_hypos() {
    make clean && make
}

debug() {
    if [ -z $CMD ];then
        usage
        exit
    fi

    case $CMD in
        addr2line)
            addr2line
            ;;
        build)
            build_hypos
            ;;
        *)
            usage
            exit
            ;;
    esac
}

while getopts 'c:a:h' OPT; do
    case $OPT in
        'h')
            usage
            exit
            ;;
        'c')
            CMD="$OPTARG"
            ;;
        'a')
            FADDR=$OPTARG
            ;;
        *)
            usage
            exit
            ;;
    esac
done

debug
