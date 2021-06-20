// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/ascii.hpp>

#include "verify.hpp"
#include <cctype>

TEST_CASE("dsl::ascii::*")
{
    SUBCASE("control")
    {
        static constexpr auto rule = lexy::dsl::ascii::control;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class()
                                  == lexy::_detail::string_view("ASCII.control"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::iscntrl(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("blank")
    {
        static constexpr auto rule = lexy::dsl::ascii::blank;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.blank"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isblank(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("newline")
    {
        static constexpr auto rule = lexy::dsl::ascii::newline;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class()
                                  == lexy::_detail::string_view("ASCII.newline"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = (c == '\n' || c == '\r') ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("other_space")
    {
        static constexpr auto rule = lexy::dsl::ascii::other_space;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class()
                                  == lexy::_detail::string_view("ASCII.other-space"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = (c == '\f' || c == '\v') ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("space")
    {
        static constexpr auto rule = lexy::dsl::ascii::space;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.space"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isspace(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("lower")
    {
        static constexpr auto rule = lexy::dsl::ascii::lower;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.lower"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::islower(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("upper")
    {
        static constexpr auto rule = lexy::dsl::ascii::upper;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
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

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isupper(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("alpha")
    {
        static constexpr auto rule = lexy::dsl::ascii::alpha;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
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

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isalpha(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("alpha_underscore")
    {
        static constexpr auto rule = lexy::dsl::ascii::alpha_underscore;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class()
                                  == lexy::_detail::string_view("ASCII.alpha-underscore"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isalpha(c) || c == '_' ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("digit")
    {
        static constexpr auto rule = lexy::dsl::ascii::digit;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.digit"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isdigit(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("alpha_digit")
    {
        static constexpr auto rule = lexy::dsl::ascii::alpha_digit;
        CHECK(std::is_same_v<decltype(rule), decltype(lexy::dsl::ascii::alnum)>);
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class()
                                  == lexy::_detail::string_view("ASCII.alpha-digit"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isalnum(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("alpha_digit_underscore")
    {
        static constexpr auto rule = lexy::dsl::ascii::alpha_digit_underscore;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class()
                                  == lexy::_detail::string_view("ASCII.alpha-digit-underscore"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isalnum(c) || c == '_' ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("punct")
    {
        static constexpr auto rule = lexy::dsl::ascii::punct;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.punct"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::ispunct(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("graph")
    {
        static constexpr auto rule = lexy::dsl::ascii::graph;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.graph"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isgraph(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("print")
    {
        static constexpr auto rule = lexy::dsl::ascii::print;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII.print"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isprint(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("character")
    {
        static constexpr auto rule = lexy::dsl::ascii::character;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str + 1);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[] = {char(c), char(c)};
            CHECK(verify<callback>(rule, str, 2) == 0);
        }
    }
}

