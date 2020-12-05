# Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

set(html_output_dir ${CMAKE_CURRENT_SOURCE_DIR}/html)

add_custom_target(lexy_docs_tutorial
    COMMAND asciidoctor -o ${html_output_dir}/tutorial.html
            ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.adoc)

add_custom_target(lexy_docs_reference
    COMMAND asciidoctor -r ${CMAKE_CURRENT_SOURCE_DIR}/godbolt.rb
            -o ${html_output_dir}/reference.html
            ${CMAKE_CURRENT_SOURCE_DIR}/reference.adoc)

add_custom_target(lexy_docs_benchmark_json
    COMMAND asciidoctor -o ${html_output_dir}/benchmark_json.html
            ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_json.adoc)

add_custom_target(lexy_docs)
add_dependencies(lexy_docs lexy_docs_tutorial lexy_docs_reference lexy_docs_benchmark_json)

