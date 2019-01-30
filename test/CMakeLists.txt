FIND_PACKAGE(doctest REQUIRED)

FILE(GLOB test_sources 
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp  
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
  )

FOREACH(test_source ${test_sources})
  GET_FILENAME_COMPONENT(test_prog ${test_source} NAME_WE)
  ADD_EXECUTABLE(${test_prog} ${test_source})
  TARGET_LINK_LIBRARIES(${test_prog} PRIVATE MyComputationLib)
  TARGET_LINK_LIBRARIES(${test_prog} PRIVATE doctest::doctest)
  add_test_with_runtime_analysis(TARGET ${test_prog})
ENDFOREACH()
