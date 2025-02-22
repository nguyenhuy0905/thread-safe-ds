include(cmake/no-insource.cmake)
cmake_minimum_required(VERSION 3.28.0)
project(tsds
    VERSION 0.1.0
    HOMEPAGE_URL "https://example.com"
    DESCRIPTION "Thread-safe Data Structure"
    LANGUAGES CXX)

include(cmake/top-level.cmake)
include(cmake/compile-options.cmake)
include(cmake/folders.cmake)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# ---- library ----

# target_sources inside the src/ directory

include(GenerateExportHeader)
add_library(tsds_lib)
# clean up GCOV files
add_custom_command(TARGET tsds_lib
  COMMENT "Clean up GCOV files"
  PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E rm -rf
  ${PROJECT_BINARY_DIR}/*.gcno
  ${PROJECT_BINARY_DIR}/*.gcda
)
target_compile_features(tsds_lib
  PUBLIC cxx_std_${CMAKE_CXX_STANDARD}
)
generate_export_header(tsds_lib
  EXPORT_FILE_NAME tsds_export.h
  BASE_NAME tsds
)
target_include_directories(tsds_lib ${WARNING_GUARD}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)
add_library(tsds::tsds ALIAS tsds_lib)

include(CMakeDependentOption)
cmake_dependent_option(tsds_DEV "Enable if you plan to work on this project"
    OFF "PROJECT_IS_TOP_LEVEL" OFF
)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
if(tsds_DEV)
  include(cmake/docs.cmake)
endif()
if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install.cmake)
endif()
