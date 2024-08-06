#!/bin/sh

# -----------------------------------------------------------------------------------------
# Hustler's Project
# -----------------------------------------------------------------------------------------

KPATH=""
TLPATH=""
CMD=""

usage() {
printf "<Hustler>  Build Linux Kernel Images
usage:   ./kernel.sh [options]
options:
    -h                      help information
    -p [path-to-kernel]     path to target kernel source code
    -t [path-to-toolchain]  optional, can modify default CC
    -c [cmds]
cmds:
    build                   build the target kernel
    clean                   clean built kernel objects
    config                  kernel menuconfig
    mrproper                remove previous configuration
    tags                    tags for better code reading
"
}

envset() {
    Host=$(uname -m)

    if [ "$Host" = "x86_64" ];then
        export CROSS_COMPILE=$TLPATH/aarch64-none-linux-gnu-
        export ARCH=arm64
    fi

    echo -n 'Host machine: ' && echo $Host
}

remove_prev_config() {
    echo "------------------------ Remove Previous Configuration -------------------------"
    start=$(date +%s%N)
    make mrproper
    end=$(date +%s%N)
    echo "mrproper took $(($(($end-$start))/1000000)) ms"
    echo "------------------------------ Done Configuration ------------------------------"
}

clean_built_objects() {
    echo "----------------------------------- Make Clean ---------------------------------"
    start=$(date +%s)
    make clean
    end=$(date +%s)
    total=$(($end-$start))
    echo "Done cleaning in $(($total/60)) min $(($total%60)) sec"
    echo "----------------------------------- Done Clean ---------------------------------"
}


build_kernel() {
    echo "------------------------------- Start Compilation ------------------------------"
    start=$(date +%s)
    make -j$(nproc) O=out/
    end=$(date +%s)
    total=$(($end-$start))
    echo "--------------------------------------------------------------------------------"
    echo "Done compiling kernel in $(($total/60)) min $(($total%60)) sec"
    echo "------------------------------- Done Compilation -------------------------------"
}

kernel_track_tags() {
    echo "--------------------------- Start Generating tags ------------------------------"
    start=$(date +%s)
    _ctags=$(command -v ctags)
    if [ -z $_ctags ];then
        echo "ctags ain't installed yet, [sudo] apt install universal-ctags"
        TAGS=
    else
        TAGS=tags
    fi
    # Current drop cscope
    _cscope=$(command -v cscope)
    if [ -z $_cscope ];then
        echo "cscope ain't installed yet, [sudo] apt install cscope"
        CSCOPE=
    else
        # CSCOPE=cscope
        CSCOPE=
    fi

    _gtags=$(command -v gtags)
    if [ -z $_gtags ];then
        echo "gtags ain't installed yet, [sudo] apt install global && pip3 install pygments"
        GTAGS=
    else
        GTAGS=gtags
    fi

    echo "--------------------------------------------------------------------------------"
    make -j$(nproc) $TAGS $CSCOPE $GTAGS
    end=$(date +%s)
    total=$(($end-$start))
    echo "--------------------------------------------------------------------------------"
    echo "Done generating tags in $(($total/60)) min $(($total%60)) sec"
    echo "--------------------------------------------------------------------------------"
}

kernel_menuconfig() {
    make menuconfig
    echo "------------------------------ Move Configuration ------------------------------"
    if [ -d "out" ];then
        mv .config out/
    else
        mkdir out
        mv .config out/
    fi
    echo "------------------------------ Done Configuration ------------------------------"
}

build() {
    if [ -z $KPATH ];then
        echo "[FATAL] Kernel path must needed (./kernel.sh -h for more information)"
        exit
    fi

    if [ ! -d "$KPATH/kernel" ];then
        echo "[FATAL] Invalid kernel path, make sure the target is kernel source"
        exit
    fi

    if [ -z $CMD ];then
        usage
        exit
    fi

    envset
    cd $KPATH

    case $CMD in
        build)
            build_kernel
            ;;
        config)
            kernel_menuconfig
            ;;
        tags)
            kernel_track_tags
            ;;
        clean)
            clean_built_objects
            ;;
        mrproper)
            remove_prev_config
            ;;
        *)
            usage
            exit
            ;;
    esac
}

while getopts 'p:c:t:h' OPT;do
    case $OPT in
        'h')
            usage
            exit
            ;;
        'p')
            KPATH="$OPTARG"
            ;;
        'c')
            CMD="$OPTARG"
            ;;
        't')
            TLPATH="$OPTARG"
            ;;
        *)
            usage
            exit
            ;;
    esac
done

build
