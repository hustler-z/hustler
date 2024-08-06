# --------------------------------------------------------------------
# Hustler's Projects                                    2024/04/28 SUN
#
#
# --------------------------------------------------------------------

#!/bin/bash

SRC_PATH=$1

usage() {
printf "hustletrack - generate tags for better code reading experience
usage: hustletrack [options]

options:
    -h                      help information
    -p [path-to-source]     path to target source code
"
}

find_sources() {
    find . -name "*.[chS]" -o -name "*.cpp" -o -name "*.hpp" \
        -o -name "Kconfig" -o -name "Makefile" -o -name "*.ld" \
        -o -name "*.mk" -o -name "*.kconfig"
}

code_tracker() {
    if [ -z $SRC_PATH ];then
        usage
        exit
    fi

    echo "----------------------- CODE TRACKER -----------------------"
    cd $SRC_PATH

    # Remove previous tags
    if [ -f GTAGS ];then
        rm -f GTAGS
    fi

    if [ -f GPATH ];then
        rm -f GPATH
    fi

    if [ -f GRTAGS ];then
        rm -f GRTAGS
    fi

    if [ -f tags ];then
        rm -f tags
    fi

    if [ -f cscope* ];then
        rm -f cscope*
    fi

    # Now build new tags

    if [ -z $SRC_PATH ];then
        echo "source code root directory required"
    else
        start=$(date +%s)

        _ctags=$(command -v ctags)
        if [ -z _ctags ];then
            echo "ctags ain't installed yet, apt install universal-ctags"
        else
            ctags --languages=Asm,c,c++,Sh,Make -R
        fi

        _cscope=$(command -v cscope)
        if [ -z _cscope ];then
            echo "cscope ain't installed yet, apt install cscope"
        else
            find_sources > cscope.files
            cscope -q -R -b -i cscope.files
        fi

        _gtags=$(command -v gtags)
        if [ -z _gtags ];then
            echo "gtags ain't installed yet, apt install global"
        else
            find_sources | gtags -i -C "${tree:-.}" -f - "$(pwd)"
        fi

        end=$(date +%s)
        cost=$(($end-$start))
        echo "------------------------------------------------------------"
        echo "code tracker setup in $(($cost/60)) min $(($cost%60)) sec"
    fi
    echo "----------------------- CODE TRACKER -----------------------"
}

while getopts 'p:h' OPT; do
    case $OPT in
        'h')
            usage
            exit
            ;;
        'p')
            SRC_PATH="$OPTARG"
            ;;
        *)
            usage
            exit
            ;;
    esac
done

code_tracker
