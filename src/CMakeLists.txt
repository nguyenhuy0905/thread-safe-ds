# ---- library definition ----

add_subdirectory(internal)
target_sources(tsds_lib
  PUBLIC
  FILE_SET CXX_MODULES FILES
  tsds.cpp
  pool_alloc.cpp
)
target_sources(tsds_lib
  PUBLIC FILE_SET HEADERS
  BASE_DIRS ${PROJECT_BINARY_DIR}
  FILES
  ${PROJECT_BINARY_DIR}/tsds_export.h
)
target_link_libraries(tsds_lib
  PRIVATE
  $<BUILD_INTERFACE:tsds_compile_options>
)

# ---- testing ----

if(BUILD_TESTING)
  include(Catch)

  find_package(Catch2 REQUIRED)
  add_executable(tsds_lib_test)
  target_sources(tsds_lib_test
    PRIVATE
    tsds.test.cpp
  )
  target_link_libraries(tsds_lib_test
    PRIVATE
    tsds_lib
    $<BUILD_INTERFACE:tsds_compile_options>
    Catch2::Catch2WithMain
  )
  target_precompile_headers(tsds_lib_test
    PRIVATE
    <catch2/catch_test_macros.hpp>
  )
  catch_discover_tests(tsds_lib_test
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
endif()
