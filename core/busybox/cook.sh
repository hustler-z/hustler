JOBS=-j$(nproc)
PWD=$(pwd)
cmd=

usage() {
printf "usage: ./cook.sh [options]

options:
    -h            help information
    -c [cmds]     execution

       config     set up configuration
       build      build busybox
       install    create filesystem skeleton
       ext4       create ext4 filesystem
       clean      clean built objects
"
}

build() {
    if [ -z $cmd ];then
        usage
        exit
    fi

    case $cmd in
        build)
            make $JOBS
            ;;
        config)
            make $JOBS menuconfig
            ;;
        install)
            make $JOBS install
            if [ -d "_install" ];then
                cp -rf etc _install/
                mkdir -p _install/proc _install/dev _install/sys _install/boot \
                    _install/tmp _install/hustler
            fi
            ;;
        clean)
            make $JOBS clean
            if [ -f "rootfs.ext4" ];then
                rm rootfs.ext4
            fi
            ;;
        ext4)
            echo "------------------------------------------------------------------"
            echo "[0] Create a empty rootfs.ext4"
            dd if=/dev/zero of=./rootfs.ext4 bs=1M count=52
            mkfs.ext4 rootfs.ext4
            echo "[1] Mount rootfs.ext4 on $PWD/fs"
            if [ ! -d "fs" ];then
                mkdir fs
            fi
            mount -o loop rootfs.ext4 ./fs
            echo "[2] Copy data to target rootfs"
            cp -rf ./_install/* ./fs/
            umount ./fs
            echo "[3] Done"
            echo "------------------------------------------------------------------"
            ;;
        *)
            ;;
    esac
}

while getopts 'c:h' OPT; do
    case $OPT in
        h)
            usage
            exit
            ;;
        c)
            cmd="$OPTARG"
            ;;
        *)
            usage
            exit
            ;;
    esac
done

build $1
