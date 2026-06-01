#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "MyComputationLib" for configuration "Debug"
set_property(TARGET MyComputationLib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(MyComputationLib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libMyComputationLib.a"
  )

list(APPEND _cmake_import_check_targets MyComputationLib )
list(APPEND _cmake_import_check_files_for_MyComputationLib "${_IMPORT_PREFIX}/lib/libMyComputationLib.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
