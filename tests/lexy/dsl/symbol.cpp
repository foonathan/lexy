// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/symbol.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/identifier.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/token.hpp>

TEST_CASE("symbol_table")
{
    // Note: try_parse() and key_index tested implicitly by the actual parsing code.

    SUBCASE("empty")
    {
        auto table = lexy::symbol_table<int>;
        CHECK(table.empty());
        CHECK(table.size() == 0);
        CHECK(table.begin() == table.end());
    }
    SUBCASE("non-empty")
    {
        auto table = lexy::symbol_table<int> //
                         .map<'a'>(0)
                         .map<'b'>(1)
                         .map<LEXY_SYMBOL("c")>(2)
#if LEXY_HAS_NTTP
                         .map<"abc">(3);
#else
                         .map<LEXY_SYMBOL("abc")>(3);
#endif
        CHECK(!table.empty());
        CHECK(table.size() == 4);

        auto iter = table.begin();
        CHECK(iter != table.end());
        CHECK(iter->symbol == lexy::_detail::string_view("a"));
        CHECK(iter->value == 0);

        ++iter;
        CHECK(iter != table.end());
        CHECK(iter->symbol == lexy::_detail::string_view("b"));
        CHECK(iter->value == 1);

        ++iter;
        CHECK(iter != table.end());
        CHECK(iter->symbol == lexy::_detail::string_view("c"));
        CHECK(iter->value == 2);

        ++iter;
        CHECK(iter != table.end());
        CHECK(iter->symbol == lexy::_detail::string_view("abc"));
        CHECK(iter->value == 3);

        ++iter;
        CHECK(iter == table.end());
    }
}

namespace
{
constexpr auto symbols = lexy::symbol_table<int> //
                             .map<'A'>(0)
                             .map<'B'>(1)
                             .map<'C'>(2)
                             .map<LEXY_SYMBOL("Abc")>(3);
}

TEST_CASE("dsl::symbol(token)")
{
    constexpr auto id = token(identifier(lexy::dsl::ascii::upper, lexy::dsl::ascii::lower));

    SUBCASE("basic")
    {
        static constexpr auto rule = lexy::dsl::symbol<symbols>(id);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, int i)
            {
                if (i == 3)
                    LEXY_VERIFY_CHECK(cur - str == 3);
                else
                    LEXY_VERIFY_CHECK(cur - str == 1);
                return i;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::unknown_symbol> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);

                auto end = str;
                if ('A' <= *end && *end <= 'Z')
                {
                    ++end;
                    while ('a' <= *end && *end <= 'z')
                        ++end;
                }
                LEXY_VERIFY_CHECK(e.end() == end);

                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_token> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto a = LEXY_VERIFY("A");
        CHECK(a == 0);
        auto b = LEXY_VERIFY("B");
        CHECK(b == 1);
        auto c = LEXY_VERIFY("C");
        CHECK(c == 2);

        auto abc = LEXY_VERIFY("Abc");
        CHECK(abc == 3);

        auto unknown = LEXY_VERIFY("Unknown");
        CHECK(unknown == -1);
        auto non_alpha = LEXY_VERIFY("123");
        CHECK(non_alpha == -2);
    }
    SUBCASE("branch")
    {
        static constexpr auto rule = opt(lexy::dsl::symbol<symbols>(id));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::nullopt)
            {
                LEXY_VERIFY_CHECK(cur == str);
                return 42;
            }
            LEXY_VERIFY_FN int success(const char* cur, int i)
            {
                if (i == 3)
                    LEXY_VERIFY_CHECK(cur - str == 3);
                else
                    LEXY_VERIFY_CHECK(cur - str == 1);
                return i;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 42);

        auto a = LEXY_VERIFY("A");
        CHECK(a == 0);
        auto b = LEXY_VERIFY("B");
        CHECK(b == 1);
        auto c = LEXY_VERIFY("C");
        CHECK(c == 2);

        auto abc = LEXY_VERIFY("Abc");
        CHECK(abc == 3);

        auto unknown = LEXY_VERIFY("Unknown");
        CHECK(unknown == 42);
        auto non_alpha = LEXY_VERIFY("123");
        CHECK(non_alpha == 42);
    }

    SUBCASE(".error")
    {
        static constexpr auto rule = lexy::dsl::symbol<symbols>(id).error<struct tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, int i)
            {
                LEXY_VERIFY_CHECK(cur - str == 1);
                return i;
            }

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);

                auto end = str;
                if ('A' <= *end && *end <= 'Z')
                {
                    ++end;
                    while ('a' <= *end && *end <= 'z')
                        ++end;
                }
                LEXY_VERIFY_CHECK(e.end() == end);

                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_token> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto a = LEXY_VERIFY("A");
        CHECK(a == 0);
        auto b = LEXY_VERIFY("B");
        CHECK(b == 1);

        auto unknown = LEXY_VERIFY("Unknown");
        CHECK(unknown == -1);
        auto non_alpha = LEXY_VERIFY("123");
        CHECK(non_alpha == -2);
    }
}

