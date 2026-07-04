cmake_minimum_required(VERSION 3.20)

project(
  async_logger
  VERSION 0.1.0
  LANGUAGES CXX)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Debug
      CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(ASYNC_LOGGER_RING_BUFFER_VERSION 0.1.0)
set(ASYNC_LOGGER_RING_BUFFER_PROVIDER
    "AUTO"
    CACHE STRING "Dependency provider for ring_buffer: FETCH, PACKAGE, or AUTO")
set_property(
  CACHE ASYNC_LOGGER_RING_BUFFER_PROVIDER PROPERTY STRINGS FETCH PACKAGE AUTO)

if(TARGET ring_buffer::ring_buffer)
  message(STATUS "Using caller-provided ring_buffer target: ring_buffer::ring_buffer")
elseif(TARGET ring_buffer)
  message(STATUS "Using caller-provided ring_buffer target: ring_buffer")
elseif(ASYNC_LOGGER_RING_BUFFER_PROVIDER STREQUAL "FETCH")
  message(STATUS "Using ring_buffer provider: FETCH")
  include(FetchContent)
  FetchContent_Declare(
    ring_buffer
    GIT_REPOSITORY https://github.com/HuRuilizhen/ring_buffer.git
    GIT_TAG v${ASYNC_LOGGER_RING_BUFFER_VERSION})
  FetchContent_MakeAvailable(ring_buffer)
elseif(ASYNC_LOGGER_RING_BUFFER_PROVIDER STREQUAL "PACKAGE")
  message(STATUS "Using ring_buffer provider: PACKAGE")
  find_package(ring_buffer ${ASYNC_LOGGER_RING_BUFFER_VERSION} CONFIG REQUIRED)
elseif(ASYNC_LOGGER_RING_BUFFER_PROVIDER STREQUAL "AUTO")
  message(STATUS "Using ring_buffer provider: AUTO")
  find_package(ring_buffer ${ASYNC_LOGGER_RING_BUFFER_VERSION} CONFIG QUIET)

  if(ring_buffer_FOUND)
    message(STATUS "Found local ring_buffer: using ring_buffer::ring_buffer")
  else()
    message(STATUS "ring_buffer package not found; falling back to FetchContent")

    include(FetchContent)
    FetchContent_Declare(
      ring_buffer
      GIT_REPOSITORY https://github.com/HuRuilizhen/ring_buffer.git
      GIT_TAG v${ASYNC_LOGGER_RING_BUFFER_VERSION})
    FetchContent_MakeAvailable(ring_buffer)
  endif()
else()
  message(
    FATAL_ERROR
      "Invalid ASYNC_LOGGER_RING_BUFFER_PROVIDER='${ASYNC_LOGGER_RING_BUFFER_PROVIDER}'. "
      "Expected one of: FETCH, PACKAGE, AUTO.")
endif()

if(TARGET ring_buffer::ring_buffer)
  set(ASYNC_LOGGER_RING_BUFFER_TARGET ring_buffer::ring_buffer)
elseif(TARGET ring_buffer)
  set(ASYNC_LOGGER_RING_BUFFER_TARGET ring_buffer)
else()
  message(FATAL_ERROR "ring_buffer target not found after dependency resolution")
endif()

add_library(async_logger STATIC src/async_logger.cc)
add_library(async_logger::async_logger ALIAS async_logger)
target_include_directories(
  async_logger PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                      $<INSTALL_INTERFACE:include>)
target_link_libraries(async_logger PUBLIC ${ASYNC_LOGGER_RING_BUFFER_TARGET})

option(ENABLE_TESTS "Enable tests" OFF)

if(ENABLE_TESTS)
  message(STATUS "Tests enabled")

  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()

  add_executable(async_logger_basic_test tests/async_logger_basic_test.cc)
  add_executable(async_logger_file_test tests/async_logger_file_test.cc)
  target_link_libraries(async_logger_basic_test PRIVATE async_logger
                                                        GTest::gtest_main)
  target_link_libraries(async_logger_file_test PRIVATE async_logger
                                                       GTest::gtest_main)
  add_test(NAME async_logger_basic_test COMMAND async_logger_basic_test)
  add_test(NAME async_logger_file_test COMMAND async_logger_file_test)
endif(ENABLE_TESTS)

option(ENABLE_EXAMPLE "Enable example" OFF)

if(ENABLE_EXAMPLE)
  message(STATUS "Example enabled")
  add_executable(async_logger_example examples/async_logger_example.cc)
  add_executable(async_logger_macro_example
                 examples/async_logger_macro_example.cc)
  target_compile_options(
    async_logger_example PRIVATE $<$<CONFIG:Debug>:-O0 -g>)
  target_compile_options(
    async_logger_macro_example PRIVATE $<$<CONFIG:Debug>:-O0 -g>)
  target_link_libraries(async_logger_example PRIVATE async_logger)
  target_link_libraries(async_logger_macro_example PRIVATE async_logger)
endif(ENABLE_EXAMPLE)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS async_logger EXPORT async_loggerTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT async_loggerTargets
  FILE async_loggerTargets.cmake
  NAMESPACE async_logger::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/async_logger)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/async_loggerConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/async_loggerConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/async_loggerConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/async_logger)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/async_loggerConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/async_loggerConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/async_logger)

export(
  EXPORT async_loggerTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/async_loggerTargets.cmake
  NAMESPACE async_logger::)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Uninstall.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/Uninstall.cmake @ONLY)

add_custom_target(
  uninstall_async_logger COMMAND ${CMAKE_COMMAND} -P
                                 ${CMAKE_CURRENT_BINARY_DIR}/Uninstall.cmake)
