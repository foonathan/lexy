# Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/lexy)
set(header_files 
        ${include_dir}/_detail/assert.hpp
        ${include_dir}/_detail/buffer_builder.hpp
        ${include_dir}/_detail/config.hpp
        ${include_dir}/_detail/detect.hpp
        ${include_dir}/_detail/integer_sequence.hpp
        ${include_dir}/_detail/invoke.hpp
        ${include_dir}/_detail/memory_resource.hpp
        ${include_dir}/_detail/nttp_string.hpp
        ${include_dir}/_detail/stateless_lambda.hpp
        ${include_dir}/_detail/string_view.hpp
        ${include_dir}/_detail/type_name.hpp

        ${include_dir}/dsl/alternative.hpp
        ${include_dir}/dsl/any.hpp
        ${include_dir}/dsl/ascii.hpp
        ${include_dir}/dsl/base.hpp
        ${include_dir}/dsl/bom.hpp
        ${include_dir}/dsl/brackets.hpp
        ${include_dir}/dsl/branch.hpp
        ${include_dir}/dsl/capture.hpp
        ${include_dir}/dsl/choice.hpp
        ${include_dir}/dsl/combination.hpp
        ${include_dir}/dsl/context.hpp
        ${include_dir}/dsl/delimited.hpp
        ${include_dir}/dsl/digit.hpp
        ${include_dir}/dsl/eof.hpp
        ${include_dir}/dsl/encode.hpp
        ${include_dir}/dsl/error.hpp
        ${include_dir}/dsl/if.hpp
        ${include_dir}/dsl/integer.hpp
        ${include_dir}/dsl/label.hpp
        ${include_dir}/dsl/minus.hpp
        ${include_dir}/dsl/newline.hpp
        ${include_dir}/dsl/not.hpp
        ${include_dir}/dsl/option.hpp
        ${include_dir}/dsl/peek.hpp
        ${include_dir}/dsl/position.hpp
        ${include_dir}/dsl/production.hpp
        ${include_dir}/dsl/punctuator.hpp
        ${include_dir}/dsl/return.hpp
        ${include_dir}/dsl/separator.hpp
        ${include_dir}/dsl/sequence.hpp
        ${include_dir}/dsl/sign.hpp
        ${include_dir}/dsl/switch.hpp
        ${include_dir}/dsl/times.hpp
        ${include_dir}/dsl/until.hpp
        ${include_dir}/dsl/value.hpp
        ${include_dir}/dsl/while.hpp
        ${include_dir}/dsl/whitespace.hpp

        ${include_dir}/input/argv_input.hpp
        ${include_dir}/input/base.hpp
        ${include_dir}/input/buffer.hpp
        ${include_dir}/input/null_input.hpp
        ${include_dir}/input/range_input.hpp
        ${include_dir}/input/shell.hpp
        ${include_dir}/input/string_input.hpp

        ${include_dir}/callback.hpp
        ${include_dir}/dsl.hpp
        ${include_dir}/encoding.hpp
        ${include_dir}/error.hpp
        ${include_dir}/error_location.hpp
        ${include_dir}/lexeme.hpp
        ${include_dir}/match.hpp
        ${include_dir}/parse.hpp
        ${include_dir}/result.hpp
        ${include_dir}/validate.hpp)

# main target
add_library(lexy INTERFACE)
add_library(foonathan::lexy ALIAS lexy)
target_sources(lexy INTERFACE "$<BUILD_INTERFACE:${header_files}>")
target_compile_features(lexy INTERFACE cxx_std_17)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    target_include_directories(lexy INTERFACE
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/>
                               $<INSTALL_INTERFACE:../include/>)
else()
    target_include_directories(lexy SYSTEM INTERFACE
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/>
                               $<INSTALL_INTERFACE:../include/>)
endif()

# FILE I/O target
add_library(lexy_file)
add_library(foonathan::lexy::file ALIAS lexy_file)
target_sources(lexy_file PUBLIC "$<BUILD_INTERFACE:${include_dir}/input/file.hpp>"
                         PRIVATE input/file.cpp)
target_link_libraries(lexy_file PUBLIC foonathan::lexy)

