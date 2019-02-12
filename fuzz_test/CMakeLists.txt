file(GLOB test_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
  )

foreach(test_source IN LISTS test_sources)
  get_filename_component(test_prog ${test_source} NAME_WE)
  add_executable(${test_prog} EXCLUDE_FROM_ALL ${test_source} ${CMAKE_CURRENT_SOURCE_DIR}/helper.cpp)
  target_link_libraries(${test_prog} PRIVATE MyComputationLib)
  add_fuzzing(TARGET ${test_prog})
endforeach()
