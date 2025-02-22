# ---- library definition ----

add_subdirectory(internal)
if(tsds_MODULE)
  target_sources(tsds_lib_module
  PUBLIC
  FILE_SET CXX_MODULES FILES
  tsds.cpp
  arena_alloc.cpp
  pool_alloc.cpp
  )
  target_link_libraries(tsds_lib_module
  PRIVATE
  $<BUILD_INTERFACE:tsds_compile_options>
  )
else()
  target_sources(tsds_lib
    INTERFACE FILE_SET HEADERS FILES
    pool_alloc.hpp
  )
endif()

# ---- testing ----

if(BUILD_TESTING)
  include(Catch)

  find_package(Catch2 REQUIRED)
  add_executable(tsds_lib_test)
  # clean up GCOV files
  add_custom_command(TARGET tsds_lib_test
  COMMENT "Clean up GCOV files"
  PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E rm -rf
  ${PROJECT_BINARY_DIR}/*.gcno
  ${PROJECT_BINARY_DIR}/*.gcda
  )
  target_sources(tsds_lib_test
    PRIVATE
    tsds.test.cpp
  )
  if(tsds_MODULE)
    target_link_libraries(tsds_lib_test
    PRIVATE
    tsds_lib_module
    $<BUILD_INTERFACE:tsds_compile_options>
    Catch2::Catch2WithMain
    )
  else()
    target_link_libraries(tsds_lib_test
    PRIVATE
    $<BUILD_INTERFACE:tsds_compile_options>
    Catch2::Catch2WithMain
    )
  endif()
  target_precompile_headers(tsds_lib_test
    PRIVATE
    <catch2/catch_test_macros.hpp>
  )
  catch_discover_tests(tsds_lib_test
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
endif()
