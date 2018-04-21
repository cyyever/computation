CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../cmake/test.cmake)

FIND_PACKAGE(doctest REQUIRED)

SET(test_progs string_test automaton_test cfg_test ll_grammar_test regex_test cnf_test)

FOREACH(test_prog ${test_progs})
  ADD_EXECUTABLE(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  TARGET_LINK_LIBRARIES(${test_prog} PRIVATE MyCompilerLib)
  TARGET_LINK_LIBRARIES(${test_prog} PRIVATE doctest::doctest)
  add_test_with_runtime_analysis(TARGET ${test_prog})
ENDFOREACH()
