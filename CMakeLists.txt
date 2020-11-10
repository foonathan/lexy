# Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

cmake_minimum_required(VERSION 3.8)
project(lexy VERSION 0.0.0 LANGUAGES CXX)

set(LEXY_USER_CONFIG_HEADER "" CACHE FILEPATH "The user config header for lexy.")

add_subdirectory(src)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(examples)
endif()