TEST_CASE("dsl::symbol(identifier)")
{
    constexpr auto id = identifier(lexy::dsl::ascii::upper, lexy::dsl::ascii::lower);

    SUBCASE("basic")
    {
        static constexpr auto rule = lexy::dsl::symbol<symbols>(id);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, int i)
            {
                if (i == 3)
                    LEXY_VERIFY_CHECK(cur - str == 3);
                else
                    LEXY_VERIFY_CHECK(cur - str == 1);
                return i;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::unknown_symbol> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);

                auto end = str;
                if ('A' <= *end && *end <= 'Z')
                {
                    ++end;
                    while ('a' <= *end && *end <= 'z')
                        ++end;
                }
                LEXY_VERIFY_CHECK(e.end() == end);

                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.upper"));
                LEXY_VERIFY_CHECK(e.position() == str);
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto a = LEXY_VERIFY("A");
        CHECK(a == 0);
        auto b = LEXY_VERIFY("B");
        CHECK(b == 1);
        auto c = LEXY_VERIFY("C");
        CHECK(c == 2);

        auto abc = LEXY_VERIFY("Abc");
        CHECK(abc == 3);

        auto unknown = LEXY_VERIFY("Unknown");
        CHECK(unknown == -1);
        auto non_alpha = LEXY_VERIFY("123");
        CHECK(non_alpha == -2);
    }
    SUBCASE("branch")
    {
        static constexpr auto rule = opt(lexy::dsl::symbol<symbols>(id));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::nullopt)
            {
                LEXY_VERIFY_CHECK(cur == str);
                return 42;
            }
            LEXY_VERIFY_FN int success(const char* cur, int i)
            {
                if (i == 3)
                    LEXY_VERIFY_CHECK(cur - str == 3);
                else
                    LEXY_VERIFY_CHECK(cur - str == 1);
                return i;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 42);

        auto a = LEXY_VERIFY("A");
        CHECK(a == 0);
        auto b = LEXY_VERIFY("B");
        CHECK(b == 1);
        auto c = LEXY_VERIFY("C");
        CHECK(c == 2);

        auto abc = LEXY_VERIFY("Abc");
        CHECK(abc == 3);

        auto unknown = LEXY_VERIFY("Unknown");
        CHECK(unknown == 42);
        auto non_alpha = LEXY_VERIFY("123");
        CHECK(non_alpha == 42);
    }

    SUBCASE(".error")
    {
        static constexpr auto rule = lexy::dsl::symbol<symbols>(id).error<struct tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, int i)
            {
                LEXY_VERIFY_CHECK(cur - str == 1);
                return i;
            }

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);

                auto end = str;
                if ('A' <= *end && *end <= 'Z')
                {
                    ++end;
                    while ('a' <= *end && *end <= 'z')
                        ++end;
                }
                LEXY_VERIFY_CHECK(e.end() == end);

                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.upper"));
                LEXY_VERIFY_CHECK(e.position() == str);
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto a = LEXY_VERIFY("A");
        CHECK(a == 0);
        auto b = LEXY_VERIFY("B");
        CHECK(b == 1);

        auto unknown = LEXY_VERIFY("Unknown");
        CHECK(unknown == -1);
        auto non_alpha = LEXY_VERIFY("123");
        CHECK(non_alpha == -2);
    }
}
