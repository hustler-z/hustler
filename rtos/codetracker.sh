#!/bin/sh

SRC_PATH=$1

find_sources() {
    find . -name "*.[chS]" -o -name "*.cpp" -o -name "*.hpp" \
        -o -name "Kconfig*" -o -name "Makefile" -o -name "*.ld" \
        -o -name "*.mk" -o -name "*.kconfig"
}

code_tracker() {
    echo "----------------------- CODE TRACKER -----------------------"
    cd $SRC_PATH

    if [ -f GTAGS ] || [ -f GPATH ] || [ -f GRTAGS ] || [ -f tags ] || [ -f cscope* ];then
        rm -rf GTAGS GPATH GRTAGS tags cscope*
    fi

    if [ -z $SRC_PATH ];then
        echo "source code root directory required"
    else
        start=$(date +%s)
        _ctags=$(command -v ctags)
        if [ -z _ctags ];then
            echo "ctags ain't installed yet, apt install universal-ctags"
        else
            ctags --languages=Asm,c -R
        fi
        # _cscope=$(command -v cscope)
        # if [ -z _cscope ];then
        #     echo "cscope ain't installed yet, apt install cscope"
        # else
        #     find_sources > cscope.files
        #     cscope -q -R -b -i cscope.files
        # fi
        _gtags=$(command -v gtags)
        if [ -z _gtags ];then
            echo "gtags ain't installed yet, apt install global"
        else
            find_sources | gtags -i -C "${tree:-.}" -f - "$(pwd)"
        fi
        end=$(date +%s)
        cost=$(($end-$start))
        echo "code tracker setup in $(($cost/60)) min $(($cost%60)) sec"
    fi
    echo "----------------------- CODE TRACKER -----------------------"
 }

 code_tracker $1
