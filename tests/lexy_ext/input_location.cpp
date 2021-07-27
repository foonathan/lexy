// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy_ext/input_location.hpp>

#include <doctest/doctest.h>
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
        return lexy::_detail::string_view(lex.begin(), lex.end()) == context.str;
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

TEST_CASE("input_location_finder")
{
    constexpr auto character = lexy::dsl::ascii::character;
    constexpr auto newline   = lexy::dsl::ascii::newline;

    auto                            input = lexy::zstring_input("Line 1\n"
                                     "Line 2\n"
                                     "Line 3\n");
    lexy_ext::input_location_finder finder(input, character, newline);

    // Mainly do tests for anchors here, full search done below.
    auto a = finder.find(input.data() + 3);
    CHECK(a.line_nr() == 1);
    CHECK(a.column_nr() == 4);
    CHECK(a.context() == str_context{"Line 1"});
    CHECK(a.newline() == str_context{"\n"});

    auto b = finder.find(input.data() + 5, a);
    CHECK(b.line_nr() == 1);
    CHECK(b.column_nr() == 6);
    CHECK(b.context() == str_context{"Line 1"});
    CHECK(b.newline() == str_context{"\n"});

    // Note: anchor is after, but still the same line.
    auto c = finder.find(input.data(), b);
    CHECK(c.line_nr() == 1);
    CHECK(c.column_nr() == 1);
    CHECK(c.context() == str_context{"Line 1"});
    CHECK(c.newline() == str_context{"\n"});
}

