CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(ComplierFuzzTest)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../cmake/fuzz_test.cmake)

SET(test_progs regex_test)

FOREACH(test_prog ${test_progs})
  ADD_EXECUTABLE(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  TARGET_LINK_LIBRARIES(${test_prog} PRIVATE MyCompilerLib)
  add_fuzz_test(${test_prog})
ENDFOREACH()
