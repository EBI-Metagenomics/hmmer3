cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(h3result VERSION 0.2.0 LANGUAGES C)

include(cmake/warnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/CPM.cmake)

cpmaddpackage("gh:EBI-Metagenomics/lip@0.5.2")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

file(GLOB H3RESULT_SOURCES "*.c")
file(GLOB H3RESULT_HEADERS "*.h")
list(FILTER H3RESULT_SOURCES EXCLUDE REGEX "test_.*")
list(FILTER H3RESULT_HEADERS EXCLUDE REGEX "test_.*")

add_library(h3result ${H3RESULT_SOURCES})
add_library(H3RESULT::h3result ALIAS h3result)
target_link_libraries(h3result PRIVATE LIP::lip)
target_link_libraries(h3result INTERFACE $<$<BOOL:${UNIX}>:m>)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/h3result)
file(COPY ${H3RESULT_HEADERS} DESTINATION ${PROJECT_BINARY_DIR}/h3result)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(h3result PUBLIC -DH3RESULT_STATIC_DEFINE)
endif()

target_include_directories(
  h3result
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
         $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  PRIVATE ${PROJECT_SOURCE_DIR}
)

set_target_properties(h3result PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(h3result PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

install(
  TARGETS h3result
  EXPORT h3result-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(DIRECTORY ${PROJECT_BINARY_DIR}/h3result DESTINATION include)

export(
  TARGETS h3result
  NAMESPACE H3RESULT::
  FILE h3result-targets.cmake)
install(
  EXPORT h3result-targets
  NAMESPACE H3RESULT::
  DESTINATION lib/cmake/h3result)

install(
  FILES ${PROJECT_SOURCE_DIR}/README.md ${PROJECT_SOURCE_DIR}/LICENSE
  DESTINATION share/docs/h3result)

include(CMakePackageConfigHelpers)
set(PROJECT_CONFIG ${PROJECT_BINARY_DIR}/h3result-config.cmake)
set(VERSION_CONFIG ${PROJECT_BINARY_DIR}/h3result-config-version.cmake)
configure_package_config_file(h3result-config.cmake.in ${PROJECT_CONFIG}
                              INSTALL_DESTINATION lib/cmake/h3result)
write_basic_package_version_file(${VERSION_CONFIG}
                                 COMPATIBILITY SameMajorVersion)
install(FILES ${PROJECT_CONFIG} ${VERSION_CONFIG} DESTINATION lib/cmake/h3result)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  enable_testing()
  file(GLOB H3RESULT_TEST_SOURCES "test_*.c")
  file(COPY h3result.mp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  foreach(source ${H3RESULT_TEST_SOURCES})
    get_filename_component(x ${source} NAME_WE)
    add_executable(${x} ${source})
    target_link_libraries(${x} PRIVATE h3result)
    target_link_libraries(${x} PRIVATE LIP::lip)
    add_test(NAME ${x} COMMAND ${x} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endforeach()
endif()
