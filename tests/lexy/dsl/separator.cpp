// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/separator.hpp>

#include "verify.hpp"

TEST_CASE("dsl::sep()")
{
    constexpr auto default_error = dsl::try_(dsl::error<lexy::unexpected_trailing_separator>);

    constexpr auto basic = dsl::sep(LEXY_LIT("abc"));
    CHECK(equivalent_rules(decltype(basic)::rule{}, LEXY_LIT("abc")));
    CHECK(equivalent_rules(decltype(basic)::trailing_rule{},
                           dsl::if_(LEXY_LIT("abc") >> default_error)));

    constexpr auto my_error      = dsl::try_(dsl::error<struct my_error_tag>);
    constexpr auto specify_error = basic.trailing_error<my_error_tag>;
    CHECK(equivalent_rules(decltype(specify_error)::rule{}, LEXY_LIT("abc")));
    CHECK(equivalent_rules(decltype(specify_error)::trailing_rule{},
                           dsl::if_(LEXY_LIT("abc") >> my_error)));
}

TEST_CASE("dsl::trailing_sep()")
{
    constexpr auto sep = dsl::trailing_sep(LEXY_LIT("abc"));
    CHECK(equivalent_rules(decltype(sep)::rule{}, LEXY_LIT("abc")));
    CHECK(equivalent_rules(decltype(sep)::trailing_rule{}, dsl::if_(LEXY_LIT("abc"))));
}

