// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/alternative.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>

TEST_CASE("dsl::_trie")
{
    constexpr auto rule = lexy::dsl::_trie<LEXY_NTTP_STRING("abc"), LEXY_NTTP_STRING("a"),
                                           LEXY_NTTP_STRING("def")>{};
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(!empty);

        constexpr auto abc = pattern_matches(rule, "abc");
        CHECK(abc);
        CHECK(abc.match() == "abc");

        constexpr auto a = pattern_matches(rule, "a");
        CHECK(a);
        CHECK(a.match() == "a");

        constexpr auto ab = pattern_matches(rule, "ab");
        CHECK(ab);
        CHECK(ab.match() == "a");

        constexpr auto def = pattern_matches(rule, "def");
        CHECK(def);
        CHECK(def.match() == "def");
    }
    SUBCASE("rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                auto match = lexy::_detail::string_view(str, cur);
                if (match == "abc")
                    return 0;
                else if (match == "a")
                    return 1;
                else if (match == "def")
                    return 2;
                else
                    CONSTEXPR_CHECK(false);
            }

            constexpr int error(test_error<lexy::exhausted_alternatives> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 1);
        constexpr auto ab = rule_matches<callback>(rule, "ab");
        CHECK(ab == 1);

        constexpr auto def = rule_matches<callback>(rule, "def");
        CHECK(def == 2);
    }
}

TEST_CASE("dsl::_alt")
{
    constexpr auto rule = lexy::dsl::_alt<decltype(LEXY_LIT("abc")), decltype(LEXY_LIT("a")),
                                          decltype(LEXY_LIT("ab")), decltype(LEXY_LIT("def"))>{};
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(!empty);

        constexpr auto abc = pattern_matches(rule, "abc");
        CHECK(abc);
        CHECK(abc.match() == "abc");

        constexpr auto a = pattern_matches(rule, "a");
        CHECK(a);
        CHECK(a.match() == "a");

        constexpr auto ab = pattern_matches(rule, "ab");
        CHECK(ab);
        CHECK(ab.match() == "a");

        constexpr auto def = pattern_matches(rule, "def");
        CHECK(def);
        CHECK(def.match() == "def");
    }
    SUBCASE("rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                auto match = lexy::_detail::string_view(str, cur);
                if (match == "abc")
                    return 0;
                else if (match == "a")
                    return 1;
                else if (match == "def")
                    return 2;
                else
                    CONSTEXPR_CHECK(false);
            }

            constexpr int error(test_error<lexy::exhausted_alternatives> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 1);
        constexpr auto ab = rule_matches<callback>(rule, "ab");
        CHECK(ab == 1);

        constexpr auto def = rule_matches<callback>(rule, "def");
        CHECK(def == 2);
    }
}

TEST_CASE("dsl::operator/")
{
    // Only check types, behavior is checked above.

    constexpr auto trie    = (LEXY_LIT("abc") / LEXY_LIT("a")) / LEXY_LIT("def");
    constexpr auto trie_eq = lexy::dsl::_trie<LEXY_NTTP_STRING("abc"), LEXY_NTTP_STRING("a"),
                                              LEXY_NTTP_STRING("def")>{};
    CHECK(std::is_same_v<decltype(trie), decltype(trie_eq)>);

    constexpr auto alt    = lexy::dsl::ascii::alpha / lexy::dsl::ascii::digit;
    constexpr auto alt_eq = lexy::dsl::_alt<lexy::dsl::ascii::_alpha, lexy::dsl::ascii::_digit>{};
    CHECK(std::is_same_v<decltype(alt), decltype(alt_eq)>);

    constexpr auto trie_alt = trie / lexy::dsl::ascii::alpha;
    constexpr auto trie_alt_eq
        = lexy::dsl::_alt<std::remove_const_t<decltype(trie_eq)>, lexy::dsl::ascii::_alpha>{};
    CHECK(std::is_same_v<decltype(trie_alt), decltype(trie_alt_eq)>);

    constexpr auto alt_trie    = alt / LEXY_LIT("abc");
    constexpr auto alt_trie_eq = lexy::dsl::_alt<lexy::dsl::ascii::_alpha, lexy::dsl::ascii::_digit,
                                                 decltype(LEXY_LIT("abc"))>{};
    CHECK(std::is_same_v<decltype(alt_trie), decltype(alt_trie_eq)>);
}

