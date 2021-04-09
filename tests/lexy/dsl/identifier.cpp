// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/identifier.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>

TEST_CASE("dsl::identifier()")
{
    SUBCASE("token")
    {
        static constexpr auto rule = identifier(lexy::dsl::ascii::alpha);
        CHECK(lexy::is_rule<decltype(rule)>);

        CHECK(std::is_same_v<const decltype(rule.leading_pattern()),
                             decltype(lexy::dsl::ascii::alpha)>);
        CHECK(std::is_same_v<const decltype(rule.trailing_pattern()),
                             decltype(lexy::dsl::ascii::alpha)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> id)
            {
                LEXY_VERIFY_CHECK(id.begin() == str);
                LEXY_VERIFY_CHECK(id.end() == cur);
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.alpha"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == 2);
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 3);

        auto abc1 = LEXY_VERIFY("abc1");
        CHECK(abc1 == 3);
    }
    SUBCASE("leading and trailing token")
    {
        static constexpr auto rule = identifier(lexy::dsl::ascii::upper, lexy::dsl::ascii::lower);
        CHECK(lexy::is_rule<decltype(rule)>);

        CHECK(std::is_same_v<const decltype(rule.leading_pattern()),
                             decltype(lexy::dsl::ascii::upper)>);
        CHECK(std::is_same_v<const decltype(rule.trailing_pattern()),
                             decltype(lexy::dsl::ascii::lower)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> id)
            {
                LEXY_VERIFY_CHECK(id.begin() == str);
                LEXY_VERIFY_CHECK(id.end() == cur);
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.upper"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == -1);
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == -1);
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == -1);

        auto A = LEXY_VERIFY("A");
        CHECK(A == 1);
        auto Ab = LEXY_VERIFY("Ab");
        CHECK(Ab == 2);
        auto Abc = LEXY_VERIFY("Abc");
        CHECK(Abc == 3);

        auto abc1 = LEXY_VERIFY("Abc1");
        CHECK(abc1 == 3);
        auto abcA = LEXY_VERIFY("AbcA");
        CHECK(abcA == 3);
    }

    SUBCASE(".reserve()")
    {
        static constexpr auto rule
            = identifier(lexy::dsl::ascii::alpha)
                  .reserve(LEXY_LIT("ab"), LEXY_KEYWORD("abc", identifier(lexy::dsl::ascii::alpha)))
                  .reserve(LEXY_LIT("int"));
        CHECK(lexy::is_rule<decltype(rule)>);

        CHECK(std::is_same_v<const decltype(rule.leading_pattern()),
                             decltype(lexy::dsl::ascii::alpha)>);
        CHECK(std::is_same_v<const decltype(rule.trailing_pattern()),
                             decltype(lexy::dsl::ascii::alpha)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> id)
            {
                LEXY_VERIFY_CHECK(id.begin() == str);
                LEXY_VERIFY_CHECK(id.end() == cur);
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.alpha"));
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::reserved_identifier> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto abcd = LEXY_VERIFY("abcd");
        CHECK(abcd == 4);
        auto integer = LEXY_VERIFY("integer");
        CHECK(integer == 7);

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.value == 2);
        CHECK(ab.errors(-2));
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.value == 3);
        CHECK(abc.errors(-2));
        auto int_ = LEXY_VERIFY("int");
        CHECK(int_.value == 3);
        CHECK(int_.errors(-2));

        auto abcd1 = LEXY_VERIFY("abcd1");
        CHECK(abcd1 == 4);
    }
    SUBCASE(".reserve_prefix()")
    {
        static constexpr auto rule
            = identifier(lexy::dsl::ascii::alpha).reserve_prefix(LEXY_LIT("ab"));
        CHECK(lexy::is_rule<decltype(rule)>);

        CHECK(std::is_same_v<const decltype(rule.leading_pattern()),
                             decltype(lexy::dsl::ascii::alpha)>);
        CHECK(std::is_same_v<const decltype(rule.trailing_pattern()),
                             decltype(lexy::dsl::ascii::alpha)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> id)
            {
                LEXY_VERIFY_CHECK(id.begin() == str);
                LEXY_VERIFY_CHECK(id.end() == cur);
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.alpha"));
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::reserved_identifier> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.value == 2);
        CHECK(ab.errors(-2));
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.value == 3);
        CHECK(abc.errors(-2));

        auto a1 = LEXY_VERIFY("a1");
        CHECK(a1 == 1);
    }
    SUBCASE(".reserve_containing()")
    {
        static constexpr auto rule
            = identifier(lexy::dsl::ascii::alpha).reserve_containing(LEXY_LIT("b"));
        CHECK(lexy::is_rule<decltype(rule)>);

        CHECK(std::is_same_v<const decltype(rule.leading_pattern()),
                             decltype(lexy::dsl::ascii::alpha)>);
        CHECK(std::is_same_v<const decltype(rule.trailing_pattern()),
                             decltype(lexy::dsl::ascii::alpha)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> id)
            {
                LEXY_VERIFY_CHECK(id.begin() == str);
                LEXY_VERIFY_CHECK(id.end() == cur);
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.alpha"));
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::reserved_identifier> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);

        auto b = LEXY_VERIFY("b");
        CHECK(b.value == 1);
        CHECK(b.errors(-2));
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.value == 2);
        CHECK(ab.errors(-2));
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.value == 3);
        CHECK(abc.errors(-2));

        auto a1 = LEXY_VERIFY("a1");
        CHECK(a1 == 1);
    }

    SUBCASE(".pattern()")
    {
        static constexpr auto rule = identifier(lexy::dsl::ascii::alpha)
                                         .reserve(LEXY_LIT("ab"), LEXY_LIT("abc"))
                                         .pattern();
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.alpha"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == 2);
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 3);

        auto abc1 = LEXY_VERIFY("abc1");
        CHECK(abc1 == 3);
    }
}

TEST_CASE("dsl::keyword")
{
    static constexpr auto id = identifier(lexy::dsl::ascii::alpha);

    static constexpr auto rule = LEXY_KEYWORD("abc", id);
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

#if LEXY_HAS_NTTP
    CHECK(std::is_same_v<decltype(rule), const decltype(lexy::dsl::keyword<"abc">(id))>);
#endif

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            LEXY_VERIFY_CHECK(str + 3 == cur);
            return 0;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_keyword> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            LEXY_VERIFY_CHECK(e.begin() == str);
            LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
            LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto a = LEXY_VERIFY("a");
    CHECK(a == -1);
    auto ab = LEXY_VERIFY("ab");
    CHECK(ab == -1);

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 0);
    auto abc1 = LEXY_VERIFY("abc1");
    CHECK(abc1 == 0);

    auto abcd = LEXY_VERIFY("abcd");
    CHECK(abcd == -1);
}

