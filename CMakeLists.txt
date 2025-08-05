cmake_minimum_required(VERSION 3.15)

project(
  async_logger
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(ring_buffer QUIET)

if(ring_buffer_FOUND)
  message(STATUS "Found local ring_buffer: using ring_buffer::ring_buffer")
else()
  message(STATUS "ring_buffer not found locally; fetching via FetchContent")

  include(FetchContent)
  FetchContent_Declare(
    ring_buffer
    GIT_REPOSITORY https://github.com/HuRuilizhen/ring_buffer.git
    GIT_TAG v1.0.1)
  FetchContent_MakeAvailable(ring_buffer)
endif()

add_library(async_logger STATIC src/async_logger.cc)
target_include_directories(
  async_logger PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                      $<INSTALL_INTERFACE:include>)
target_link_libraries(
  async_logger PUBLIC $<$<BOOL:${ring_buffer_FOUND}>:ring_buffer::ring_buffer>
                      $<$<NOT:$<BOOL:${ring_buffer_FOUND}>>:ring_buffer>)

option(ENABLE_TESTS "Enable tests" ON)

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

option(ENABLE_EXAMPLE "Enable example" ON)

if(ENABLE_EXAMPLE)
  message(STATUS "Example enabled")
  add_executable(async_logger_example examples/async_logger_example.cc)
  add_executable(async_logger_macro_example
                 examples/async_logger_macro_example.cc)
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
  uninstall_async_buffer COMMAND ${CMAKE_COMMAND} -P
                                 ${CMAKE_CURRENT_BINARY_DIR}/Uninstall.cmake)
