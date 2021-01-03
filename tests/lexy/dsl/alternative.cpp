// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/alternative.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>

TEST_CASE("dsl::_trie")
{
    static constexpr auto rule = lexy::dsl::_trie<LEXY_NTTP_STRING("abc"), LEXY_NTTP_STRING("a"),
                                                  LEXY_NTTP_STRING("def")>{};
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            auto match = lexy::_detail::string_view(str, cur);
            if (match == "abc")
                return 0;
            else if (match == "a")
                return 1;
            else if (match == "def")
                return 2;
            else
                LEXY_VERIFY_CHECK(false);
            return -1;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::exhausted_alternatives> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 0);

    auto a = LEXY_VERIFY("a");
    CHECK(a == 1);
    auto ab = LEXY_VERIFY("ab");
    CHECK(ab == 1);

    auto def = LEXY_VERIFY("def");
    CHECK(def == 2);
}

TEST_CASE("dsl::_alt")
{
    static constexpr auto rule
        = lexy::dsl::_alt<decltype(LEXY_LIT("abc")), decltype(LEXY_LIT("a")),
                          decltype(LEXY_LIT("ab")), decltype(LEXY_LIT("def"))>{};
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            auto match = lexy::_detail::string_view(str, cur);
            if (match == "abc")
                return 0;
            else if (match == "a")
                return 1;
            else if (match == "def")
                return 2;
            else
                LEXY_VERIFY_CHECK(false);
            return -1;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::exhausted_alternatives> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 0);

    auto a = LEXY_VERIFY("a");
    CHECK(a == 1);
    auto ab = LEXY_VERIFY("ab");
    CHECK(ab == 1);

    auto def = LEXY_VERIFY("def");
    CHECK(def == 2);
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

