# Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

add_executable(lexy_example_tutorial)
set_target_properties(lexy_example_tutorial PROPERTIES OUTPUT_NAME "tutorial")
target_sources(lexy_example_tutorial PRIVATE tutorial.cpp)
target_link_libraries(lexy_example_tutorial PRIVATE foonathan::lexy::dev foonathan::lexy::file)

add_executable(lexy_example_email)
set_target_properties(lexy_example_email PROPERTIES OUTPUT_NAME "email")
target_sources(lexy_example_email PRIVATE email.cpp)
target_link_libraries(lexy_example_email PRIVATE foonathan::lexy::dev foonathan::lexy::file)

add_executable(lexy_example_json)
set_target_properties(lexy_example_json PROPERTIES OUTPUT_NAME "json")
target_sources(lexy_example_json PRIVATE json.cpp)
target_link_libraries(lexy_example_json PRIVATE foonathan::lexy::dev foonathan::lexy::file)

add_executable(lexy_example_shell)
set_target_properties(lexy_example_shell PROPERTIES OUTPUT_NAME "shell")
target_sources(lexy_example_shell PRIVATE shell.cpp)
target_link_libraries(lexy_example_shell PRIVATE foonathan::lexy::dev foonathan::lexy::file)

add_executable(lexy_example_xml)
set_target_properties(lexy_example_xml PROPERTIES OUTPUT_NAME "xml")
target_sources(lexy_example_xml PRIVATE xml.cpp)
target_link_libraries(lexy_example_xml PRIVATE foonathan::lexy::dev foonathan::lexy::file)

