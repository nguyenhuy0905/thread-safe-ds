if(CMAKE_SKIP_INSTALL_RULES)
  return()
endif()
include(GNUInstallDirs)
if(tsds_MODULE)
  set(tsds_INSTALLS tsds_lib_module tsds_header)
else()
  set(tsds_INSTALLS tsds_header)
endif()
install(TARGETS ${tsds_INSTALLS}
  EXPORT tsdsTargets
  FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tsds
  FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tsds
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tsds
)
if(EXISTS "${PROJECT_BINARY_DIR}/tsds_export.h")
  install(FILES
    ${PROJECT_BINARY_DIR}/tsds_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tsds
  )
endif()
install(EXPORT tsdsTargets
  FILE tsdsTargets.cmake
  NAMESPACE tsds::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsds
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/tsdsConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES "${CMAKE_CURRENT_LIST_DIR}/install-config.cmake"
  RENAME "tsdsConfig.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsds
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tsdsConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsds)
