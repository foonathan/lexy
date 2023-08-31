// Copyright (C) 2020-2023 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/operator.hpp>

#include <doctest/doctest.h>
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl/expression.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/input/string_input.hpp>

namespace
{

namespace dsl = lexy::dsl;

struct state
{
    char const*    begin;
    std::ptrdiff_t diff;
};

struct tag
{
    constexpr tag(state& st, char const* it)
    {
        st.diff = it - st.begin;
    }
};

struct test_finish : lexy::expression_production
{
    static constexpr auto atom = LEXY_LIT("x");

    struct operation : dsl::prefix_op
    {
        static constexpr auto op = dsl::op<tag>(LEXY_LIT("-"));
        using operand            = dsl::atom;
    };

    static constexpr auto value = lexy::noop;
};

struct test_bp
{
    static constexpr auto rule  = dsl::op<tag>(LEXY_LIT("-")) | dsl::op<tag>(LEXY_LIT("+"));
    static constexpr auto value = lexy::forward<tag>;
};

struct test_p
{
    static constexpr auto rule  = dsl::op<tag>(LEXY_LIT("-"));
    static constexpr auto value = lexy::forward<tag>;
};

} // namespace

TEST_CASE("dsl::op_state")
{
    SUBCASE("finish")
    {
        auto const* str   = "-x";
        auto        input = lexy::zstring_input(str);
        auto        st    = state{input.reader().position(), -1};
        auto        ret   = lexy::parse<test_finish>(input, st, lexy::noop);
        CHECK(ret.is_success());
        CHECK(st.diff == 0);
    }
    SUBCASE("bp")
    {
        auto const* str   = "-";
        auto        input = lexy::zstring_input(str);
        auto        st    = state{input.reader().position(), -1};
        auto        ret   = lexy::parse<test_bp>(input, st, lexy::noop);
        CHECK(ret.is_success());
        CHECK(st.diff == 0);
    }
    SUBCASE("p")
    {
        auto const* str   = "-";
        auto        input = lexy::zstring_input(str);
        auto        st    = state{input.reader().position(), -1};
        auto        ret   = lexy::parse<test_p>(input, st, lexy::noop);
        CHECK(ret.is_success());
        CHECK(st.diff == 0);
    }
}
