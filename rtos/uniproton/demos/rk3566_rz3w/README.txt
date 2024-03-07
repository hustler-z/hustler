###################### RK3566_RZ3W ######################

$ sh build_app.sh

(a) When build_app.sh failed, execute the command below:
$ sudo dpkg-reconfigure dash

(b) In order to port rk3566_rz3w

Modified files:
        modified:   build/uniproton_ci_lib/globle.py
        modified:   cmake/common/build_auxiliary_script/make_lib_rename_file_type.sh
        modified:   cmake/tool_chain/uniproton_tool_chain.cmake
        modified:   config.xml
        modified:   src/include/uapi/prt_buildef_common.h

Added files:
        build/uniproton_config/config_armv8_rk3566_rz3w/
        cmake/tool_chain/rk3566_rz3w_armv8.cmake
        cmake/tool_chain/rk3566_rz3w_armv8_config.cmake.in
        demos/rk3566_rz3w/

(c) Later work: implementation and testing on radxa zero 3w board
