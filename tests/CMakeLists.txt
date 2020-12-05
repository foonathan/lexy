# Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

# Fetch doctest.
message(STATUS "Fetching doctest")
include(FetchContent)
FetchContent_Declare(doctest URL https://github.com/onqtam/doctest/archive/2.4.1.zip)
FetchContent_MakeAvailable(doctest)

# A generic test target.
add_library(lexy_test_base ${CMAKE_CURRENT_SOURCE_DIR}/doctest_main.cpp)
target_link_libraries(lexy_test_base PUBLIC foonathan::lexy::dev foonathan::lexy::file doctest)
target_compile_definitions(lexy_test_base PUBLIC DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING=1 LEXY_TEST)

if(MSVC)
    target_compile_options(lexy_test_base PUBLIC
        /Zc:preprocessor    # need a conforming preprocessor for stringifying escape characters
        "/utf-8"            # use utf-8 instead of current code page
    )
endif()

# Add the individual tests.
add_subdirectory(lexy)
add_subdirectory(examples)

add_test(NAME unit_tests COMMAND lexy_test)
add_test(NAME email COMMAND lexy_test_email)
add_test(NAME json COMMAND lexy_test_json)
add_test(NAME shell COMMAND lexy_test_shell)
add_test(NAME xml COMMAND lexy_test_xml)

