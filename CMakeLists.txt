cmake_minimum_required(VERSION 3.13)

project(MyComputationLib LANGUAGES CXX)

include(cmake/all.cmake)

file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp  ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp)

find_package(MicrosoftGSL REQUIRED)

add_library(MyComputationLib ${SOURCE})

target_link_libraries(MyComputationLib PUBLIC MicrosoftGSL::GSL)

# test
option(BUILD_TEST "Build test" ON)
if(BUILD_TEST)
  add_subdirectory(test)
endif()

option(BUILD_FUZZING "Build fuzzing" OFF)
if(BUILD_FUZZING)
  add_subdirectory(fuzz_test)
endif()

# install lib
install(TARGETS MyComputationLib EXPORT ${PROJECT_NAME}Targets
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
install(EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Config.cmake
  DESTINATION lib/cmake/${PROJECT_NAME})
