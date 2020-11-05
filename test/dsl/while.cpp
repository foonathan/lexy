// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/while.hpp>

#include "verify.hpp"
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/not.hpp>
#include <lexy/match.hpp>

TEST_CASE("dsl::while_()")
{
    SUBCASE("simple pattern")
    {
        constexpr auto pattern = while_(LEXY_LIT("ab"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(a);
        CHECK(a.match().empty());

        constexpr auto ab = pattern_matches(pattern, "ab");
        CHECK(ab);
        CHECK(ab.match() == "ab");

        constexpr auto abab = pattern_matches(pattern, "abab");
        CHECK(abab);
        CHECK(abab.match() == "abab");

        constexpr auto ababab = pattern_matches(pattern, "ababab");
        CHECK(ababab);
        CHECK(ababab.match() == "ababab");
    }
    SUBCASE("simple rule")
    {
        constexpr auto rule = while_(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto one = rule_matches<callback>(rule, "abc");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, "abcabc");
        CHECK(two == 2);
        constexpr auto three = rule_matches<callback>(rule, "abcabcabc");
        CHECK(three == 3);

        constexpr auto partial = rule_matches<callback>(rule, "abcab");
        CHECK(partial == 1);
    }

    SUBCASE("branch pattern")
    {
        constexpr auto pattern = while_(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(!a);
        CHECK(a.match().empty());

        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(abc);
        CHECK(abc.match() == "abc");
        constexpr auto abcabc = pattern_matches(pattern, "abcabc");
        CHECK(abcabc);
        CHECK(abcabc.match() == "abcabc");

        constexpr auto abcabca = pattern_matches(pattern, "abcabca");
        CHECK(!abcabca);
        CHECK(abcabca.match().empty());
    }
    SUBCASE("branch rule")
    {
        constexpr auto rule = while_(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "bc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto one = rule_matches<callback>(rule, "abc");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, "abcabc");
        CHECK(two == 2);
        constexpr auto three = rule_matches<callback>(rule, "abcabcabc");
        CHECK(three == 3);

        constexpr auto partial = rule_matches<callback>(rule, "abcab");
        CHECK(partial == -1);
    }

    SUBCASE("negative")
    {
        constexpr auto pattern = while_(!LEXY_LIT("a") >> LEXY_LIT("b"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(a);
        CHECK(a.match() == "a");
        constexpr auto ba = pattern_matches(pattern, "ba");
        CHECK(ba);
        CHECK(ba.match() == "ba");
        constexpr auto bba = pattern_matches(pattern, "bba");
        CHECK(bba);
        CHECK(bba.match() == "bba");

        constexpr auto bb = pattern_matches(pattern, "bb");
        CHECK(!bb);
    }

    SUBCASE("choice pattern")
    {
        constexpr auto pattern = while_(LEXY_LIT("a") >> LEXY_LIT("bc") | LEXY_LIT("bc"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(!a);
        CHECK(a.match().empty());
        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(abc);
        CHECK(abc.match() == "abc");

        constexpr auto b = pattern_matches(pattern, "b");
        CHECK(b);
        CHECK(b.match().empty());
        constexpr auto bc = pattern_matches(pattern, "bc");
        CHECK(bc);
        CHECK(bc.match() == "bc");

        constexpr auto abc_bc = pattern_matches(pattern, "abcbc");
        CHECK(abc_bc);
        CHECK(abc_bc.match() == "abcbc");
        constexpr auto abc_bc_a = pattern_matches(pattern, "abcbca");
        CHECK(!abc_bc_a);
        CHECK(abc_bc_a.match().empty());
    }
    SUBCASE("choice rule")
    {
        constexpr auto rule = while_(LEXY_LIT("a") >> LEXY_LIT("bc") | LEXY_LIT("bbc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "bc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto one = rule_matches<callback>(rule, "abc");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, "abcbbc");
        CHECK(two == 2);
        constexpr auto three = rule_matches<callback>(rule, "bbcabcabc");
        CHECK(three == 3);

        constexpr auto partial = rule_matches<callback>(rule, "abcab");
        CHECK(partial == -1);
    }
}

TEST_CASE("dsl::while_one()")
{
    constexpr auto result     = while_one(LEXY_LIT("abc"));
    constexpr auto equivalent = LEXY_LIT("abc") >> while_(LEXY_LIT("abc"));
    CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

    CHECK(!lexy::match(lexy::zstring_input(""), result));
    CHECK(lexy::match(lexy::zstring_input("abc"), result));
    CHECK(lexy::match(lexy::zstring_input("abcabc"), result));
}

TEST_CASE("dsl::do_while()")
{
    constexpr auto result     = do_while(LEXY_LIT("a"), LEXY_LIT("b"));
    constexpr auto equivalent = LEXY_LIT("a") >> while_(LEXY_LIT("b") >> LEXY_LIT("a"));
    CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

    CHECK(!lexy::match(lexy::zstring_input(""), result));
    CHECK(lexy::match(lexy::zstring_input("a"), result));
    CHECK(lexy::match(lexy::zstring_input("aba"), result));
    CHECK(lexy::match(lexy::zstring_input("ababa"), result));
}

