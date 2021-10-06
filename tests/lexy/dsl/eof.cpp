// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/eof.hpp>

#include "verify.hpp"

TEST_CASE("dsl::eof")
{
    constexpr auto rule = lexy::dsl::eof;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.trace == test_trace().eof());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::fatal_error);
    CHECK(abc.trace == test_trace().expected_char_class(0, "EOF").cancel());

    // 0xFF is the EOF marker for UTF-8 input.
    auto invalid_UTF8 = LEXY_VERIFY(lexy::utf8_encoding{}, 0xFF, 'a', 'b', 'c');
    CHECK(invalid_UTF8.status == test_result::success);
    CHECK(invalid_UTF8.trace == test_trace().eof());
}

