// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/until.hpp>

#include "verify.hpp"
#include <lexy/dsl/peek.hpp>

TEST_CASE("dsl::until()")
{
    constexpr auto rule = until(LEXY_LIT("!"));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(!empty);
        CHECK(empty.match().empty());

        constexpr auto zero = pattern_matches(rule, "!");
        CHECK(zero);
        CHECK(zero.match() == "!");
        constexpr auto one = pattern_matches(rule, "a!");
        CHECK(one);
        CHECK(one.match() == "a!");
        constexpr auto two = pattern_matches(rule, "xy!");
        CHECK(two);
        CHECK(two.match() == "xy!");

        constexpr auto unterminated = pattern_matches(rule, "abc");
        CHECK(!unterminated);
        CHECK(unterminated.match().empty());
    }
    SUBCASE("rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.position() == lexy::_detail::string_view(str).end());
                assert(e.string() == "!");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto zero = rule_matches<callback>(rule, "!");
        CHECK(zero == 1);
        constexpr auto one = rule_matches<callback>(rule, "a!");
        CHECK(one == 2);
        constexpr auto two = rule_matches<callback>(rule, "ab!");
        CHECK(two == 3);

        constexpr auto unterminated = rule_matches<callback>(rule, "abc");
        CHECK(unterminated == -1);
    }
}

TEST_CASE("dsl::until().or_eof()")
{
    constexpr auto atom = until(LEXY_LIT("!")).or_eof();
    CHECK(lexy::is_atom<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(empty);
    CHECK(empty.count == 0);

    constexpr auto zero = atom_matches(atom, "!");
    CHECK(zero);
    CHECK(zero.count == 1);
    constexpr auto one = atom_matches(atom, "a!");
    CHECK(one);
    CHECK(one.count == 2);
    constexpr auto two = atom_matches(atom, "xy!");
    CHECK(two);
    CHECK(two.count == 3);

    constexpr auto unterminated = atom_matches(atom, "abc");
    CHECK(unterminated);
    CHECK(unterminated.count == 3);
}

TEST_CASE("dsl::until(peek())")
{
    constexpr auto rule = until(peek(LEXY_LIT("!")));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(!empty);
        CHECK(empty.match().empty());

        constexpr auto zero = pattern_matches(rule, "!");
        CHECK(zero);
        CHECK(zero.match() == "");
        constexpr auto one = pattern_matches(rule, "a!");
        CHECK(one);
        CHECK(one.match() == "a");
        constexpr auto two = pattern_matches(rule, "xy!");
        CHECK(two);
        CHECK(two.match() == "xy");

        constexpr auto unterminated = pattern_matches(rule, "abc");
        CHECK(!unterminated);
        CHECK(unterminated.match().empty());
    }
    SUBCASE("rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.position() == lexy::_detail::string_view(str).end());
                assert(e.string() == "!");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto zero = rule_matches<callback>(rule, "!");
        CHECK(zero == 0);
        constexpr auto one = rule_matches<callback>(rule, "a!");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, "ab!");
        CHECK(two == 2);

        constexpr auto unterminated = rule_matches<callback>(rule, "abc");
        CHECK(unterminated == -1);
    }
}

TEST_CASE("dsl::until(peek()).or_eof()")
{
    constexpr auto atom = until(peek(LEXY_LIT("!"))).or_eof();
    CHECK(lexy::is_atom<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(empty);
    CHECK(empty.count == 0);

    constexpr auto zero = atom_matches(atom, "!");
    CHECK(zero);
    CHECK(zero.count == 0);
    constexpr auto one = atom_matches(atom, "a!");
    CHECK(one);
    CHECK(one.count == 1);
    constexpr auto two = atom_matches(atom, "xy!");
    CHECK(two);
    CHECK(two.count == 2);

    constexpr auto unterminated = atom_matches(atom, "abc");
    CHECK(unterminated);
    CHECK(unterminated.count == 3);
}

