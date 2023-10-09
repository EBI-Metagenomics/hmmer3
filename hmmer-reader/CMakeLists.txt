cmake_minimum_required(VERSION 3.20.2 FATAL_ERROR)
project(
  hmmer_reader
  VERSION 0.7.3
  LANGUAGES C)
set(PROJECT_DESCRIPTION "HMMER ASCII file reader.")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

include(cmake/sanitizers.cmake)
include(cmake/warnings.cmake)

add_library(
  hmmer_reader
  src/aux.c
  src/error.c
  src/fsm.c
  src/hmr.c
  src/node.c
  src/prof.c
  src/to.c
  src/tok.c)
add_library(HMMER_READER::hmmer_reader ALIAS hmmer_reader)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(hmmer_reader PUBLIC -DHMMER_READER_STATIC_DEFINE)
endif()

target_include_directories(
  hmmer_reader
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  PRIVATE ${PROJECT_SOURCE_DIR}/src)

set_target_properties(hmmer_reader PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(hmmer_reader PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_compile_options(hmmer_reader PRIVATE ${WARNING_FLAGS})

install(
  TARGETS hmmer_reader
  EXPORT hmmer_reader-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(DIRECTORY include/hmmer_reader/ DESTINATION include/hmmer_reader)

export(
  TARGETS hmmer_reader
  NAMESPACE HMMER_READER::
  FILE hmmer_reader-targets.cmake)
install(
  EXPORT hmmer_reader-targets
  NAMESPACE HMMER_READER::
  DESTINATION lib/cmake/hmmer_reader)

install(FILES ${PROJECT_SOURCE_DIR}/README.md ${PROJECT_SOURCE_DIR}/LICENSE
        DESTINATION share/docs/hmmer_reader)

include(CMakePackageConfigHelpers)

set(project_config ${PROJECT_BINARY_DIR}/hmmer_reader-config.cmake)
set(version_config ${PROJECT_BINARY_DIR}/hmmer_reader-config-version.cmake)

configure_package_config_file(hmmer_reader-config.cmake.in ${project_config}
                              INSTALL_DESTINATION lib/cmake/hmmer_reader)

write_basic_package_version_file(${version_config}
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${project_config} ${version_config}
        DESTINATION lib/cmake/hmmer_reader)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  enable_testing()
  add_subdirectory(test)
endif()
