# Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

find_program(HUGO_BINARY hugo)
if(NOT HUGO_BINARY)
    message(WARNING "hugo not found, won't be able to generate documentation")
endif()

add_custom_target(lexy_docs
        COMMAND ${HUGO_BINARY} --minify
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(lexy_docs_serve
        COMMAND ${HUGO_BINARY} serve
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        USES_TERMINAL)

