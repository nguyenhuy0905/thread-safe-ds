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

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# ---- library ----

# target_sources inside the src/ directory

add_library(tsds_header INTERFACE)
target_compile_features(tsds_header
  INTERFACE cxx_std_${CMAKE_CXX_STANDARD}
)
target_include_directories(tsds_header ${WARNING_GUARD}
  INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
add_library(tsds::tsds_header ALIAS tsds_header)

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
include(cmake/linter.cmake)
