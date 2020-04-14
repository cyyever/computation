cmake_minimum_required(VERSION 3.13)

project(MyComputationLib LANGUAGES CXX)

include(cmake/all.cmake)

file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp)

find_package(range-v3 REQUIRED)
find_package(Boost REQUIRED)

add_library(MyComputationLib ${SOURCE})

target_link_libraries(MyComputationLib PUBLIC range-v3)
target_link_libraries(MyComputationLib PUBLIC Boost::headers)
target_compile_definitions(MyComputationLib
                           PUBLIC RANGES_NO_STD_FORWARD_DECLARATIONS)

find_package(MicrosoftGSL REQUIRED)
target_link_libraries(MyComputationLib PUBLIC Microsoft::GSL)
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  find_package(libcxx REQUIRED)
  target_link_libraries(MyComputationLib PUBLIC libcxx::libcxx)
elseif(MSVC)
  find_package(MicrosoftSTL REQUIRED)
  target_link_libraries(MyComputationLib PUBLIC Microsoft::STL)
endif()

# test
option(BUILD_TEST "Build test" ON)
if(BUILD_TEST)
  add_subdirectory(test)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  option(BUILD_FUZZING "Build fuzzing" ON)
else()
  option(BUILD_FUZZING "Build fuzzing" OFF)
endif()
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()

# install lib
install(
  TARGETS MyComputationLib
  EXPORT ${PROJECT_NAME}Targets
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
install(EXPORT ${PROJECT_NAME}Targets DESTINATION lib/cmake/${PROJECT_NAME})