TEST_CASE("find_input_location")
{
    constexpr auto character = lexy::dsl::ascii::character;
    constexpr auto newline   = lexy::dsl::ascii::newline;

    SUBCASE("basic")
    {
        TEST_CONSTEXPR auto input = lexy::zstring_input("Line 1\n"
                                                        "Line 2\n"
                                                        "Line 3\n");

        TEST_CONSTEXPR auto first_1
            = lexy_ext::find_input_location(input, input.data(), character, newline);
        CHECK(first_1.line_nr() == 1);
        CHECK(first_1.column_nr() == 1);
        CHECK(first_1.context() == str_context{"Line 1"});

        TEST_CONSTEXPR auto first_2
            = lexy_ext::find_input_location(input, input.data() + 4, character, newline);
        CHECK(first_2.line_nr() == 1);
        CHECK(first_2.column_nr() == 5);
        CHECK(first_2.context() == str_context{"Line 1"});

        TEST_CONSTEXPR auto second
            = lexy_ext::find_input_location(input, input.data() + 10, character, newline);
        CHECK(second.line_nr() == 2);
        CHECK(second.column_nr() == 4);
        CHECK(second.context() == str_context{"Line 2"});

        TEST_CONSTEXPR auto third
            = lexy_ext::find_input_location(input, input.data() + 17, character, newline);
        CHECK(third.line_nr() == 3);
        CHECK(third.column_nr() == 4);
        CHECK(third.context() == str_context{"Line 3"});

        TEST_CONSTEXPR auto nl
            = lexy_ext::find_input_location(input, input.data() + 6, character, newline);
        CHECK(input.data()[6] == '\n');
        CHECK(nl.line_nr() == 1);
        CHECK(nl.column_nr() == 7);
        CHECK(nl.context() == str_context{"Line 1"});

        TEST_CONSTEXPR auto last
            = lexy_ext::find_input_location(input, input.data() + input.size() - 1, character,
                                            newline);
        CHECK(last.line_nr() == 3);
        CHECK(last.column_nr() == 7);
        CHECK(last.context() == str_context{"Line 3"});
        CHECK(last.newline() == str_context{"\n"});

        TEST_CONSTEXPR auto end
            = lexy_ext::find_input_location(input, input.data() + input.size(), character, newline);
        CHECK(end.line_nr() == 4);
        CHECK(end.column_nr() == 1);
        CHECK(end.context().empty());
        CHECK(end.newline().empty());
    }
    SUBCASE("no trailing newline")
    {
        TEST_CONSTEXPR auto input = lexy::zstring_input("Line 1\n"
                                                        "Line 2\n"
                                                        "Line 3");

        TEST_CONSTEXPR auto first_1
            = lexy_ext::find_input_location(input, input.data(), character, newline);
        CHECK(first_1.line_nr() == 1);
        CHECK(first_1.column_nr() == 1);
        CHECK(first_1.context() == str_context{"Line 1"});

        TEST_CONSTEXPR auto first_2
            = lexy_ext::find_input_location(input, input.data() + 4, character, newline);
        CHECK(first_2.line_nr() == 1);
        CHECK(first_2.column_nr() == 5);
        CHECK(first_2.context() == str_context{"Line 1"});

        TEST_CONSTEXPR auto second
            = lexy_ext::find_input_location(input, input.data() + 10, character, newline);
        CHECK(second.line_nr() == 2);
        CHECK(second.column_nr() == 4);
        CHECK(second.context() == str_context{"Line 2"});

        TEST_CONSTEXPR auto third
            = lexy_ext::find_input_location(input, input.data() + 17, character, newline);
        CHECK(third.line_nr() == 3);
        CHECK(third.column_nr() == 4);
        CHECK(third.context() == str_context{"Line 3"});

        TEST_CONSTEXPR auto nl
            = lexy_ext::find_input_location(input, input.data() + 6, character, newline);
        CHECK(input.data()[6] == '\n');
        CHECK(nl.line_nr() == 1);
        CHECK(nl.column_nr() == 7);
        CHECK(nl.context() == str_context{"Line 1"});

        TEST_CONSTEXPR auto last
            = lexy_ext::find_input_location(input, input.data() + input.size() - 1, character,
                                            newline);
        CHECK(last.line_nr() == 3);
        CHECK(last.column_nr() == 6);
        CHECK(last.context() == str_context{"Line 3"});
        CHECK(last.newline().empty());

        TEST_CONSTEXPR auto end
            = lexy_ext::find_input_location(input, input.data() + input.size(), character, newline);
        CHECK(end.line_nr() == 3);
        CHECK(end.column_nr() == 7);
        CHECK(end.context() == str_context{"Line 3"});
        CHECK(end.newline().empty());
    }
    SUBCASE("weird character")
    {
        // 0xFF isn't an ascii character
        static constexpr char array[] = {'a', 'b', 'c', char(0xFF), 'd', '\0'};
        TEST_CONSTEXPR auto   input   = lexy::zstring_input(array);

        TEST_CONSTEXPR auto before
            = lexy_ext::find_input_location(input, input.data() + 2, character, newline);
        CHECK(before.line_nr() == 1);
        CHECK(before.column_nr() == 3);
        CHECK(before.context().size() == 5);

        TEST_CONSTEXPR auto weird
            = lexy_ext::find_input_location(input, input.data() + 3, character, newline);
        CHECK(weird.line_nr() == 1);
        CHECK(weird.column_nr() == 4);
        CHECK(weird.context().size() == 5);

        TEST_CONSTEXPR auto after
            = lexy_ext::find_input_location(input, input.data() + 4, character, newline);
        CHECK(after.line_nr() == 1);
        CHECK(after.column_nr() == 4);
        CHECK(after.context().size() == 5);
    }

    SUBCASE("weird character, default")
    {
        // 0xFF isn't an ascii character
        static constexpr char array[] = {'a', 'b', 'c', char(0xFF), 'd', '\0'};
        TEST_CONSTEXPR auto   input   = lexy::zstring_input(array);

        TEST_CONSTEXPR auto before = lexy_ext::find_input_location(input, input.data() + 2);
        CHECK(before.line_nr() == 1);
        CHECK(before.column_nr() == 3);
        CHECK(before.context().size() == 5);

        TEST_CONSTEXPR auto weird = lexy_ext::find_input_location(input, input.data() + 3);
        CHECK(weird.line_nr() == 1);
        CHECK(weird.column_nr() == 4);
        CHECK(weird.context().size() == 5);

        TEST_CONSTEXPR auto after = lexy_ext::find_input_location(input, input.data() + 4);
        CHECK(after.line_nr() == 1);
        CHECK(after.column_nr() == 5);
        CHECK(after.context().size() == 5);
    }
}

