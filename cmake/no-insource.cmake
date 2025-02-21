# Prevents in-source builds
function(tsds_no_insource_build)
  get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
  get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)
  if("${srcdir}" STREQUAL "${bindir}")
    message(FATAL_ERROR "In-source builds are not allowed!
    Please create a separate build directory!
    ")
  endif()
endfunction()

tsds_no_insource_build()
