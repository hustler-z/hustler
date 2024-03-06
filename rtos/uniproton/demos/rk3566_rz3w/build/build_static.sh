PROJPATH=/os/oscope/rtos/uniproton
DEMOPATH=$PROJPATH/demos/$1

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

# UniProton kernel lib
if [ -d $PROJPATH/output/UniProton/lib/$1 ];then
    cp $PROJPATH/output/UniProton/lib/$1/* $DEMOPATH/libs
fi

# UniProton secure c lib
if [ -d $PROJPATH/output/libboundscheck/lib/$1 ];then
    cp $PROJPATH/output/libboundscheck/lib/$1/* $DEMOPATH/libs
fi

# libc
if [ -d $PROJPATH/output/libc ];then
    cp -r $PROJPATH/output/libc $DEMOPATH/include
fi

if [ -d $DEMOPATH/include/libc/include ];then
    rm -rf $DEMOPATH/include/libc
fi

mkdir -p $DEMOPATH/include/libc/include
cp -r $PROJPATH/src/libc/musl/include/* $DEMOPATH/include/libc/include/
cp -r $PROJPATH/src/libc/litelibc/include/bits/* $DEMOPATH/include/libc/include/bits/

cp -r $PROJPATH/src/include/uapi/* $DEMOPATH/include
cp -r $PROJPATH/build/uniproton_config/config_armv8_rk3566_rz3w/prt_buildef.h $DEMOPATH/include/
cd $DEMOPATH
