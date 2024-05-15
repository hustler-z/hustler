#!/bin/sh

PLAT=$(uname -m)

cmd=""

usage() {
printf "To build syspeek (a toolchain to analyze linux system)
usage:              ./cook.sh [options]
options:
    -h              help information
    -c [cmds]       commands to configure syspeek toolchain
cmds:
    setup           setup bpf program compilation environment
    build           build bpf program
    unset           delete bpftool and libbpf directories
"
}

bpf_env_setup() {
    apt update
	apt-get install -y --no-install-recommends \
        libelf1 libelf-dev zlib1g-dev \
        make clang llvm

    if [ ! -d "bpftool/.git" ];then
        if [ -d "bpftool" ];then
            rm -rf bpftool
        fi
        git clone https://github.com/libbpf/bpftool.git
    fi

    if [ ! -d "libbpf/.git" ];then
        if [ -d "libbpf" ];then
            rm -rf libbpf
        fi
        git clone https://github.com/libbpf/libbpf.git
    fi

    if [ ! -d "bpftool/libbpf/.git" ];then
        cp -rf libbpf bpftool/
    fi
}

bpf_build() {
    make build
}

bpf_clean() {
    make clean
}

bpf_clean_env() {
    rm -rf bpftool
    rm -rf libbpf
}

main() {
    if [ -z $cmd ];then
        usage
        exit
    fi

    case $cmd in
        setup)
            bpf_env_setup
            ;;
        build)
            bpf_build
            ;;
        unset)
            bpf_clean_env
            ;;
        clean)
            bpf_clean
            ;;
        *)
            usage
            exit
            ;;
    esac
}

while getopts 'c:h' OPT; do
    case $OPT in
        'h')
            usage
            exit
            ;;
        'c')
            cmd="$OPTARG"
            ;;
        *)
            usage
            exit
            ;;
    esac
done

main
