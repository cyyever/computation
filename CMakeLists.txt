cmake_minimum_required(VERSION 4.3.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES
    "1942b4fa-b2c5-4546-9385-83f254070067")
set(DISABLE_RUNTIME_ANALYSIS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(MyComputationLib LANGUAGES CXX)
include(cmake/all.cmake)

file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.hpp
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)

find_package(Boost CONFIG REQUIRED)
find_package(CyyAlgorithmLib REQUIRED)
find_package(CyyNaiveLib)

add_library(MyComputationLib ${SOURCE})

target_link_libraries(MyComputationLib PUBLIC Boost::headers)
target_link_libraries(MyComputationLib PUBLIC CyyAlgorithmLib)

target_sources(
  MyComputationLib
  PUBLIC FILE_SET
         public_headers
         TYPE
         HEADERS
         BASE_DIRS
         src
         FILES
         ${headers})

# test
add_subdirectory(test)

option(BUILD_FUZZING "Build fuzzing" OFF)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()

# install lib
install(
  TARGETS MyComputationLib
  EXPORT MyComputationLibConfig
  FILE_SET public_headers
  DESTINATION include/cyy/computation)

# install cmake config file
install(EXPORT MyComputationLibConfig DESTINATION lib/cmake/MyComputationLib EXPORT_PACKAGE_DEPENDENCIES)
