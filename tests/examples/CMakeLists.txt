# Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

add_executable(lexy_test_email email.cpp)
target_link_libraries(lexy_test_email PRIVATE lexy_test_base)

add_executable(lexy_test_json json.cpp)
target_link_libraries(lexy_test_json PRIVATE lexy_test_base)

add_executable(lexy_test_xml xml.cpp)
target_link_libraries(lexy_test_xml PRIVATE lexy_test_base)

