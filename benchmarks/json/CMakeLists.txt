# Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

include(FetchContent)

# Fetch benchmark data.
message(STATUS "Fetching json benchmark data")
# renovate: datasource=git-refs depName=https://github.com/miloyip/nativejson-benchmark version=master
set(json_data_revision 478d5727c2a4048e835a29c65adecc7d795360d5)
set(json_data_dir "${CMAKE_CURRENT_BINARY_DIR}/data/${json_data_revision}")
function(fetch_data file url)
    if (NOT EXISTS "${json_data_dir}/${file}")
        file(DOWNLOAD "${url}" "${json_data_dir}/${file}" STATUS status)
        list(GET status 0 status_code)
        list(GET status 1 status_string)

        if(NOT status_code EQUAL 0)
            message(FATAL_ERROR "error downloading ${file}: ${status_string}")
        endif()
    endif()
endfunction()

set(json_data_url "https://raw.githubusercontent.com/miloyip/nativejson-benchmark/${json_data_revision}/data")
fetch_data(canada.json "${json_data_url}/canada.json")
fetch_data(citm_catalog.json "${json_data_url}/citm_catalog.json")
fetch_data(twitter.json "${json_data_url}/twitter.json")

# Benchmarking executable.
add_executable(lexy_benchmark_json)
target_sources(lexy_benchmark_json PRIVATE main.cpp baseline.cpp lexy.cpp)
target_link_libraries(lexy_benchmark_json PRIVATE foonathan::lexy::dev foonathan::lexy::file foonathan::lexy::unicode nanobench)
target_compile_definitions(lexy_benchmark_json PRIVATE LEXY_BENCHMARK_DATA="${json_data_dir}/")
set_target_properties(lexy_benchmark_json PROPERTIES OUTPUT_NAME "json")

# Compare with PEGTL's json parser.
message(STATUS "Fetching PEGTL")
# PEGTL 4 no longer provides the contrib/json.hpp API used by this benchmark.
FetchContent_Declare(pegtl URL https://github.com/taocpp/PEGTL/archive/refs/tags/3.2.9.zip)
FetchContent_MakeAvailable(pegtl)
set_property(DIRECTORY ${pegtl_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)

target_sources(lexy_benchmark_json PRIVATE pegtl.cpp)
target_link_libraries(lexy_benchmark_json PRIVATE taocpp::pegtl)

# Compare with nlohmann/json.
message(STATUS "Fetching nlohmann/json")
# Only the headers are needed; do not configure the dependency's own build.
FetchContent_Declare(nlohmann_json URL https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.zip
    SOURCE_SUBDIR include)
FetchContent_MakeAvailable(nlohmann_json)

target_sources(lexy_benchmark_json PRIVATE nlohmann.cpp)
target_include_directories(lexy_benchmark_json SYSTEM PRIVATE ${nlohmann_json_SOURCE_DIR}/include)

# Compare with rapidjson.
message(STATUS "Fetching rapidjson")
# renovate: datasource=git-refs depName=https://github.com/Tencent/rapidjson version=master
FetchContent_Declare(rapidjson URL https://github.com/Tencent/rapidjson/archive/24b5e7a8b27f42fa16b96fc70aade9106cf7102f.zip
    SOURCE_SUBDIR include)
FetchContent_MakeAvailable(rapidjson)

target_sources(lexy_benchmark_json PRIVATE rapidjson.cpp)
target_include_directories(lexy_benchmark_json SYSTEM PRIVATE ${rapidjson_SOURCE_DIR}/include)

# Compare with Boost.JSON.
find_package(Boost CONFIG QUIET COMPONENTS json)
if (Boost_FOUND)
    target_sources(lexy_benchmark_json PRIVATE boost.cpp)
    target_link_libraries(lexy_benchmark_json PRIVATE Boost::json)
    target_compile_definitions(lexy_benchmark_json PRIVATE LEXY_HAS_BOOST_JSON)
endif()

# compare glaze
FetchContent_Declare(glaze URL https://github.com/stephenberry/glaze/archive/refs/tags/v9.0.0.zip)
FetchContent_MakeAvailable(glaze)
target_sources(lexy_benchmark_json PRIVATE glaze.cpp)
target_link_libraries(lexy_benchmark_json PRIVATE glaze::glaze)
