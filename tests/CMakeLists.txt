# Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

# Fetch doctest.
message(STATUS "Fetching doctest")
include(FetchContent)
FetchContent_Declare(doctest URL https://github.com/onqtam/doctest/archive/2.4.5.zip)
FetchContent_MakeAvailable(doctest)

# A generic test target.
add_library(lexy_test_base ${CMAKE_CURRENT_SOURCE_DIR}/doctest_main.cpp)
target_link_libraries(lexy_test_base PUBLIC foonathan::lexy::dev foonathan::lexy::file foonathan::lexy::unicode doctest)
target_compile_definitions(lexy_test_base PUBLIC LEXY_TEST)

if(MSVC AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    target_compile_options(lexy_test_base PUBLIC
        /Zc:preprocessor    # need a conforming preprocessor for stringifying escape characters
        "/utf-8"            # use utf-8 instead of current code page
    )
endif()

# Add the individual tests.
add_subdirectory(lexy)
add_subdirectory(lexy_ext)
add_subdirectory(examples)
add_subdirectory(playground)

add_test(NAME lexy_test COMMAND lexy_test)
add_test(NAME lexy_ext_test COMMAND lexy_ext_test)
add_test(NAME email COMMAND lexy_test_email)
add_test(NAME ip COMMAND lexy_test_ip_address)
add_test(NAME json COMMAND lexy_test_json)
add_test(NAME shell COMMAND lexy_test_shell)
add_test(NAME xml COMMAND lexy_test_xml)

