CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

PROJECT(MyComputationLib LANGUAGES CXX)
INCLUDE(cmake/all.cmake)

FILE(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp  ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp)

FIND_PACKAGE(doctest REQUIRED)

ADD_LIBRARY(MyComputationLib SHARED ${SOURCE})

TARGET_LINK_LIBRARIES(MyComputationLib PRIVATE doctest::doctest)

if(CMAKE_BUILD_TYPE STREQUAL Release)
  TARGET_COMPILE_DEFINITIONS(MyComputationLib PRIVATE DOCTEST_CONFIG_DISABLE)
endif()

# test
option(build_test "Build test" ON)
if(BUILD_TEST)
  add_subdirectory(test)
endif()

option(BUILD_FUZZING "Build fuzzing" OFF)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()

# install lib
INSTALL(TARGETS MyComputationLib EXPORT ${PROJECT_NAME}Targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  COMPONENT library
  INCLUDES DESTINATION include
  )

foreach(header_dir . contex_free_lang formal_grammar lang regular_lang)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/${header_dir} DESTINATION include/cyy/computation/
    FILES_MATCHING PATTERN "*.hpp")
endforeach()

# install cmake config file
INSTALL(EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Config.cmake
  DESTINATION lib/cmake/${PROJECT_NAME})
