include(CMakeDependentOption)
# compilation options and features

add_library(tsds_compile_options INTERFACE)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(tsds_compile_options INTERFACE -Og)
endif()

# ---- feature ----
target_compile_features(tsds_compile_options
  INTERFACE
  cxx_std_${CMAKE_CXX_STANDARD}
)

# ---- warnings ----
cmake_dependent_option(tsds_WARNINGS "Whether to use compiler warnings" ON
  "PROJECT_IS_TOP_LEVEL;tsds_DEV" OFF)
if(tsds_WARNINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tsds_compile_options
      INTERFACE
      -Wall -Wpedantic -Werror -Wextra -Wconversion -Wshadow -Wunused
      -Wsign-conversion -Wcast-qual -Wformat=2 -Wundef -Wnull-dereference
      -Wimplicit-fallthrough -Wnon-virtual-dtor -Wold-style-cast)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(tsds_compile_options
      INTERFACE
    /W4 /permissive- /volatile:iso /Zc:inline /Zc:preprocessor /Zc:enumTypes
    /Zc:lambda /Zc:__cplusplus /Zc:externConstexpr /Zc:throwingNew /EHsc)
  endif()
endif()

# ---- sanitizers ----
cmake_dependent_option(tsds_ASAN "Whether to link with AddressSanitizer" ON
  "PROJECT_IS_TOP_LEVEL;NOT tsds_MSAN;NOT tsds_TSAN;tsds_DEV" OFF)
set(sanitizer_list "")
if(tsds_ASAN)
  list(APPEND sanitizer_list "address")
endif()
# MSan, TSan and ASan are mutually exclusive
# MSan, TSan and UBSan are clang-and-gnu specific
cmake_dependent_option(tsds_MSAN "Whether to link with MemorySanitizer" OFF
  "PROJECT_IS_TOP_LEVEL;NOT tsds_ASAN;NOT tsds_TSAN;tsds_DEV" OFF)
cmake_dependent_option(tsds_TSAN "Whether to link with ThreadSanitizer" OFF
  "PROJECT_IS_TOP_LEVEL;NOT tsds_ASAN;NOT tsds_MSAN;tsds_DEV" OFF)
cmake_dependent_option(tsds_UBSAN "Whether to link with"
  "UndefinedBehaviorSanitizer" ON "PROJECT_IS_TOP_LEVEL;tsds_DEV" OFF)
if(tsds_MSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND sanitizer_list "memory")
  endif()
endif()
if(tsds_TSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND sanitizer_list "thread")
  endif()
endif()
if(tsds_UBSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND sanitizer_list "undefined")
  endif()
endif()
if(tsds_ASAN OR tsds_MSAN OR tsds_TSAN OR tsds_UBSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(JOIN sanitizer_list "," sanitizer_opts)
    target_compile_options(tsds_compile_options
      INTERFACE
      "-fsanitize=${sanitizer_opts}"
    )
    target_link_options(tsds_compile_options
        INTERFACE "-fsanitize=${sanitizer_opts}")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(tsds_compile_options
      INTERFACE
      "/fsanitize=${sanitizer_opts}"
    )
    target_link_options(tsds_compile_options
        INTERFACE "/fsanitize=${sanitizer_opts}")
  endif()
endif()

# ---- ccache ----
option(tsds_CCACHE "Whether to use ccache" OFF)
if(tsds_CCACHE)
  find_program(ccache-prog ccache)

endif()

# ---- PCH ----
option(tsds_PCH "Whether to build with PCH" OFF)
if(tsds_PCH)
  target_precompile_headers(tsds_compile_options
    INTERFACE
    # add some more here, if you need!
    <iostream> <memory> <print>
  )
endif()

# ---- modules ----

cmake_dependent_option(tsds_MODULE "Whether to build using modules" OFF
  "CMAKE_VERSION VERSION_GREATER_EQUAL 3.28;
  CMAKE_CXX_STANDARD GREATER_EQUAL 20;
  CMAKE_GENERATOR STREQUAL Ninja" OFF
)
cmake_dependent_option(tsds_IMPORT_STD "Whether to use import std" OFF
  "CMAKE_CXX_STANDARD GREATER_EQUAL 23" OFF
)
if(tsds_MODULE)
  target_compile_definitions(tsds_compile_options INTERFACE TSDS_MODULE)
  add_library(tsds_lib_module)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # otherwise, nasty GCC module bug.
    # to be honest, even doing the other way doesn't fix it.
  endif()
  if(tsds_IMPORT_STD)
    target_compile_definitions(tsds_compile_options INTERFACE TSDS_IMPORT_STD)
  endif()
endif()

# ---- coverage ----
cmake_dependent_option(tsds_COV "Whether to link with coverage" ON
  "PROJECT_IS_TOP_LEVEL;tsds_DEV" OFF)
if(tsds_COV)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tsds_compile_options INTERFACE --coverage)
    target_link_options(tsds_compile_options INTERFACE --coverage)
    include(cmake/coverage.cmake)
  endif()
endif()
