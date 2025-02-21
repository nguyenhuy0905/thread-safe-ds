include(CMakeDependentOption)
cmake_dependent_option(tsds_DOCS
  "Whether to generate documents with Doxygen"
  ON "PROJECT_IS_TOP_LEVEL;tsds_DEV" OFF)
if(tsds_DOCS)
  find_package(Doxygen REQUIRED)
  set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/docs")
  configure_file(${PROJECT_SOURCE_DIR}/docs/Doxyfile.in
    ${PROJECT_BINARY_DIR}/docs/Doxyfile
    @ONLY)
  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE}
    ${PROJECT_BINARY_DIR}/docs/Doxyfile
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/docs"
    COMMENT "Build documentation"
    VERBATIM
  )
endif()
