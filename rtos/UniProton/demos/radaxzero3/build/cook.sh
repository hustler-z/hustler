#!/bin/bash

OPT=$1

usage() {
printf "usage: ./cook.sh [options]
options:
    build    -    build the project
    clean    -    clean the built objects
"
}

build() {
    echo "---------------------------------------------------------------------"
    begin=$(date +%s)
    ./build_app.sh
    end=$(date +%s)
    tts=$(($end-$begin))
    echo "---------------------------------------------------------------------"
    echo "Done building uniproton within $(($tts/60)) min $(($tts%60)) sec"
    echo "---------------------------------------------------------------------"
}

clean() {
    # remove built libraries
    rm -rf ../libs/*
    # remove include files
    rm -rf ../include/*
    # remove built components
    rm -rf ../component/libmetal ../component/open-amp ../component/*.tar.gz
    # remove built objects
    rm -rf ../build/libmetal ../build/open-amp ../build/*.asm ../build/*.elf ../build/*.bin
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
            usage
            ;;
    esac
}

main $1
