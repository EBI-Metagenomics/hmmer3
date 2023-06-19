cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(
  h3client
  VERSION 0.12.1
  LANGUAGES C)

include(cmake/warnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/CPM.cmake)

cpmaddpackage(
  GITHUB_REPOSITORY nanomsg/nng
  VERSION 1.5.2
  OPTIONS "NNG_TESTS OFF"
          "NNG_TOOLS OFF"
          "NNG_ENABLE_NNGCAT OFF")
cpmaddpackage("gh:EBI-Metagenomics/lip@0.5.2")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_library(
  h3client
  src/alidisplay.c
  src/amsg.c
  src/answer.c
  src/argless.c
  src/buff.c
  src/deadline.c
  src/dialer.c
  src/domain.c
  src/echo.c
  src/fini.c
  src/hit.c
  src/strerror.c
  src/hmmd/alidisplay.c
  src/hmmd/domain.c
  src/hmmd/hit.c
  src/hmmd/stats.c
  src/hmmd/status.c
  src/hmmd/tophits.c
  src/hmmd/zsetby.c
  src/hmsg.c
  src/itoa.c
  src/msg.c
  src/nnge.c
  src/result.c
  src/stats.c
  src/stream.c
  src/timeout.c
  src/tophits.c
  src/utils.c
  src/zc.c)
add_library(H3CLIENT::h3client ALIAS h3client)

if(NOT TARGET nng::nng)
  add_library(nng::nng ALIAS nng)
endif()

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(h3client PUBLIC -DH3CLIENT_STATIC_DEFINE)
endif()

target_include_directories(
  h3client
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

set_target_properties(h3client PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(h3client PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_compile_options(h3client PRIVATE ${WARNING_FLAGS})

target_link_libraries(h3client PRIVATE LIP::lip)
target_link_libraries(h3client PRIVATE nng::nng)
target_link_libraries(h3client PRIVATE $<$<BOOL:${UNIX}>:m>)

install(
  TARGETS h3client
  EXPORT h3client-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(DIRECTORY include/h3client/ DESTINATION include/h3client)
install(FILES ${CONFIG_FILE} DESTINATION include/h3client)

export(
  TARGETS h3client
  NAMESPACE H3CLIENT::
  FILE h3client-targets.cmake)
install(
  EXPORT h3client-targets
  NAMESPACE H3CLIENT::
  DESTINATION lib/cmake/h3client)

install(FILES ${PROJECT_SOURCE_DIR}/README.md ${PROJECT_SOURCE_DIR}/LICENSE
        DESTINATION share/docs/h3client)

include(CMakePackageConfigHelpers)

set(project_config ${PROJECT_BINARY_DIR}/h3client-config.cmake)
set(version_config ${PROJECT_BINARY_DIR}/h3client-config-version.cmake)

configure_package_config_file(h3client-config.cmake.in ${project_config}
                              INSTALL_DESTINATION lib/cmake/h3client)

write_basic_package_version_file(${version_config}
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${project_config} ${version_config}
        DESTINATION lib/cmake/h3client)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  enable_testing()
  add_subdirectory(tests)
endif()
