// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/newline.hpp>

#include "verify.hpp"

TEST_CASE("dsl::newline")
{
    constexpr auto rule = lexy::dsl::newline;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "newline").cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::fatal_error);
    CHECK(abc.trace == test_trace().expected_char_class(0, "newline").cancel());

    auto n = LEXY_VERIFY("\n");
    CHECK(n.status == test_result::success);
    CHECK(n.trace == test_trace().token("\\n"));
    auto rn = LEXY_VERIFY("\r\n");
    CHECK(rn.status == test_result::success);
    CHECK(rn.trace == test_trace().token("\\r\\n"));

    auto r = LEXY_VERIFY("\r");
    CHECK(r.status == test_result::fatal_error);
    CHECK(r.trace == test_trace().expected_char_class(0, "newline").error_token("\\r").cancel());

    auto nr = LEXY_VERIFY("\n\r");
    CHECK(nr.status == test_result::success);
    CHECK(nr.trace == test_trace().token("\\n"));
    auto nn = LEXY_VERIFY("\n\n");
    CHECK(nn.status == test_result::success);
    CHECK(nn.trace == test_trace().token("\\n"));
    auto nrn = LEXY_VERIFY("\n\r\n");
    CHECK(nrn.status == test_result::success);
    CHECK(nrn.trace == test_trace().token("\\n"));

    auto utf16 = LEXY_VERIFY(u"\r\n");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("\\r\\n"));
}

TEST_CASE("dsl::eol")
{
    constexpr auto rule = lexy::dsl::eol;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.trace == test_trace().token("EOL", ""));

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::fatal_error);
    CHECK(abc.trace == test_trace().expected_char_class(0, "EOL").cancel());

    auto n = LEXY_VERIFY("\n");
    CHECK(n.status == test_result::success);
    CHECK(n.trace == test_trace().token("EOL", "\\n"));
    auto rn = LEXY_VERIFY("\r\n");
    CHECK(rn.status == test_result::success);
    CHECK(rn.trace == test_trace().token("EOL", "\\r\\n"));

    auto r = LEXY_VERIFY("\r");
    CHECK(r.status == test_result::fatal_error);
    CHECK(r.trace == test_trace().expected_char_class(0, "EOL").error_token("\\r").cancel());

    auto nr = LEXY_VERIFY("\n\r");
    CHECK(nr.status == test_result::success);
    CHECK(nr.trace == test_trace().token("EOL", "\\n"));
    auto nn = LEXY_VERIFY("\n\n");
    CHECK(nn.status == test_result::success);
    CHECK(nn.trace == test_trace().token("EOL", "\\n"));
    auto nrn = LEXY_VERIFY("\n\r\n");
    CHECK(nrn.status == test_result::success);
    CHECK(nrn.trace == test_trace().token("EOL", "\\n"));

    auto utf16 = LEXY_VERIFY(u"\r\n");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("EOL", "\\r\\n"));
}

