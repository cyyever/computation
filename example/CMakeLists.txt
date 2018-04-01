CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

PROJECT(example)

INCLUDE(../cmake/compiler.cmake)

FIND_PACKAGE(MyCompilerLib REQUIRED)

SET(example_progs cfg_example)

FOREACH(example_prog ${example_progs})
  ADD_EXECUTABLE(${example_prog} ${CMAKE_CURRENT_SOURCE_DIR}/${example_prog}.cpp)
  TARGET_LINK_LIBRARIES(${example_prog} PRIVATE MyCompilerLib)
ENDFOREACH()
