CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

ENABLE_TESTING()
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../cmake/all.cmake)

IF (NOT WIN32)
  add_compile_options("-g")
#  add_compile_options("-fsanitize=leak")
#  add_compile_options("-fsanitize=undefined")
#  add_compile_options("-fsanitize=address")
ENDIF()

FIND_PACKAGE(doctest REQUIRED)

SET(test_progs cfg_test)

FOREACH(test_prog ${test_progs})
  ADD_EXECUTABLE(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  TARGET_LINK_LIBRARIES(${test_prog} PRIVATE MyCompilerLib)
  TARGET_LINK_LIBRARIES(${test_prog} PRIVATE doctest::doctest)
  ADD_TEST(NAME ${test_prog} COMMAND ${test_prog})
ENDFOREACH()
ADD_CUSTOM_TARGET(check ALL COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIGURATION> DEPENDS ${test_progs})
