#!/bin/sh

TC_PREFIX=/bsp/pro/toolchains/armcc-bm-64/bin/aarch64-none-elf
ELF=out/hypos.elf
FADDR=
CMD=
OBJ=

usage() {
printf "Quick Debug Tool
usage: ./cook.sh [options]
options:
    -h                   help information
    -c [cmd]             debug command
    -a [address]         debug address
cmd:
    al                   addr2line => locate line number
    bd                   build hypos
    cr                   prepare for code review
    ca                   clean all up
    bs                   build dumpsym
    nm                   symbols lookup
"
}

addr2line() {
    if [ -z $FADDR ];then
        echo "./cook.sh -c a2l -a [address]"
        exit
    fi

    if [ ! -f $ELF ];then
        echo "[error] $ELF ain't exist."
        exit
    fi

    $TC_PREFIX-addr2line -e $ELF $FADDR -f
}

nm() {
    if [ -z $OBJ ];then
        echo "./cook.sh -c nm -t [ELF]"
        exit
    fi

    $TC_PREFIX-nm -pa $OBJ | doc/dumpsym --all-symbols --sort-by-name --sort
}

build_symbols() {
    gcc doc/dumpsym.c -o doc/dumpsym
}

build_hypos() {
    start=$(date +%s)
    make clean && make
    end=$(date +%s)
    cost=$(($end-$start))

    echo ""
    echo "- Built hypos in $(($cost/60)) mins $(($cost%60)) secs -"
}

clean_all() {
    make clean && make remove
}

prep_review() {
    _ctags=$(command -v ctags)

    if [ ! -z $_ctags ];then
        make ctags
    fi

    _gtags=$(command -v gtags)

    if [ ! -z $_gtags ];then
        make gtags
    fi
}

debug() {
    if [ -z $CMD ];then
        usage
        exit
    fi

    case $CMD in
        al)
            addr2line
            ;;
        bd)
            build_hypos
            ;;
        ca)
            clean_all
            ;;
        cr)
            prep_review
            ;;
        bs)
            build_symbols
            ;;
        nm)
            nm
            ;;
        *)
            usage
            exit
            ;;
    esac
}

while getopts 'c:a:t:h' OPT; do
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
        "t")
            OBJ="$OPTARG"
            ;;
        *)
            usage
            exit
            ;;
    esac
done

debug
