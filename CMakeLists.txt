cmake_minimum_required(VERSION 3.19)

project(MyComputationLib LANGUAGES CXX)

include(cmake/all.cmake)

file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(range-v3 REQUIRED)

add_library(MyComputationLib ${SOURCE})

target_link_libraries(MyComputationLib PUBLIC Boost::headers)
target_link_libraries(MyComputationLib PUBLIC range-v3::range-v3)
target_link_libraries(MyComputationLib PRIVATE fmt::fmt)

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
