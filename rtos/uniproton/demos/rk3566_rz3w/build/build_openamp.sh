PROJPATH=/os/oscope/rtos/uniproton
DEMOPATH=/os/oscope/rtos/uniproton/demos/rk3566_rz3w

if [ ! -d $DEMOPATH/component/libmetal ];then
    echo "################# git clone libmetal #################"
    cd $DEMOPATH/component
    git clone https://gitee.com/src-openeuler/libmetal.git
    mv ./libmetal/libmetal-2022.10.0.tar.gz .
    rm -rf ./libmetal
    tar -zxvf libmetal-2022.10.0.tar.gz
    mv ./libmetal-2022.10.0 ./libmetal
    rm -rf libmetal-2022.10.0.tar.gz
    cp UniProton-patch-for-libmetal.patch ./libmetal
    cd libmetal
    patch -p1 -d . < UniProton-patch-for-libmetal.patch
fi

if [ ! -d $DEMOPATH/component/openamp ];then
    echo "################# git clone openamp #################"
    cd $DEMOPATH/component
    git clone https://gitee.com/src-openeuler/OpenAMP.git
    mv ./OpenAMP/openamp-2022.10.1.tar.gz .
    rm -rf ./OpenAMP
    tar -zxvf openamp-2022.10.1.tar.gz
    mv ./openamp-2022.10.1 ./openamp
    rm -rf openamp-2022.10.1.tar.gz
    cp UniProton-patch-for-openamp.patch ./openamp
    cd openamp
    patch -p1 -d . < UniProton-patch-for-openamp.patch
fi

echo "######################### build metal #########################"
cd $DEMOPATH/component/libmetal
mkdir -p build
cd build
rm -rf *
cmake $DEMOPATH/component/libmetal -DCMAKE_TOOLCHAIN_FILE=$DEMOPATH/component/libmetal/cmake/platforms/uniproton_arm64_gcc.cmake \
    -DTOOLCHAIN_PATH:STRING=$1 -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
	echo "make metal failed!"
	exit 1
fi

echo "######################### build openamp #########################"
cd $DEMOPATH/component/openamp
mkdir -p build
cd build
rm -rf *
cmake $DEMOPATH/component/openamp -DCMAKE_TOOLCHAIN_FILE=$DEMOPATH/component/openamp/cmake/platforms/uniproton_arm64_gcc.cmake \
    -DTOOLCHAIN_PATH:STRING=$1
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
    echo "make openamp failed!"
    exit 1
fi

cd $DEMOPATH/component

cp ./libmetal/output/usr/local/lib/*.a ../libs
cp ./openamp/output/usr/local/lib/*.a ../libs
