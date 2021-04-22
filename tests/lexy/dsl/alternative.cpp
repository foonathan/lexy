// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/alternative.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/while.hpp>

TEST_CASE("dsl::operator/")
{
    SUBCASE("literals only")
    {
        static constexpr auto rule
            = LEXY_LIT("abc") / LEXY_LIT("a") / LEXY_LIT("ab") / LEXY_LIT("def");
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
                else if (match == "ab")
                    return 2;
                else if (match == "def")
                    return 3;
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
        CHECK(ab == 2);

        auto def = LEXY_VERIFY("def");
        CHECK(def == 3);
    }
    SUBCASE("non-literals only")
    {
        static constexpr auto rule = lexy::dsl::ascii::alnum
                                     / token(while_one(lexy::dsl::lit_c<'a'>))
                                     / token(while_one(lexy::dsl::ascii::alpha));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                auto match = lexy::_detail::string_view(str, cur);
                return int(match.size());
            }

            LEXY_VERIFY_FN int error(test_error<lexy::exhausted_alternatives> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto aa = LEXY_VERIFY("aa");
        CHECK(aa == 2);
        auto aaa = LEXY_VERIFY("aaa");
        CHECK(aaa == 3);

        auto aabbcc = LEXY_VERIFY("aabbcc");
        CHECK(aabbcc == 6);

        auto digit = LEXY_VERIFY("1");
        CHECK(digit == 1);
    }
    SUBCASE("mixed")
    {
        static constexpr auto rule = LEXY_LIT("abc") / lexy::dsl::ascii::digit
                                     / token(while_one(lexy::dsl::lit_c<'a'>)) / LEXY_LIT("ab")
                                     / LEXY_LIT("def");
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                auto match = lexy::_detail::string_view(str, cur);
                return int(match.size());
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
        CHECK(abc == 3);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto aa = LEXY_VERIFY("aa");
        CHECK(aa == 2);
        auto aaa = LEXY_VERIFY("aaa");
        CHECK(aaa == 3);
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == 2);

        auto def = LEXY_VERIFY("def");
        CHECK(def == 3);

        auto digit = LEXY_VERIFY("4");
        CHECK(digit == 1);
    }
}

