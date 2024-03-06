PROJPATH=/os/oscope/rtos/UniProton
DEMOPATH=/os/oscope/rtos/UniProton/demos/rk3568_jailhouse

echo "################# git clone libmetal #################"
# pushd ../component
cd $DEMOPATH/component
rm -rf ./libmetal*
git clone https://gitee.com/src-openeuler/libmetal.git
mv ./libmetal/libmetal-2022.10.0.tar.gz .
rm -rf ./libmetal
tar -zxvf libmetal-2022.10.0.tar.gz
mv ./libmetal-2022.10.0 ./libmetal
rm -rf libmetal-2022.10.0.tar.gz
cp UniProton-patch-for-libmetal.patch ./libmetal
cd libmetal
patch -p1 -d . < UniProton-patch-for-libmetal.patch
# popd

echo "################# git clone openamp #################"
# pushd ../component
cd $DEMOPATH/component
rm -rf ./open-amp*
git clone https://gitee.com/src-openeuler/OpenAMP.git
mv ./OpenAMP/openamp-2022.10.1.tar.gz .
rm -rf ./OpenAMP
tar -zxvf openamp-2022.10.1.tar.gz
mv ./openamp-2022.10.1 ./open-amp
rm -rf openamp-2022.10.1.tar.gz
cp UniProton-patch-for-openamp.patch ./open-amp
cd open-amp
patch -p1 -d . < UniProton-patch-for-openamp.patch
# popd

echo "######################### build metal #########################"
# pushd .
cd $DEMOPATH/component/libmetal
# mkdir -p libmetal
# cd libmetal
mkdir -p build
cd build
rm -rf *
# cmake ../../../component/libmetal -DCMAKE_TOOLCHAIN_FILE=../../../component/libmetal/cmake/platforms/uniproton_arm64_gcc.cmake \
#     -DTOOLCHAIN_PATH:STRING=$1 -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF
cmake $DEMOPATH/component/libmetal -DCMAKE_TOOLCHAIN_FILE=$DEMOPATH/component/libmetal/cmake/platforms/uniproton_arm64_gcc.cmake \
    -DTOOLCHAIN_PATH:STRING=$1 -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
	echo "make metal failed!"
	exit 1
fi
# popd

echo "######################### build openamp #########################"
# pushd .
cd $DEMOPATH/component/open-amp
# mkdir -p open-amp
# cd open-amp
mkdir -p build
cd build
rm -rf *
# cmake ../../../component/open-amp -DCMAKE_TOOLCHAIN_FILE=../../../component/open-amp/cmake/platforms/uniproton_arm64_gcc.cmake \
#     -DTOOLCHAIN_PATH:STRING=$1
cmake $DEMOPATH/component/open-amp -DCMAKE_TOOLCHAIN_FILE=$DEMOPATH/component/open-amp/cmake/platforms/uniproton_arm64_gcc.cmake \
    -DTOOLCHAIN_PATH:STRING=$1
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
        echo "make openamp failed!"
        exit 1
fi
# popd
cd $DEMOPATH

cp ./libmetal/output/usr/local/lib/*.a ../libs
cp ./open-amp/output/usr/local/lib/*.a ../libs
