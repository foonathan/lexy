# Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

set(tests
        detail/buffer_builder.cpp
        detail/integer_sequence.cpp
        detail/invoke.cpp
        detail/nttp_string.cpp
        detail/stateless_lambda.cpp
        detail/std.cpp
        detail/string_view.cpp
        detail/type_name.cpp

        dsl/alternative.cpp
        dsl/any.cpp
        dsl/argv_input.cpp
        dsl/ascii.cpp
        dsl/atom.cpp
        dsl/bom.cpp
        dsl/brackets.cpp
        dsl/branch.cpp
        dsl/capture.cpp
        dsl/choice.cpp
        dsl/code_point.cpp
        dsl/combination.cpp
        dsl/context.cpp
        dsl/delimited.cpp
        dsl/digit.cpp
        dsl/encode.cpp
        dsl/eof.cpp
        dsl/error.cpp
        dsl/if.cpp
        dsl/integer.cpp
        dsl/label.cpp
        dsl/list.cpp
        dsl/literal.cpp
        dsl/lookahead.cpp
        dsl/loop.cpp
        dsl/match.cpp
        dsl/member.cpp
        dsl/minus.cpp
        dsl/newline.cpp
        dsl/option.cpp
        dsl/parse_state.cpp
        dsl/peek.cpp
        dsl/position.cpp
        dsl/production.cpp
        dsl/punctuator.cpp
        dsl/return.cpp
        dsl/separator.cpp
        dsl/sequence.cpp
        dsl/sign.cpp
        dsl/switch.cpp
        dsl/terminator.cpp
        dsl/times.cpp
        dsl/until.cpp
        dsl/value.cpp
        dsl/while.cpp
        dsl/whitespace.cpp

        engine/any.cpp
        engine/char_class.cpp
        engine/code_point.cpp
        engine/digits.cpp
        engine/eof.cpp
        engine/failure.cpp
        engine/literal.cpp
        engine/success.cpp
        engine/trie.cpp

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
        error_location.cpp
        lexeme.cpp
        match.cpp
        parse.cpp
        result.cpp
        validate.cpp
    )

add_executable(lexy_test ${tests})
target_link_libraries(lexy_test PRIVATE lexy_test_base)

