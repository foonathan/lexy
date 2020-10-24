# Copyright (C) 2019-2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

cmake_minimum_required(VERSION 3.8)
project(lexy VERSION 0.0.0 LANGUAGES CXX)

add_subdirectory(src)

# subdirectories
option(LEXY_BUILD_TEST "build tests of lexy" OFF)
if(${LEXY_BUILD_TEST} OR (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    enable_testing()
    add_subdirectory(test)
endif()

option(LEXY_BUILD_EXAMPLE "build examples of lexy" OFF)
if(${LEXY_BUILD_EXAMPLE} OR (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    add_subdirectory(example)
endif()

