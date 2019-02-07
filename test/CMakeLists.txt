include(../cmake/test.cmake)
find_package(doctest REQUIRED)

file(GLOB test_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp  
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
  )

foreach(test_source IN LISTS ${test_sources})
  get_filename_component(test_prog ${test_source} NAME_WE)
  add_executable(${test_prog} ${test_source})
  target_link_libraries(${test_prog} PRIVATE MyComputationLib)
  target_link_libraries(${test_prog} PRIVATE doctest::doctest)
  add_test_with_runtime_analysis(TARGET ${test_prog})
endforeach()
