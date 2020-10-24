# Copyright (C) 2019-2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

# get doctest
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/doctest_main.h)
    file(DOWNLOAD
         https://raw.githubusercontent.com/onqtam/doctest/master/doctest/doctest.h
         ${CMAKE_CURRENT_BINARY_DIR}/doctest.h
         STATUS status
         LOG log)
    # need separate file for zapcc
    configure_file(${CMAKE_CURRENT_BINARY_DIR}/doctest.h ${CMAKE_CURRENT_BINARY_DIR}/doctest_main.h COPYONLY)

    list(GET status 0 status_code)
    list(GET status 1 status_string)

    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "error downloading doctest: ${status_string}"
                "${log}")
    endif()
endif()

# define dummy target with all the options
add_library(lexy_test_base INTERFACE)
target_sources(lexy_test_base INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
target_link_libraries(lexy_test_base INTERFACE foonathan::lexy foonathan::lexy::file)
target_include_directories(lexy_test_base INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(lexy_test_base INTERFACE -DDOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING=1)
target_compile_options(lexy_test_base INTERFACE
                       # clang/GCC warnings
                       $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
                       -pedantic-errors -Werror -Wall -Wextra -Wconversion -Wsign-conversion -Wno-unused-local-typedefs>
                       # disable annoying warnings
                       $<$<CXX_COMPILER_ID:GNU>: -Wno-parentheses -Wno-noexcept-type>
                       # MSVC warnings
                       $<$<CXX_COMPILER_ID:MSVC>:
                       /WX /W4 /wd4127>)

# the unit tests
file(GLOB_RECURSE tests LIST_DIRECTORIES false CONFIGURE_DEPENDS *.cpp) # TODO: temporary

add_executable(lexy_test ${tests})
target_link_libraries(lexy_test PUBLIC lexy_test_base)
add_test(NAME test COMMAND lexy_test)

