#  Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
#  This file is subject to the license terms in the LICENSE file
#  found in the top-level directory of this distribution.

# Compile-only tests.
add_executable(lexy_test_playground playground.cpp)
target_link_libraries(lexy_test_playground PRIVATE lexy_test_base)
add_executable(lexy_test_godbolt godbolt.cpp)
target_link_libraries(lexy_test_godbolt PRIVATE lexy_test_base)

