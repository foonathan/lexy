// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/production.hpp>

#include "verify.hpp"
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/sequence.hpp>

namespace p_basic
{
struct prod
{
    static constexpr auto rule = LEXY_LIT("abc") + lexy::dsl::id<0>;
};
} // namespace p_basic

namespace p_token
{
struct prod
{
    static constexpr auto rule = LEXY_LIT("abc");
};
} // namespace p_token

namespace p_branch
{
struct prod
{
    static constexpr auto rule = LEXY_LIT("abc") >> lexy::dsl::id<0>;
};
} // namespace p_branch

namespace p_branch_branch
{
struct prod
{
    static constexpr auto rule = lexy::dsl::p<p_branch::prod>;
};
} // namespace p_branch_branch

TEST_CASE("dsl::inline_")
{
    using namespace p_basic;
    static constexpr auto rule = lexy::dsl::inline_<prod>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur, lexy::id<0>)
        {
            LEXY_VERIFY_CHECK(cur - str == 3);
            return 0;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 0);
}

TEST_CASE("dsl::p")
{
    SUBCASE("basic")
    {
        using namespace p_basic;
        static constexpr auto rule = lexy::dsl::p<prod>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(prod, lexy::id<0>)
            {
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, int result)
            {
                LEXY_VERIFY_CHECK(cur - str == 3);
                LEXY_VERIFY_CHECK(result == 0);
                return result;
            }

            LEXY_VERIFY_FN int error(prod, test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
    }
    SUBCASE("token")
    {
        using namespace p_token;
        static constexpr auto rule = lexy::dsl::p<prod>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(prod)
            {
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, int result)
            {
                LEXY_VERIFY_CHECK(cur - str == 3);
                LEXY_VERIFY_CHECK(result == 0);
                return result;
            }

            LEXY_VERIFY_FN int error(prod, test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
    }
    SUBCASE("branch")
    {
        using namespace p_branch;
        static constexpr auto rule = lexy::dsl::p<prod> | LEXY_LIT("def") >> lexy::dsl::id<1>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(prod, lexy::id<0>)
            {
                return 0;
            }

            LEXY_VERIFY_FN int success(const char* cur, int result)
            {
                LEXY_VERIFY_CHECK(lexy::_detail::string_view(str, cur) == "abc");
                LEXY_VERIFY_CHECK(result == 0);
                return result;
            }
            LEXY_VERIFY_FN int success(const char* cur, lexy::id<1>)
            {
                LEXY_VERIFY_CHECK(lexy::_detail::string_view(str, cur) == "def");
                return 1;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::exhausted_choice> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
        auto def = LEXY_VERIFY("def");
        CHECK(def == 1);
    }
    SUBCASE("branch in branch")
    {
        using namespace p_branch_branch;
        static constexpr auto rule = lexy::dsl::p<prod> | LEXY_LIT("def") >> lexy::dsl::id<1>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(p_branch::prod, lexy::id<0>)
            {
                return 0;
            }
            LEXY_VERIFY_FN int success(prod, int i)
            {
                return i;
            }

            LEXY_VERIFY_FN int success(const char* cur, int result)
            {
                LEXY_VERIFY_CHECK(lexy::_detail::string_view(str, cur) == "abc");
                LEXY_VERIFY_CHECK(result == 0);
                return result;
            }
            LEXY_VERIFY_FN int success(const char* cur, lexy::id<1>)
            {
                LEXY_VERIFY_CHECK(lexy::_detail::string_view(str, cur) == "def");
                return 1;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::exhausted_choice> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
        auto def = LEXY_VERIFY("def");
        CHECK(def == 1);
    }
}

namespace recurse_indirect
{
struct outer;
struct inner
{
    static constexpr auto rule = lexy::dsl::recurse<outer>;
};

struct outer
{
    static constexpr auto rule = if_(LEXY_LIT("a") >> lexy::dsl::p<inner>);
};
} // namespace recurse_indirect

namespace recurse_right
{
struct prod
{
    static constexpr auto rule = if_(LEXY_LIT("a") >> lexy::dsl::recurse<prod>);
};
} // namespace recurse_right

TEST_CASE("dsl::recurse")
{
    SUBCASE("indirect recursion")
    {
        using namespace recurse_indirect;
        static constexpr auto rule = lexy::dsl::p<outer>;

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(inner, int outer_result)
            {
                return outer_result + 1;
            }
            LEXY_VERIFY_FN int success(outer)
            {
                return 0;
            }
            LEXY_VERIFY_FN int success(outer, int inner_result)
            {
                return inner_result;
            }
            LEXY_VERIFY_FN int success(const char*, int result)
            {
                return result;
            }

            int error(inner, int)
            {
                LEXY_VERIFY_CHECK(false);
                return -1;
            }
            int error(outer, int)
            {
                LEXY_VERIFY_CHECK(false);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto aa = LEXY_VERIFY("aa");
        CHECK(aa == 2);
        auto aaa = LEXY_VERIFY("aaa");
        CHECK(aaa == 3);
    }
    SUBCASE("right recursion")
    {
        using namespace recurse_right;
        static constexpr auto rule = lexy::dsl::p<prod>;

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(prod)
            {
                return 0;
            }
            LEXY_VERIFY_FN int success(prod, int result)
            {
                return result + 1;
            }
            LEXY_VERIFY_FN int success(const char*, int result)
            {
                return result;
            }

            int error(prod, int)
            {
                LEXY_VERIFY_CHECK(false);
                return -1;
            }
            int error(int)
            {
                LEXY_VERIFY_CHECK(false);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto aa = LEXY_VERIFY("aa");
        CHECK(aa == 2);
        auto aaa = LEXY_VERIFY("aaa");
        CHECK(aaa == 3);
    }
}

