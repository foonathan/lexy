# Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

# Get doctest.
if(NOT EXISTS ${CMAKE_BINARY_DIR}/doctest_main.h)
    file(DOWNLOAD
         https://raw.githubusercontent.com/onqtam/doctest/master/doctest/doctest.h
         ${CMAKE_BINARY_DIR}/doctest.h
         STATUS status
         LOG log)
    # We need separate file for zapcc.
    configure_file(${CMAKE_BINARY_DIR}/doctest.h ${CMAKE_BINARY_DIR}/doctest_main.h COPYONLY)

    list(GET status 0 status_code)
    list(GET status 1 status_string)

    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "error downloading doctest: ${status_string}"
                "${log}")
    endif()
endif()

# A generic test target.
add_library(lexy_test_base INTERFACE)
target_sources(lexy_test_base INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/doctest_main.cpp)
target_link_libraries(lexy_test_base INTERFACE foonathan::lexy::dev foonathan::lexy::file)
target_include_directories(lexy_test_base INTERFACE ${CMAKE_BINARY_DIR})
target_compile_definitions(lexy_test_base INTERFACE DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING=1 LEXY_TEST)

# Add the individual tests.
add_subdirectory(lexy)
add_subdirectory(examples)

add_test(NAME unit_tests COMMAND lexy_test)
add_test(NAME email COMMAND lexy_test_email)
add_test(NAME json COMMAND lexy_test_json)
add_test(NAME shell COMMAND lexy_test_shell)
add_test(NAME xml COMMAND lexy_test_xml)

