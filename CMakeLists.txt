cmake_minimum_required(VERSION 3.19)

project(MyComputationLib LANGUAGES CXX)

include(cmake/all.cmake)

file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp)

find_package(Boost REQUIRED)
if (NOT MSVC)
  find_package(fmt REQUIRED)
endif()
find_package(spdlog REQUIRED)
find_package(CyyNaiveLib REQUIRED)
find_package(CyyAlgorithmLib REQUIRED)

add_library(MyComputationLib ${SOURCE})

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  find_package(libcxx REQUIRED)
  target_link_libraries(MyComputationLib PRIVATE libcxx::libcxx)
  target_compile_definitions(MyComputationLib PUBLIC _LIBCPP_ENABLE_EXPERIMENTAL)
endif()

target_link_libraries(MyComputationLib PUBLIC Boost::headers)
if (NOT MSVC)
  target_link_libraries(MyComputationLib PRIVATE fmt::fmt)
endif()
target_link_libraries(MyComputationLib PUBLIC CyyAlgorithmLib)

target_include_directories(
  MyComputationLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                          $<INSTALL_INTERFACE:include/cyy/computation>)

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
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib COMPONENT library
  INCLUDES
  DESTINATION include)

install(
  DIRECTORY src/
  DESTINATION include/cyy/computation
  FILES_MATCHING
  PATTERN "*.hpp")

# install cmake config file
install(EXPORT MyComputationLibConfig DESTINATION lib/cmake/MyComputationLib)
