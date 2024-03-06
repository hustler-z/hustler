#!/bin/sh

SRC_PATH=$1

code_tracker() {
    echo "----------------------- CODE TRACKER -----------------------"
    cd $SRC_PATH
    if [ -z $SRC_PATH ];then
        echo "source code root directory required"
    else
        start=$(date +%s)
        _ctags=$(command -v ctags)
        if [ -z _ctags ];then
            echo "ctags ain't installed yet, sudo apt install exuberant-ctags"
        else
            ctags --languages=Asm,c -R
        fi
        _cscope=$(command -v cscope)
        if [ -z _cscope ];then
            echo "cscope ain't installed yet, sudo apt install cscope"
        else
            find . -name "*.c" -o -name "*.cpp" -o -name "*.h" \
                -o -name "*.hpp" -o -name "*.S" > cscope.files
            cscope -q -R -b -i cscope.files
        fi
        end=$(date +%s)
        cost=$(($end-$start))
        echo "code tracker setup in $(($cost/60)) min $(($cost%60)) sec"
    fi
    echo "----------------------- CODE TRACKER -----------------------"
 }

 code_tracker $1
