// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/error_location.hpp>

#include <doctest/doctest.h>
#include <lexy/_detail/string_view.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl/ascii.hpp>
#include <lexy/input/string_input.hpp>

namespace
{
struct str_context
{
    const char* str;

    template <typename Reader>
    friend bool operator==(const lexy::lexeme<Reader>& lex, str_context context)
    {
        return lexy::as_string<lexy::_detail::string_view>(lex) == context.str;
    }
};
} // namespace

#if !defined(__clang__) && defined(_MSC_VER)
// https://developercommunity2.visualstudio.com/t/GF--invalidates-end-iterators-for-const/1276953
#    define TEST_CONSTEXPR
#elif !defined(__clang__) && defined(__GNUC__) && __GNUC__ < 8
// GCC 7 doesn't like this one either and fails the test.
#    define TEST_CONSTEXPR
#else
#    define TEST_CONSTEXPR constexpr
#endif

TEST_CASE("make_error_location")
{
    constexpr auto character = lexy::dsl::ascii::character;
    constexpr auto newline   = lexy::dsl::ascii::newline;

    SUBCASE("basic")
    {
        TEST_CONSTEXPR auto input = lexy::zstring_input("Line 1\n"
                                                        "Line 2\n"
                                                        "Line 3\n");

        TEST_CONSTEXPR auto first_1
            = lexy::make_error_location(input, input.begin(), character, newline);
        CHECK(first_1.line == 1);
        CHECK(first_1.column == 1);
        CHECK(first_1.context == str_context{"Line 1"});

        TEST_CONSTEXPR auto first_2
            = lexy::make_error_location(input, input.begin() + 4, character, newline);
        CHECK(first_2.line == 1);
        CHECK(first_2.column == 5);
        CHECK(first_2.context == str_context{"Line 1"});

        TEST_CONSTEXPR auto second
            = lexy::make_error_location(input, input.begin() + 10, character, newline);
        CHECK(second.line == 2);
        CHECK(second.column == 4);
        CHECK(second.context == str_context{"Line 2"});

        TEST_CONSTEXPR auto third
            = lexy::make_error_location(input, input.begin() + 17, character, newline);
        CHECK(third.line == 3);
        CHECK(third.column == 4);
        CHECK(third.context == str_context{"Line 3"});

        TEST_CONSTEXPR auto nl
            = lexy::make_error_location(input, input.begin() + 6, character, newline);
        CHECK(input.begin()[6] == '\n');
        CHECK(nl.line == 1);
        CHECK(nl.column == 7);
        CHECK(nl.context == str_context{"Line 1"});

        TEST_CONSTEXPR auto last
            = lexy::make_error_location(input, input.end() - 1, character, newline);
        CHECK(last.line == 3);
        CHECK(last.column == 7);
        CHECK(last.context == str_context{"Line 3"});

        TEST_CONSTEXPR auto end = lexy::make_error_location(input, input.end(), character, newline);
        CHECK(end.line == 4);
        CHECK(end.column == 1);
        CHECK(end.context.empty());

        // +1 is ok, input is null-terminated so it is the real one past location of the array
        TEST_CONSTEXPR auto out_of_bounds
            = lexy::make_error_location(input, input.end() + 1, character, newline);
        CHECK(out_of_bounds.line == 4);
        CHECK(out_of_bounds.column == 1);
        CHECK(out_of_bounds.context.empty());
    }
    SUBCASE("no trailing newline")
    {
        TEST_CONSTEXPR auto input = lexy::zstring_input("Line 1\n"
                                                        "Line 2\n"
                                                        "Line 3");

        TEST_CONSTEXPR auto first_1
            = lexy::make_error_location(input, input.begin(), character, newline);
        CHECK(first_1.line == 1);
        CHECK(first_1.column == 1);
        CHECK(first_1.context == str_context{"Line 1"});

        TEST_CONSTEXPR auto first_2
            = lexy::make_error_location(input, input.begin() + 4, character, newline);
        CHECK(first_2.line == 1);
        CHECK(first_2.column == 5);
        CHECK(first_2.context == str_context{"Line 1"});

        TEST_CONSTEXPR auto second
            = lexy::make_error_location(input, input.begin() + 10, character, newline);
        CHECK(second.line == 2);
        CHECK(second.column == 4);
        CHECK(second.context == str_context{"Line 2"});

        TEST_CONSTEXPR auto third
            = lexy::make_error_location(input, input.begin() + 17, character, newline);
        CHECK(third.line == 3);
        CHECK(third.column == 4);
        CHECK(third.context == str_context{"Line 3"});

        TEST_CONSTEXPR auto nl
            = lexy::make_error_location(input, input.begin() + 6, character, newline);
        CHECK(input.begin()[6] == '\n');
        CHECK(nl.line == 1);
        CHECK(nl.column == 7);
        CHECK(nl.context == str_context{"Line 1"});

        TEST_CONSTEXPR auto last
            = lexy::make_error_location(input, input.end() - 1, character, newline);
        CHECK(last.line == 3);
        CHECK(last.column == 6);
        CHECK(last.context == str_context{"Line 3"});

        TEST_CONSTEXPR auto end = lexy::make_error_location(input, input.end(), character, newline);
        CHECK(end.line == 3);
        CHECK(end.column == 7);
        CHECK(end.context == str_context{"Line 3"});
    }
    SUBCASE("weird character")
    {
        // 0xFF isn't an ascii character
        static constexpr char array[] = {'a', 'b', 'c', char(0xFF), 'd', '\0'};
        TEST_CONSTEXPR auto   input   = lexy::zstring_input(array);

        TEST_CONSTEXPR auto before
            = lexy::make_error_location(input, input.begin() + 2, character, newline);
        CHECK(before.line == 1);
        CHECK(before.column == 3);
        CHECK(before.context.size() == 5);

        TEST_CONSTEXPR auto weird
            = lexy::make_error_location(input, input.begin() + 3, character, newline);
        CHECK(weird.line == 1);
        CHECK(weird.column == 4);
        CHECK(weird.context.size() == 5);

        TEST_CONSTEXPR auto after
            = lexy::make_error_location(input, input.begin() + 4, character, newline);
        CHECK(after.line == 1);
        CHECK(after.column == 4);
        CHECK(after.context.size() == 5);
    }
}

