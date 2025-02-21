if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(GCOV_EXE "llvm-cov gcov")
else()
  set(GCOV_EXE "gcov")
endif()
add_custom_target(coverage
  COMMENT "Generate coverage data"
  COMMAND gcovr
  -r ${PROJECT_SOURCE_DIR}
  --html ${PROJECT_BINARY_DIR}/docs/coverage.html
  --gcov-executable ${GCOV_EXE}
)
