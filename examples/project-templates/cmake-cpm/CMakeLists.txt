cmake_minimum_required(VERSION 3.15)
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.8/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=78ba32abdf798bc616bab7c73aac32a17bbd7b06ad9e26a6add69de8f3ae4791
)
project(lexy-cmake-cpm-example LANGUAGES CXX)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
CPMUsePackageLock(package-lock.cmake)
CPMAddPackage(
  NAME lexy
  GITHUB_REPOSITORY foonathan/lexy
  GIT_TAG 20926cf89aaccafc0fa44e02de14fdc411493822
  OPTIONS
    "LEXY_FORCE_CPP17      OFF"
    "LEXY_BUILD_BENCHMARKS OFF"
    "LEXY_BUILD_EXAMPLES   OFF"
    "LEXY_BUILD_TESTS      OFF"
    "LEXY_BUILD_DOCS       OFF"
    "LEXY_BUILD_PACKAGE    OFF"
    "LEXY_ENABLE_INSTALL   ON"
)

add_executable(json-built-with-cpm ${CMAKE_CURRENT_LIST_DIR}/../../json.cpp)
target_link_libraries(json-built-with-cpm PUBLIC foonathan::lexy)

