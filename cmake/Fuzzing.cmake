# Fuzzing configuration for lexy
# Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

# Fuzzing options
option(LEXY_BUILD_FUZZERS "whether or not fuzzers should be built" OFF)
option(LEXY_BUILD_FUZZERS_AFLPLUSPLUS "Use AFL++ instead of libFuzzer" OFF)
option(FORCE_STATIC_LINKING "Force static linking for fuzzing targets" OFF)

# Check compiler requirements if fuzzing is enabled
function(setup_fuzzing_environment)
  if(NOT LEXY_BUILD_FUZZERS)
    return()
  endif()

  # Verify we're using Clang
  execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --version
        OUTPUT_VARIABLE CXX_COMPILER_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(NOT CXX_COMPILER_VERSION MATCHES "clang")
    message(FATAL_ERROR "Fuzzing requires a Clang-based compiler")
  endif()

  # Auto-detect AFL compiler
  if(CMAKE_C_COMPILER MATCHES ".*afl-.*" OR CMAKE_CXX_COMPILER MATCHES ".*afl-.*")
    set(LEXY_BUILD_FUZZERS_AFLPLUSPLUS ON CACHE BOOL "Use AFL++ instead of libFuzzer" FORCE)
    message(STATUS "AFL++ compiler detected - automatically enabling AFL++ mode")
  endif()

  # Add sanitizer flags for better fuzzing
  if(NOT DEFINED ENV{SANITIZER})
    set(ENV{SANITIZER} "address")
  endif()

  # Add fuzzers directory
  add_subdirectory(fuzzers)
endfunction()

# Function to add a fuzzing target
function(add_fuzzer name)
  if(NOT LEXY_BUILD_FUZZERS)
    return()
  endif()

  set(options "")
  set(oneValueArgs "")
  set(multiValueArgs SOURCES INCLUDES LIBRARIES)
  cmake_parse_arguments(FUZZER "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${name} ${FUZZER_SOURCES})

  if(FUZZER_INCLUDES)
    target_include_directories(${name} PRIVATE ${FUZZER_INCLUDES})
  endif()

  target_link_libraries(${name} PRIVATE ${FUZZER_LIBRARIES})

  # Configure fuzzer specific flags
  if(LEXY_BUILD_FUZZERS_AFLPLUSPLUS)
    # AFL++ configuration
    target_compile_options(${name} PRIVATE -fno-omit-frame-pointer -gline-tables-only)
  else()
    # libFuzzer configuration
    target_compile_options(${name} PRIVATE -fsanitize=fuzzer -fno-omit-frame-pointer)
    target_link_options(${name} PRIVATE -fsanitize=fuzzer)
  endif()

  # Add sanitizer if specified
  if(DEFINED ENV{SANITIZER} AND NOT "$ENV{SANITIZER}" STREQUAL "none")
    target_compile_options(${name} PRIVATE -fsanitize=$ENV{SANITIZER})
    target_link_options(${name} PRIVATE -fsanitize=$ENV{SANITIZER})
  endif()

  # Handle static linking option
  if(FORCE_STATIC_LINKING)
    target_link_options(${name} PRIVATE -static)
  endif()
endfunction()
