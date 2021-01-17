#  Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
#  This file is subject to the license terms in the LICENSE file
#  found in the top-level directory of this distribution.

set(tests
        parse_tree_doctest.cpp
    )

add_executable(lexy_ext_test ${tests})
target_link_libraries(lexy_ext_test PRIVATE lexy_test_base)

