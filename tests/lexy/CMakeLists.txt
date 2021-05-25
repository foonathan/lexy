# Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

set(tests
        detail/buffer_builder.cpp
        detail/integer_sequence.cpp
        detail/invoke.cpp
        detail/lazy_init.cpp
        detail/nttp_string.cpp
        detail/stateless_lambda.cpp
        detail/std.cpp
        detail/string_view.cpp
        detail/tuple.cpp
        detail/type_name.cpp

        callback/aggregate.cpp
        callback/base.cpp
        callback/bind.cpp
        callback/constant.cpp
        callback/container.cpp
        callback/forward.cpp
        callback/integer.cpp
        callback/noop.cpp
        callback/object.cpp
        callback/string.cpp

        dsl/alternative.cpp
        dsl/any.cpp
        dsl/argv_input.cpp
        dsl/ascii.cpp
        dsl/base.cpp
        dsl/bom.cpp
        dsl/brackets.cpp
        dsl/branch.cpp
        dsl/capture.cpp
        dsl/choice.cpp
        dsl/code_point.cpp
        dsl/combination.cpp
        dsl/context_counter.cpp
        dsl/context_flag.cpp
        dsl/context_lexeme.cpp
        dsl/delimited.cpp
        dsl/digit.cpp
        dsl/encode.cpp
        dsl/eof.cpp
        dsl/error.cpp
        dsl/identifier.cpp
        dsl/if.cpp
        dsl/integer.cpp
        dsl/label.cpp
        dsl/list.cpp
        dsl/literal.cpp
        dsl/lookahead.cpp
        dsl/loop.cpp
        dsl/member.cpp
        dsl/minus.cpp
        dsl/newline.cpp
        dsl/option.cpp
        dsl/peek.cpp
        dsl/position.cpp
        dsl/production.cpp
        dsl/punctuator.cpp
        dsl/recover.cpp
        dsl/return.cpp
        dsl/separator.cpp
        dsl/sequence.cpp
        dsl/sign.cpp
        dsl/symbol.cpp
        dsl/terminator.cpp
        dsl/times.cpp
        dsl/token.cpp
        dsl/until.cpp
        dsl/while.cpp
        dsl/whitespace.cpp

        engine/any.cpp
        engine/char_class.cpp
        engine/code_point.cpp
        engine/digits.cpp
        engine/eof.cpp
        engine/failure.cpp
        engine/find.cpp
        engine/literal.cpp
        engine/minus.cpp
        engine/trie.cpp
        engine/until.cpp
        engine/while.cpp

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
        lexeme.cpp
        match.cpp
        parse.cpp
        parse_tree.cpp
        production.cpp
        token.cpp
        validate.cpp
    )

option(LEXY_DISABLE_CONSTEXPR_TESTS "whether or not constexpr unit tests are disabled" OFF)

add_executable(lexy_test ${tests})
target_link_libraries(lexy_test PRIVATE lexy_test_base)
if(LEXY_DISABLE_CONSTEXPR_TESTS)
    message(STATUS "constexpr unit tests are disabled")
    target_compile_definitions(lexy_test PRIVATE -DLEXY_DISABLE_CONSTEXPR_TESTS)
endif()

