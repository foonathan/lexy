# Copyright (C) 2019-2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

# Get doctest.
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/doctest_main.h)
    file(DOWNLOAD
         https://raw.githubusercontent.com/onqtam/doctest/master/doctest/doctest.h
         ${CMAKE_CURRENT_BINARY_DIR}/doctest.h
         STATUS status
         LOG log)
    # We need separate file for zapcc.
    configure_file(${CMAKE_CURRENT_BINARY_DIR}/doctest.h ${CMAKE_CURRENT_BINARY_DIR}/doctest_main.h COPYONLY)

    list(GET status 0 status_code)
    list(GET status 1 status_string)

    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "error downloading doctest: ${status_string}"
                "${log}")
    endif()
endif()

# Define dummy target with all the options.
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

# Add the unit tests.
set(tests
        detail/buffer_builder.cpp
        detail/integer_sequence.cpp
        detail/invoke.cpp
        detail/nttp_string.cpp
        detail/stateless_lambda.cpp
        detail/string_view.cpp
        detail/type_name.cpp

        dsl/alternative.cpp
        dsl/any.cpp
        dsl/argv_input.cpp
        dsl/ascii.cpp
        dsl/atom.cpp
        dsl/bom.cpp
        dsl/brackets.cpp
        dsl/branch.cpp
        dsl/capture.cpp
        dsl/choice.cpp
        dsl/code_point.cpp
        dsl/combination.cpp
        dsl/context.cpp
        dsl/delimited.cpp
        dsl/digit.cpp
        dsl/encode.cpp
        dsl/eof.cpp
        dsl/error.cpp
        dsl/if.cpp
        dsl/integer.cpp
        dsl/label.cpp
        dsl/list.cpp
        dsl/literal.cpp
        dsl/lookahead.cpp
        dsl/loop.cpp
        dsl/match.cpp
        dsl/minus.cpp
        dsl/newline.cpp
        dsl/not.cpp
        dsl/option.cpp
        dsl/parse_state.cpp
        dsl/peek.cpp
        dsl/position.cpp
        dsl/production.cpp
        dsl/punctuator.cpp
        dsl/return.cpp
        dsl/separator.cpp
        dsl/sequence.cpp
        dsl/sign.cpp
        dsl/switch.cpp
        dsl/times.cpp
        dsl/until.cpp
        dsl/value.cpp
        dsl/while.cpp
        dsl/whitespace.cpp

        input/argv_input.cpp
        input/base.cpp
        input/buffer.cpp
        input/file.cpp
        input/null_input.cpp
        input/range_input.cpp
        input/shell.cpp
        input/string_input.cpp
        
        callback.cpp
        encoding.cpp
        error.cpp
        error_location.cpp
        lexeme.cpp
        match.cpp
        parse.cpp
        result.cpp
        validate.cpp 
    )

add_executable(lexy_test test.cpp ${tests})
target_link_libraries(lexy_test PUBLIC lexy_test_base)
add_test(NAME test COMMAND lexy_test)

