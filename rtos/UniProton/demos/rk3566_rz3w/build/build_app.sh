export TOOLCHAIN_PATH=/os/toolchains/armcc-bm-64-uniproton
export TOOLCHAIN_GCC_PATH=/os/toolchains/armcc-bm-64-uniproton/bin/aarch64-none-elf-gcc
# export ALL="task-switch task-preempt semaphore-shuffle interrupt-latency deadlock-break message-latency"
export ALL="rk3566_rz3w"

PROJPATH=/os/oscope/rtos/UniProton
export RPROTON_BINARY_DIR=$PROJPATH
DEMOPATH=/os/oscope/rtos/UniProton/demos/rk3566_rz3w
export CPU_TYPE=$ALL

sh ./build_static.sh rk3566_rz3w
sh ./build_openamp.sh $TOOLCHAIN_PATH

cd $PROJPATH/src/component/lua-5.3.4/src
    make posix ARCH=arm64 CROSS_COMPILE=$TOOLCHAIN_PATH/bin/aarch64-none-elf-
    make echo
    cp ./liblua.a ./../../../../demos/rk3566_rz3w/libs/
    cp ./lua.h ./../../../../demos/rk3566_rz3w/include/
    cp ./luaconf.h ./../../../../demos/rk3566_rz3w/include/
    cp ./lualib.h ./../../../../demos/rk3566_rz3w/include/
    cp ./lauxlib.h ./../../../../demos/rk3566_rz3w/include/
    cp ./lua.hpp ./../../../../demos/rk3566_rz3w/include/

    # 临时拷贝，用于排查lua依赖
    cp ./lprefix.h ./../../../../demos/rk3566_rz3w/include/
    cp ./lua.c ./../../../../demos/rk3566_rz3w/apps/ivshmem/
cd $DEMOPATH/build

function build()
{
    export APP=$1
    export TMP_DIR=$APP
    echo "-------------------------- APP PATH: $APP --------------------------"

    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCPU_TYPE:SRTING="rk3566_rz3w"
    cd $TMP_DIR
    make $APP
    cd -

    cp ./$TMP_DIR/$APP $APP.elf

    $TOOLCHAIN_PATH/bin/aarch64-none-elf-objcopy -O binary ./$APP.elf $APP.bin
    $TOOLCHAIN_PATH/bin/aarch64-none-elf-objdump -D ./$APP.elf > $APP.asm
    rm -rf $TMP_DIR
}

if [ "$1" == "all" ] || [ "$1" == "" ]
then
    for i in $ALL
    do
        build $i
    done
elif [ "$1" != "" ]
then
    build $1
fi
