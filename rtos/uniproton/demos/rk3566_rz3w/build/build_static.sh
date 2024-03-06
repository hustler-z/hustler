PROJPATH=/os/oscope/rtos/UniProton
DEMOPATH=/os/oscope/rtos/UniProton/demos/rk3566_rz3w

if [ ! -d $PROJPATH/platform/libboundscheck ];then
    git clone https://gitee.com/openeuler/libboundscheck.git
    mkdir -p $PROJPATH/platform/libboundscheck/include
    mkdir -p $PROJPATH/platform/libboundscheck/src
    cp libboundscheck/include/* ../../../platform/libboundscheck/include
    cp libboundscheck/include/* ../include
    cp libboundscheck/src/* ../../../platform/libboundscheck/src
    rm -rf libboundscheck
fi

echo ""
echo "----------------------------------- COMPILE UNIPROTON KERNEL -----------------------------------"
cd $PROJPATH
python3 build.py $1
echo "----------------------------------- COMPILE UNIPROTON KERNEL -----------------------------------"
echo ""

# cp output/UniProton/lib/$1/* demos/$1/libs
# cp output/libboundscheck/lib/$1/* demos/$1/libs
# cp -r output/libc demos/$1/include
# cp -r src/include/uapi/* demos/$1/include

if [ -d $DEMOPATH/include/libc/include ];then
    rm -rf $DEMOPATH/include/libc
fi

mkdir -p $DEMOPATH/include/libc/include
cp -r $PROJPATH/src/libc/musl/include/* $DEMOPATH/include/libc/include/
cp -r $PROJPATH/src/libc/litelibc/include/bits/* $DEMOPATH/include/libc/include/bits/

cp -r src/include/uapi/* demos/$1/include
cp -r build/uniproton_config/config_armv8_rk3566_rz3w/prt_buildef.h demos/$1/include/
cd $DEMOPATH
