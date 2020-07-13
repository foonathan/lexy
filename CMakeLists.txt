# Copyright (C) 2019-2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

cmake_minimum_required(VERSION 3.8)
project(lexy VERSION 0.0.0 LANGUAGES CXX)

file(GLOB_RECURSE source_files LIST_DIRECTORIES false CONFIGURE_DEPENDS *.hpp) # TODO: temporary

# main target
add_library(lexy INTERFACE)
add_library(foonathan::lexy ALIAS lexy)
target_sources(lexy INTERFACE "$<BUILD_INTERFACE:${source_files}>")
target_compile_features(lexy INTERFACE cxx_std_17)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    target_include_directories(lexy INTERFACE
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                               $<INSTALL_INTERFACE:include/>)
else()
    target_include_directories(lexy SYSTEM INTERFACE
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                               $<INSTALL_INTERFACE:include/>)
endif()

# subdirectories
option(LEXY_BUILD_TEST "build tests of lexy" OFF)
if(${LEXY_BUILD_TEST} OR (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    enable_testing()
    add_subdirectory(test)
endif()

