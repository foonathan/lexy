# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

cmake_minimum_required(VERSION 3.8)
project(lexy VERSION 0.0.0 LANGUAGES CXX)

set(LEXY_USER_CONFIG_HEADER "" CACHE FILEPATH "The user config header for lexy.")

add_subdirectory(src)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.18)
    option(LEXY_BUILD_BENCHMARKS "whether or not benchmarks should be built" OFF)
    option(LEXY_BUILD_EXAMPLES   "whether or not examples should be built" ON)
    option(LEXY_BUILD_TESTS      "whether or not tests should be built" ON)
    option(LEXY_BUILD_DOCS       "whether or not docs should be built" OFF)

    option(LEXY_FORCE_CPP17      "whether or not Lexy should use C++17 even if compiler supports C++20" OFF)

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
endif()

