# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

cmake_minimum_required(VERSION 3.8)
project(lexy VERSION 0.0.0 LANGUAGES CXX)

set(LEXY_USER_CONFIG_HEADER "" CACHE FILEPATH "The user config header for lexy.")
option(LEXY_FORCE_CPP17     "Whether or not lexy should use C++17 even if compiler supports C++20." OFF)

add_subdirectory(src)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.18)
    option(LEXY_BUILD_BENCHMARKS "whether or not benchmarks should be built" OFF)
    option(LEXY_BUILD_EXAMPLES   "whether or not examples should be built" ON)
    option(LEXY_BUILD_TESTS      "whether or not tests should be built" ON)
    option(LEXY_BUILD_DOCS       "whether or not docs should be built" OFF)

    if(LEXY_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
    if(LEXY_BUILD_BENCHMARKS)
        add_subdirectory(benchmarks)
    endif()
    if(LEXY_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
    if(LEXY_BUILD_DOCS)
        add_subdirectory(docs EXCLUDE_FROM_ALL)
    endif()

    set(package_files include/ src/ CMakeLists.txt LICENSE)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lexy-src.zip
        COMMAND ${CMAKE_COMMAND} -E tar c ${CMAKE_CURRENT_BINARY_DIR}/lexy-src.zip --format=zip -- ${package_files}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${package_files})
    add_custom_target(lexy_package DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/lexy-src.zip)
endif()

