# Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

# Fetch nanobench.
message(STATUS "Fetching nanobench")
include(FetchContent)
FetchContent_Declare(nanobench URL https://github.com/martinus/nanobench/archive/v4.3.0.zip)
FetchContent_MakeAvailable(nanobench)

add_subdirectory(json)
add_subdirectory(file)

