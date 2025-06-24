cmake_minimum_required(VERSION 3.25)
project(lexy-cmake-fetchcontent-example LANGUAGES CXX)
include(FetchContent)
FetchContent_Declare(
  lexy
  GIT_REPOSITORY https://github.com/foonathan/lexy.git
  GIT_TAG 20926cf89aaccafc0fa44e02de14fdc411493822
  EXCLUDE_FROM_ALL
  SYSTEM
  CMAKE_ARGS
    "-DLEXY_FORCE_CPP17=OFF"
    "-DLEXY_BUILD_BENCHMARKS=OFF"
    "-DLEXY_BUILD_EXAMPLES=OFF"
    "-DLEXY_BUILD_TESTS=OFF"
    "-DLEXY_BUILD_DOCS=OFF"
    "-DLEXY_BUILD_PACKAGE=OFF"
    "-DLEXY_ENABLE_INSTALL=ON"
)
FetchContent_MakeAvailable(lexy)

add_executable(json-built-with-fetchcontent ${CMAKE_CURRENT_LIST_DIR}/../../json.cpp)
target_link_libraries(json-built-with-fetchcontent PUBLIC foonathan::lexy)

