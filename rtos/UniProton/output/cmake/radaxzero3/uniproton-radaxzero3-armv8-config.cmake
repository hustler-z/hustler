
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was radaxzero3_armv8_config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################
include(${CMAKE_CURRENT_LIST_DIR}/uniproton-radaxzero3-armv8-targets.cmake)



set_and_check(INSTALL_RADAXZERO3_ARMV8_BASE_DIR             "${PACKAGE_PREFIX_DIR}/.")
set_and_check(INSTALL_RADAXZERO3_ARMV8_INCLUDE_DIR          "${PACKAGE_PREFIX_DIR}/UniProton/include")
set_and_check(INSTALL_RADAXZERO3_ARMV8_ARCHIVE_DIR          "${PACKAGE_PREFIX_DIR}/UniProton/lib/radaxzero3")
set_and_check(INSTALL_RADAXZERO3_ARMV8_ARCHIVE_CONFIG_DIR   "${PACKAGE_PREFIX_DIR}/UniProton/config")
set_and_check(INSTALL_RADAXZERO3_ARMV8_CONFIG_DIR           "${PACKAGE_PREFIX_DIR}/cmake/radaxzero3")

check_required_components(uniproton-radaxzero3-armv8)
