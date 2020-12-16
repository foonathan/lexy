// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/ascii.hpp>

#include "verify.hpp"
#include <cctype>

TEST_CASE("dsl::ascii::*")
{
    SUBCASE("control")
    {
        constexpr auto rule = lexy::dsl::ascii::control;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.control");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::blank;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.blank");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::newline;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.newline");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::other_space;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.other-space");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::space;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.space");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::lower;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.lower");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::upper;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.upper");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::alpha;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.alpha");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isalpha(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("digit")
    {
        constexpr auto rule = lexy::dsl::ascii::digit;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.digit");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isdigit(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("alnum")
    {
        constexpr auto rule = lexy::dsl::ascii::alnum;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.alnum");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[]  = {char(c), char(c)};
            auto       result = std::isalnum(c) ? 0 : -1;
            CHECK(verify<callback>(rule, str, 2) == result);
        }
    }
    SUBCASE("punct")
    {
        constexpr auto rule = lexy::dsl::ascii::punct;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.punct");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::graph;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.graph");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::print;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.print");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
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
        constexpr auto rule = lexy::dsl::ascii::character;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        for (auto c = 0; c <= 127; ++c)
        {
            const char str[] = {char(c), char(c)};
            CHECK(verify<callback>(rule, str, 2) == 0);
        }
    }
}

