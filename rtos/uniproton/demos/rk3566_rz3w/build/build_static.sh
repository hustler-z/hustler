PROJPATH=/os/oscope/rtos/uniproton
DEMOPATH=$PROJPATH/demos/$1

git clone https://gitee.com/openeuler/libboundscheck.git
if [ ! -d $PROJPATH/platform/libboundscheck/include ] && [ ! -d $PROJPATH/platform/libboundscheck/src ];then
    mkdir -p $PROJPATH/platform/libboundscheck/include
    mkdir -p $PROJPATH/platform/libboundscheck/src
fi

cp libboundscheck/include/* ../../../platform/libboundscheck/include
cp libboundscheck/include/* ../include
cp libboundscheck/src/* ../../../platform/libboundscheck/src
rm -rf libboundscheck

echo ""
echo "----------------------------------- COMPILE UNIPROTON KERNEL -----------------------------------"
cd $PROJPATH
python3 build.py $1
echo "----------------------------------- COMPILE UNIPROTON KERNEL -----------------------------------"
echo ""

# UniProton kernel lib
if [ -d $PROJPATH/output/UniProton/lib/$1 ];then
    cp $PROJPATH/output/UniProton/lib/$1/* $DEMOPATH/libs
else
    echo "[warning] --------------------------------------------------------------------------------------"
fi

# UniProton secure c lib
if [ -d $PROJPATH/output/libboundscheck/lib/$1 ];then
    cp $PROJPATH/output/libboundscheck/lib/$1/* $DEMOPATH/libs
else
    echo "[warning] --------------------------------------------------------------------------------------"
fi

# libc
if [ -d $PROJPATH/output/libc ];then
    cp -r $PROJPATH/output/libc $DEMOPATH/include
else
    echo "[warning] --------------------------------------------------------------------------------------"
fi

if [ -d $DEMOPATH/include/libc/include ];then
    rm -rf $DEMOPATH/include/libc
else
    echo "[warning] --------------------------------------------------------------------------------------"
fi

mkdir -p $DEMOPATH/include/libc/include
cp -r $PROJPATH/src/libc/musl/include/* $DEMOPATH/include/libc/include/
cp -r $PROJPATH/src/libc/litelibc/include/bits/* $DEMOPATH/include/libc/include/bits/

cp -r $PROJPATH/src/include/uapi/* $DEMOPATH/include
cp -r $PROJPATH/build/uniproton_config/config_armv8_rk3566_rz3w/prt_buildef.h $DEMOPATH/include/
cd $DEMOPATH
