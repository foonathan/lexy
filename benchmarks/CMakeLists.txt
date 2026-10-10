# Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

cmake_minimum_required(VERSION 3.31)

# Fetch nanobench.
message(STATUS "Fetching nanobench")
include(FetchContent)
FetchContent_Declare(nanobench URL https://github.com/martinus/nanobench/archive/refs/tags/v4.6.0.zip)
FetchContent_MakeAvailable(nanobench)

add_subdirectory(json)
add_subdirectory(file)
add_subdirectory(swar)

