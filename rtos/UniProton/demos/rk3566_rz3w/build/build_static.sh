PROJPATH=/os/oscope/rtos/UniProton
DEMOPATH=/os/oscope/rtos/UniProton/demos/rk3568_jailhouse

git clone https://gitee.com/openeuler/libboundscheck.git

cp libboundscheck/include/* ../../../platform/libboundscheck/include
cp libboundscheck/include/* ../include
cp libboundscheck/src/* ../../../platform/libboundscheck/src
rm -rf libboundscheck

# pushd ./../../../
cd $PROJPATH
# python build.py $1
python3 build.py $1
cp output/UniProton/lib/$1/* demos/$1/libs
cp output/libboundscheck/lib/$1/* demos/$1/libs
cp -r output/libc demos/$1/include
cp -r src/include/uapi/* demos/$1/include
cp -r build/uniproton_config/config_armv8_rk3568_jailhouse/prt_buildef.h demos/$1/include/
# popd
cd $DEMOPATH
