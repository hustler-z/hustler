export TOOLCHAIN_PATH=/bsp/pro/toolchains/in-use/armcc-bm64-v13.2
export TOOLCHAIN_GCC_PATH=/bsp/pro/toolchains/in-use/armcc-bm64-v13.2/bin/aarch64-none-elf-gcc
# export ALL="task-switch task-preempt semaphore-shuffle interrupt-latency deadlock-break message-latency"
export ALL="radaxzero3"

sh ./build_static.sh radaxzero3
sh ./build_openamp.sh $TOOLCHAIN_PATH

pushd ./../../../src/component/lua-5.3.4/src
    make posix
    make echo
    cp ./liblua.a ./../../../../demos/radaxzero3/libs/
    cp ./lua.h ./../../../../demos/radaxzero3/include/
    cp ./luaconf.h ./../../../../demos/radaxzero3/include/
    cp ./lualib.h ./../../../../demos/radaxzero3/include/
    cp ./lauxlib.h ./../../../../demos/radaxzero3/include/
    cp ./lua.hpp ./../../../../demos/radaxzero3/include/

    # 临时拷贝，用于排查lua依赖
    cp ./lprefix.h ./../../../../demos/radaxzero3/include/
    cp ./lua.c ./../../../../demos/radaxzero3/apps/ivshmem/
popd

function build()
{
    export APP=$1
    export TMP_DIR=$APP

    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCPU_TYPE:SRTING="radaxzero3"
    pushd $TMP_DIR
    make $APP
    popd

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
