#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "UniProton::CortexMXsec_c" for configuration ""
set_property(TARGET UniProton::CortexMXsec_c APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(UniProton::CortexMXsec_c PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/libboundscheck/lib/radaxzero3/libCortexMXsec_c.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS UniProton::CortexMXsec_c )
list(APPEND _IMPORT_CHECK_FILES_FOR_UniProton::CortexMXsec_c "${_IMPORT_PREFIX}/libboundscheck/lib/radaxzero3/libCortexMXsec_c.lib" )

# Import target "UniProton::RADAXZERO3" for configuration ""
set_property(TARGET UniProton::RADAXZERO3 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(UniProton::RADAXZERO3 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "ASM;C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/UniProton/lib/radaxzero3/libRADAXZERO3.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS UniProton::RADAXZERO3 )
list(APPEND _IMPORT_CHECK_FILES_FOR_UniProton::RADAXZERO3 "${_IMPORT_PREFIX}/UniProton/lib/radaxzero3/libRADAXZERO3.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
