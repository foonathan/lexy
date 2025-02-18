cmake_minimum_required(VERSION 3.18)

set(BASE_FLAGS -O2 -ggdb -fno-omit-frame-pointer)

if(FORCE_STATIC_LINKING)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(BUILD_SHARED_LIBS OFF)
endif()

# Handle fuzzing-specific settings
if(LEXY_FUZZING_ENGINE STREQUAL "libfuzzer")
    # For libFuzzer, use fuzzer-no-link for the library
    set(SANITIZER_FLAGS
        -fsanitize=address,undefined,fuzzer-no-link
    )

    set(FUZZING_FLAGS
        -fsanitize=fuzzer,address,undefined
    )
else()  # AFL
    set(SANITIZER_FLAGS
        -fsanitize=address,undefined
    )

    set(FUZZING_FLAGS
        -fsanitize=fuzzer
    )
endif()

add_compile_options(${BASE_FLAGS} ${SANITIZER_FLAGS})
add_link_options(${BASE_FLAGS} ${SANITIZER_FLAGS})

function(add_lexy_fuzzer TARGET_NAME SOURCE_FILE EXAMPLE_FILE SEED_CONTENT)
    # Create corpus directories
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_corpus)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_artifacts)

    if(EXISTS "${CMAKE_SOURCE_DIR}/examples/${EXAMPLE_FILE}")
        configure_file(
            ${CMAKE_SOURCE_DIR}/examples/${EXAMPLE_FILE}
            ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE_FILE}
            COPYONLY
        )
    endif()

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_corpus/simple.${TARGET_NAME}
        "${SEED_CONTENT}"
    )

    add_executable(${TARGET_NAME} ${SOURCE_FILE})

    target_compile_options(${TARGET_NAME} PRIVATE
        ${BASE_FLAGS}
        ${SANITIZER_FLAGS}
        ${FUZZING_FLAGS}
    )

    # Handle static linking options
    if(FORCE_STATIC_LINKING)
        if(LEXY_FUZZING_ENGINE STREQUAL "libfuzzer")
            target_link_options(${TARGET_NAME} PRIVATE
                ${BASE_FLAGS}
                ${FUZZING_FLAGS}
                ${SANITIZER_FLAGS}
                -static-libstdc++
                -static-libgcc
            )
        else() # AFL
            target_link_options(${TARGET_NAME} PRIVATE
                ${BASE_FLAGS}
                ${SANITIZER_FLAGS}
                ${FUZZING_FLAGS}
                -static-libstdc++
                -static-libgcc
            )
        endif()
    else()
        target_link_options(${TARGET_NAME} PRIVATE
            ${BASE_FLAGS}
            ${SANITIZER_FLAGS}
            ${FUZZING_FLAGS}
        )
    endif()

    # Link against the libraries
    if(FORCE_STATIC_LINKING)
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            -static-libstdc++
            -static-libgcc
            lexy_core
            lexy_file
            lexy_unicode
        )
    else()
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            lexy_core
            lexy_file
            lexy_unicode
        )
    endif()

    target_include_directories(${TARGET_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_target_properties(${TARGET_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endfunction()

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
