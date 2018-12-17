# this registers the build-tree with a global CMake-registry
export(PACKAGE GridTools)

include(CMakePackageConfigHelpers)

# for install tree
set(GRIDTOOLS_MODULE_PATH lib/cmake)
set(GRIDTOOLS_SOURCES_PATH src)
set(GRIDTOOLS_INCLUDE_PATH include)
configure_package_config_file(cmake/GridToolsConfig.cmake.in
  ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/install/GridToolsConfig.cmake
  PATH_VARS GRIDTOOLS_MODULE_PATH GRIDTOOLS_SOURCES_PATH GRIDTOOLS_INCLUDE_PATH
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR})
write_basic_package_version_file(
  ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/install/GridToolsConfigVersion.cmake
  COMPATIBILITY SameMajorVersion )
# for build tree
set(GRIDTOOLS_MODULE_PATH ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build-install/lib/cmake)
set(GRIDTOOLS_SOURCES_PATH ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build-install/src)
set(GRIDTOOLS_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include)
configure_package_config_file(cmake/GridToolsConfig.cmake.in
  ${PROJECT_BINARY_DIR}/GridToolsConfig.cmake
  PATH_VARS GRIDTOOLS_MODULE_PATH GRIDTOOLS_SOURCES_PATH GRIDTOOLS_INCLUDE_PATH
  INSTALL_DESTINATION ${PROJECT_BINARY_DIR})
write_basic_package_version_file(
  ${PROJECT_BINARY_DIR}/GridToolsConfigVersion.cmake
  COMPATIBILITY SameMajorVersion )

install(TARGETS GridTools EXPORT GridToolsTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
if (COMPONENT_GCL)
    install(TARGETS GridToolsGCL EXPORT GridToolsTargets
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      RUNTIME DESTINATION bin
      INCLUDES DESTINATION include
    )
    export(TARGETS GridTools GridToolsGCL
        FILE ${PROJECT_BINARY_DIR}/GridToolsTargets.cmake
        NAMESPACE gridtools::
    )
else()
    export(TARGETS GridTools
        FILE ${PROJECT_BINARY_DIR}/GridToolsTargets.cmake
        NAMESPACE gridtools::
    )
endif()
install(EXPORT GridToolsTargets
  FILE GridToolsTargets.cmake
  NAMESPACE gridtools::
  DESTINATION ${INSTALL_CONFIGDIR}
)

install(DIRECTORY include/gridtools/ DESTINATION include/gridtools)

# Install the GridToolsConfig.cmake and GridToolsConfigVersion.cmake
install(FILES "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/install/GridToolsConfig.cmake"
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/install/GridToolsConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake" COMPONENT dev)

# TODO for a next CMake refactoring: move this to an appropriate location
# Also consider using a separate directory for source files which are installed, e.g. src_public
if(COMPONENT_C_BINDINGS)
    set(BINDINGS_SOURCE_DIR "\${GridTools_SOURCES_PATH}")
    set(BINDINGS_CMAKE_PATH "\${GridTools_MODULE_PATH}")
    set(BINDINGS_LIBRARIES gridtools::GridTools)
    configure_file(cmake/gt_bindings.cmake.in
        ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build-install/lib/cmake/gt_bindings.cmake
        @ONLY)

    set(CMAKE_SOURCES
        "${PROJECT_SOURCE_DIR}/cmake/gt_bindings_generate.cmake"
        "${PROJECT_SOURCE_DIR}/cmake/fortran_helpers.cmake"
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build-install/lib/cmake/gt_bindings.cmake"
        )
    set(CBINDINGS_SOURCES
        "${PROJECT_SOURCE_DIR}/src/c_bindings/generator.cpp"
        "${PROJECT_SOURCE_DIR}/src/c_bindings/generator_main.cpp"
        "${PROJECT_SOURCE_DIR}/src/c_bindings/array_descriptor.f90"
        "${PROJECT_SOURCE_DIR}/src/c_bindings/handle.f90"
        "${PROJECT_SOURCE_DIR}/src/c_bindings/handle.cpp"
        )

    install(FILES ${CMAKE_SOURCES} DESTINATION "lib/cmake")
    install(FILES ${CBINDINGS_SOURCES} DESTINATION "src/c_bindings")

    install(FILES ${CMAKE_SOURCES} DESTINATION "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build-install/lib/cmake")
    install(FILES ${CBINDINGS_SOURCES} DESTINATION "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build-install/src/c_bindings")
endif()

if ( GT_INSTALL_EXAMPLES )
    install(DIRECTORY
            gt_examples
            DESTINATION "${GT_INSTALL_EXAMPLES_PATH}/examples/src"
            FILES_MATCHING REGEX ".*cpp|.*hpp")
endif()
