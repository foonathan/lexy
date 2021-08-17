#  Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
#  This file is subject to the license terms in the LICENSE file
#  found in the top-level directory of this distribution.

#===
# Compile-only playground and godbolt infrastructure tests.
add_library(lexy_test_playground OBJECT playground.cpp)
target_link_libraries(lexy_test_playground PRIVATE lexy_test_base)

add_library(lexy_test_godbolt OBJECT godbolt.cpp)
target_link_libraries(lexy_test_godbolt PRIVATE lexy_test_base)
#===

#===
# Compile-only godbolt example tests.
add_library(lexy_test_godbolt_examples OBJECT)
target_link_libraries(lexy_test_godbolt_examples PRIVATE lexy_test_base)

file(GLOB examples CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/docs/assets/examples/*.cpp)
target_sources(lexy_test_godbolt_examples PRIVATE ${examples})
#===

# Compile-only playground example tests.
add_library(lexy_test_playground_examples OBJECT)
target_link_libraries(lexy_test_playground_examples PRIVATE lexy_test_base)

file(GLOB examples CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/docs/assets/playground/*.cpp)
foreach(file ${examples})
    get_filename_component(name ${file} NAME)

    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${name}
        COMMAND ${CMAKE_COMMAND} -E echo "#include <lexy/dsl.hpp>"                                  >  "${CMAKE_CURRENT_BINARY_DIR}/${name}"
        COMMAND ${CMAKE_COMMAND} -E echo "#include <lexy/action/trace.hpp>"                         >> "${CMAKE_CURRENT_BINARY_DIR}/${name}"
        COMMAND ${CMAKE_COMMAND} -E echo "namespace dsl = lexy::dsl;"                               >> "${CMAKE_CURRENT_BINARY_DIR}/${name}"
        COMMAND ${CMAKE_COMMAND} -E echo "#line 1 \"${file}\""                                      >> "${CMAKE_CURRENT_BINARY_DIR}/${name}"
        COMMAND ${CMAKE_COMMAND} -E cat "${file}"                                                   >> "${CMAKE_CURRENT_BINARY_DIR}/${name}"
        COMMAND ${CMAKE_COMMAND} -E echo "[[maybe_unused]] constexpr auto rule = production::rule;" >> "${CMAKE_CURRENT_BINARY_DIR}/${name}"
        VERBATIM DEPENDS "${file}")
    target_sources(lexy_test_playground_examples PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${name})
endforeach()

