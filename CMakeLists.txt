# Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

cmake_minimum_required(VERSION 3.8)
project(lexy VERSION 0.0.0 LANGUAGES CXX)

set(LEXY_USER_CONFIG_HEADER "" CACHE FILEPATH "The user config header for lexy.")

add_subdirectory(src)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.12)
    option(LEXY_BUILD_BENCHMARKS "whether or not benchmarks should be built" OFF)
    option(LEXY_BUILD_EXAMPLES   "whether or not examples should be built" ON)
    option(LEXY_BUILD_TESTS      "whether or not tests should be built" ON)
    option(LEXY_BUILD_DOCS       "whether or not docs should be built" OFF)

    if(LEXY_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
    if(LEXY_BUILD_BENCHMARKS)
        add_subdirectory(benchmarks EXCLUDE_FROM_ALL)
    endif()
    if(LEXY_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
    if(LEXY_BUILD_DOCS)
        add_subdirectory(docs EXCLUDE_FROM_ALL)
    endif()
endif()

