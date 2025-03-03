# Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

# Only process this file if fuzzing is enabled
if(NOT LEXY_BUILD_FUZZERS)
    return()
endif()

# Create directories for corpus and artifacts
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/corpus)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/artifacts)

# Wrapper around the add_fuzzer function that adds lexy-specific configuration
function(add_lexy_fuzzer TARGET_NAME SOURCE_FILE EXAMPLE_FILE SEED_CONTENT)
    # Create corpus directories for this specific fuzzer
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_corpus)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_artifacts)

    # Copy example file if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/examples/${EXAMPLE_FILE}")
        configure_file(
            ${CMAKE_SOURCE_DIR}/examples/${EXAMPLE_FILE}
            ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE_FILE}
            COPYONLY
        )
    endif()

    # Create seed content
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_corpus/simple.${TARGET_NAME}
        "${SEED_CONTENT}"
    )

    # Add the fuzzer using our common function from Fuzzing.cmake
    add_fuzzer(${TARGET_NAME}
        SOURCES
            ${SOURCE_FILE}
        INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
            ${CMAKE_CURRENT_BINARY_DIR}
        LIBRARIES
            lexy_core
            lexy_file
            lexy_unicode
    )

    # Set output directory
    set_target_properties(${TARGET_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endfunction()

# Add the fuzzers
add_lexy_fuzzer(
    json_fuzzer
    json_fuzzer.cpp
    json.cpp
    "{\"test\": true, \"number\": 42, \"string\": \"hello\", \"array\": [1,2,3]}"
)

add_lexy_fuzzer(
    xml_fuzzer
    xml_fuzzer.cpp
    xml.cpp
    "<root><test>Hello</test><data><![CDATA[Test data]]></data><empty/></root>"
)

add_lexy_fuzzer(
    parse_tree_fuzzer
    parse_tree_fuzzer.cpp
    "parse_tree.hpp"
    "<root><node>Sample</node></root>"
)

add_lexy_fuzzer(
    visualize_fuzzer
    visualize_fuzzer.cpp
    "visualize.hpp"
    "normal string\n\\u0041\\u0042\\n\\t\\r\\u{10FFFF}\\u{000000}\n<unicode>⚡️🌈🌍</unicode>\n\\u0000"
)

add_lexy_fuzzer(
    buffer_fuzzer
    buffer_fuzzer.cpp
    "_detail/buffer_builder.hpp"
    "ABCD\\0\\xFF\\x80\\xFE\\0\\1\\2\\3\\4\\5\\6\\7\\x42\\xAA\\xBB\\xCC\\xDD"
)
