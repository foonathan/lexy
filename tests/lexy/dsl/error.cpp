// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/error.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>

TEST_CASE(".error")
{
    struct error;
    static constexpr auto rule = LEXY_LIT("abc").error<error>;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            LEXY_VERIFY_CHECK(cur - str == 3);
            return 0;
        }

        LEXY_VERIFY_FN int error(test_error<error> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 0);
}

TEST_CASE("dsl::error")
{
    struct tag;

    SUBCASE("no range")
    {
        static constexpr auto rule = lexy::dsl::error<tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == -1);
    }
    SUBCASE("range")
    {
        static constexpr auto rule = lexy::dsl::error<tag>(LEXY_LIT("ab") + LEXY_LIT("c"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                if (e.begin() != e.end())
                {
                    LEXY_VERIFY_CHECK(e.begin() == str);
                    LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                    return -1;
                }
                else
                {
                    LEXY_VERIFY_CHECK(e.position() == str);
                    return -2;
                }
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == -1);
    }
}

TEST_CASE("dsl::require")
{
    struct tag;
    static constexpr auto rule = lexy::dsl::require(LEXY_LIT("ab") + LEXY_LIT("c")).error<tag>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            LEXY_VERIFY_CHECK(cur == str);
            return 0;
        }

        LEXY_VERIFY_FN int error(test_error<tag> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.value == 0);
    CHECK(empty.errors(-1));

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 0);
}

TEST_CASE("dsl::prevent")
{
    struct tag;
    static constexpr auto rule
        = lexy::dsl::prevent(LEXY_LIT("ab") + LEXY_LIT("c")).error<tag> + lexy::dsl::ascii::alpha;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str);
        }

        LEXY_VERIFY_FN int error(test_error<tag> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            return -1;
        }
        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.alpha"));
            return -2;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -2);

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.value == 1);
    CHECK(abc.errors(-1));

    auto d = LEXY_VERIFY("d");
    CHECK(d == 1);
}

