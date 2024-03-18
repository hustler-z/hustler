#!/bin/sh

KPATH=$2
OBJDUMP=

# Change the toolchain path if necessary

envset() {

    Host=$(uname -m)

    echo "--------------------------------------------------------------------------------"
    if [ "$Host" = "x86_64" ];then
        TOOLCHAIN=/os/toolchains/armcc-64/bin/aarch64-none-linux-gnu
        OBJDUMP=$TOOLCHAIN-objdump
        export CROSS_COMPILE=$TOOLCHAIN-
        export ARCH=arm64
    fi

    if [ "$Host" = "aarch64" ];then
        OBJDUMP=objdump
    fi

    echo -n 'Host machine: ' && echo $(uname -m)
    echo "--------------------------------------------------------------------------------"
}

build() {

    # set up compilation evironment
    envset

    case $1 in
        cfg)
            cd $KPATH
            make menuconfig
            echo "------------------------------ Move Configuration ------------------------------"
            if [ -d "out" ];then
                mv .config out/
            else
                mkdir out
                mv .config out/
            fi
            echo "------------------------------ Done Configuration ------------------------------"
            ;;
        out)
            cd $KPATH

            echo "------------------------------- Start Compilation ------------------------------"
            start=$(date +%s)
            make -j$(nproc) O=out/
            end=$(date +%s)
            total=$(($end-$start))
            echo "--------------------------------------------------------------------------------"
            echo "Done compiling kernel in $(($total/60)) min $(($total%60)) sec"
            echo "------------------------------- Done Compilation -------------------------------"
            ;;
        tags)
            cd $KPATH

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
            ;;
        mrp)
            echo "------------------------ Remove Previous Configuration -------------------------"
            cd $KPATH
            start=$(date +%s%N)
            make mrproper
            end=$(date +%s%N)
            echo "mrproper took $(($(($end-$start))/1000000)) ms"
            echo "------------------------------ Done Configuration ------------------------------"
            ;;
        cln)
            echo "----------------------------------- Make Clean ---------------------------------"
            cd $KPATH
            start=$(date +%s)
            make clean
            end=$(date +%s)
            total=$(($end-$start))
            echo "Done cleaning in $(($total/60)) min $(($total%60)) sec"
            echo "----------------------------------- Done Clean ---------------------------------"
            ;;
        dsm)
            echo "---------------------------------- DISASSEMBLE ---------------------------------"
            echo "objdump -r[relocation] -S[source] -l[line NR] -d[disassemble] *.o"
            echo ""
            if [ -d "/debug" ];then
                $OBJDUMP -r -S -l -d $KPATH > /debug/debug.asm
            fi
            echo "---------------------------------- DISASSEMBLE ---------------------------------"
            ;;
        *)
            echo "Usage: ./compilation <option> [path]"
            echo "--------------------------------------------------------------------------------"
            echo "[0] generate tags:      ./compilation.sh tags [path]"
            echo "[1] create .config:     ./compilation.sh cfg  [path]"
            echo "[2] compile the kernel: ./compilation.sh out  [path]"
            echo "[3] mrproper:           ./compilation.sh mrp  [path]"
            echo "[4] clean:              ./compilation.sh cln  [path]"
            echo "[5] disassmble *.o:     ./compilation.sh dsm  [path]"
            echo "[6] clang kernel:       ./compilation.sh llvm [path]"
            echo "--------------------------------------------------------------------------------"
            ;;
    esac
}

build $1 $2
