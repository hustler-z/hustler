# Install script for directory: /bsp/pro/hustler/rtos/uniproton

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/bsp/pro/hustler/rtos/uniproton/output")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/bsp/pro/toolchains/in-use/armcc-bm64-v13.2/bin/aarch64-none-elf-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/bsp/pro/hustler/rtos/uniproton/build/output/tmp/armv8_radaxzero3/platform/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/bsp/pro/hustler/rtos/uniproton/build/output/tmp/armv8_radaxzero3/src/cmake_install.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/radaxzero3/uniproton-radaxzero3-armv8-targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/radaxzero3/uniproton-radaxzero3-armv8-targets.cmake"
         "/bsp/pro/hustler/rtos/uniproton/build/output/tmp/armv8_radaxzero3/CMakeFiles/Export/cmake/radaxzero3/uniproton-radaxzero3-armv8-targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/radaxzero3/uniproton-radaxzero3-armv8-targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/radaxzero3/uniproton-radaxzero3-armv8-targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake/radaxzero3" TYPE FILE FILES "/bsp/pro/hustler/rtos/uniproton/build/output/tmp/armv8_radaxzero3/CMakeFiles/Export/cmake/radaxzero3/uniproton-radaxzero3-armv8-targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake/radaxzero3" TYPE FILE FILES "/bsp/pro/hustler/rtos/uniproton/build/output/tmp/armv8_radaxzero3/CMakeFiles/Export/cmake/radaxzero3/uniproton-radaxzero3-armv8-targets-noconfig.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake/radaxzero3" TYPE FILE FILES "/bsp/pro/hustler/rtos/uniproton/build/output/tmp/armv8_radaxzero3/uniproton-radaxzero3-armv8-config.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/libboundscheck/lib/radaxzero3" TYPE STATIC_LIBRARY FILES "/bsp/pro/hustler/rtos/uniproton/build/output/radaxzero3/armv8/FPGA/libCortexMXsec_c.lib")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/UniProton/lib/radaxzero3" TYPE STATIC_LIBRARY FILES "/bsp/pro/hustler/rtos/uniproton/build/output/radaxzero3/armv8/FPGA/libRADAXZERO3.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/UniProton/config/radaxzero3" TYPE FILE FILES "/bsp/pro/hustler/rtos/uniproton/build/uniproton_config/config_armv8_radaxzero3/prt_buildef.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/UniProton/config" TYPE FILE FILES
    "/bsp/pro/hustler/rtos/uniproton/src/config/prt_config.c"
    "/bsp/pro/hustler/rtos/uniproton/src/config/prt_config_internal.h"
    "/bsp/pro/hustler/rtos/uniproton/src/config/config/prt_config.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/UniProton/include/common" TYPE FILE FILES
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_buildef_common.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_clk.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_cpup.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_err.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_errno.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_event.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_exc.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_fs.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_gdbstub_ext.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_hook.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_hwi.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_idle.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_mem.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_module.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_posix_ext.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_proxy_ext.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_queue.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_sem.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_signal.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_sys.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_task.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_tick.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_timer.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_typedef.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/UniProton/include/hw/board" TYPE FILE FILES
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/hw/armv8/os_cpu_armv8.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/hw/armv8/os_exc_armv8.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/UniProton/include/hw/cpu" TYPE FILE FILES "/bsp/pro/hustler/rtos/uniproton/src/include/posix/semaphore.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/UniProton/include" TYPE FILE FILES
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_errno.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_event.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_exc.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_hook.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_hwi.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_mem.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_module.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_queue.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_sem.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_sys.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_task.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_tick.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_timer.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_typedef.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_cpup.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_err.h"
    "/bsp/pro/hustler/rtos/uniproton/src/include/uapi/prt_signal.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/bsp/pro/hustler/rtos/uniproton/build/output/tmp/armv8_radaxzero3/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
