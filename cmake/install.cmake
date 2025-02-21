include(GNUInstallDirs)
install(TARGETS tsds_lib
  EXPORT tsdsTargets
  FILE_SET tsds_modules DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tsds
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  # CXX_MODULES_BMI DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tsds
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tsds
)
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
