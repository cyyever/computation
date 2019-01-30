SET(test_progs regex_fuzzing)

FOREACH(test_prog ${test_progs})
  ADD_EXECUTABLE(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  TARGET_LINK_LIBRARIES(${test_prog} PRIVATE MyComputationLib)
  add_fuzzing(TARGET ${test_prog})
ENDFOREACH()
