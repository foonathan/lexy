// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/digits.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/engine/char_class.hpp>
#include <lexy/engine/literal.hpp>

namespace
{
using digit_set = lexy::engine_char_range<'0', '9'>;

constexpr auto zero_trie = lexy::linear_trie<LEXY_NTTP_STRING("0")>;
using zero               = lexy::engine_literal<zero_trie>;

constexpr auto sep_trie = lexy::linear_trie<LEXY_NTTP_STRING("--")>;
using sep               = lexy::engine_literal<sep_trie>;
} // namespace

TEST_CASE("engine_digits")
{
    using engine = lexy::engine_digits<digit_set>;
    CHECK(lexy::engine_is_matcher<engine>);

    constexpr auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.ec == digit_set::error_code::error);

    constexpr auto zero = engine_matches<engine>("0");
    CHECK(zero);
    CHECK(zero.count == 1);

    constexpr auto one = engine_matches<engine>("1");
    CHECK(one);
    CHECK(one.count == 1);
    constexpr auto one_zero_one = engine_matches<engine>("101");
    CHECK(one_zero_one);
    CHECK(one_zero_one.count == 3);

    constexpr auto zero_zero_seven = engine_matches<engine>("007");
    CHECK(zero_zero_seven);
    CHECK(zero_zero_seven.count == 3);
}

TEST_CASE("engine_digits_sep")
{
    using engine = lexy::engine_digits_sep<digit_set, sep>;
    CHECK(lexy::engine_is_matcher<engine>);

    constexpr auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.ec == digit_set::error_code::error);

    constexpr auto zero = engine_matches<engine>("0");
    CHECK(zero);
    CHECK(zero.count == 1);

    constexpr auto one = engine_matches<engine>("1");
    CHECK(one);
    CHECK(one.count == 1);
    constexpr auto one_zero_one = engine_matches<engine>("101");
    CHECK(one_zero_one);
    CHECK(one_zero_one.count == 3);

    constexpr auto zero_zero_seven = engine_matches<engine>("007");
    CHECK(zero_zero_seven);
    CHECK(zero_zero_seven.count == 3);

    constexpr auto with_sep = engine_matches<engine>("1--0--7");
    CHECK(with_sep);
    CHECK(with_sep.count == 7);

    constexpr auto leading_sep = engine_matches<engine>("--101");
    CHECK(!leading_sep);
    CHECK(leading_sep.count == 0);
    CHECK(leading_sep.ec == digit_set::error_code::error);
    constexpr auto trailing_sep = engine_matches<engine>("101--");
    CHECK(!trailing_sep);
    CHECK(trailing_sep.count == 5);
    CHECK(trailing_sep.ec == digit_set::error_code::error);

    constexpr auto partial_sep = engine_matches<engine>("1-01");
    CHECK(partial_sep);
    CHECK(partial_sep.count == 1);
}

TEST_CASE("engine_digits_trimmed")
{
    using engine = lexy::engine_digits_trimmed<digit_set, zero>;
    CHECK(lexy::engine_is_matcher<engine>);

    constexpr auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.ec == engine::translate(digit_set::error_code::error));

    constexpr auto zero = engine_matches<engine>("0");
    CHECK(zero);
    CHECK(zero.count == 1);

    constexpr auto one = engine_matches<engine>("1");
    CHECK(one);
    CHECK(one.count == 1);
    constexpr auto one_zero_one = engine_matches<engine>("101");
    CHECK(one_zero_one);
    CHECK(one_zero_one.count == 3);

    constexpr auto zero_zero_seven = engine_matches<engine>("007");
    CHECK(!zero_zero_seven);
    CHECK(zero_zero_seven.count == 1);
    CHECK(zero_zero_seven.ec == engine::error_code::leading_zero);
}

TEST_CASE("engine_digits_trimmed_sep")
{
    using engine = lexy::engine_digits_trimmed_sep<digit_set, zero, sep>;
    CHECK(lexy::engine_is_matcher<engine>);

    constexpr auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.ec == engine::translate(digit_set::error_code::error));

    constexpr auto zero = engine_matches<engine>("0");
    CHECK(zero);
    CHECK(zero.count == 1);

    constexpr auto one = engine_matches<engine>("1");
    CHECK(one);
    CHECK(one.count == 1);
    constexpr auto one_zero_one = engine_matches<engine>("101");
    CHECK(one_zero_one);
    CHECK(one_zero_one.count == 3);

    constexpr auto zero_zero_seven = engine_matches<engine>("007");
    CHECK(!zero_zero_seven);
    CHECK(zero_zero_seven.count == 1);
    CHECK(zero_zero_seven.ec == engine::error_code::leading_zero);

    constexpr auto with_sep = engine_matches<engine>("1--0--7");
    CHECK(with_sep);
    CHECK(with_sep.count == 7);

    constexpr auto leading_sep = engine_matches<engine>("--101");
    CHECK(!leading_sep);
    CHECK(leading_sep.count == 0);
    CHECK(leading_sep.ec == engine::translate(digit_set::error_code::error));
    constexpr auto trailing_sep = engine_matches<engine>("101--");
    CHECK(!trailing_sep);
    CHECK(trailing_sep.count == 5);
    CHECK(trailing_sep.ec == engine::translate(digit_set::error_code::error));

    constexpr auto partial_sep = engine_matches<engine>("1-01");
    CHECK(partial_sep);
    CHECK(partial_sep.count == 1);
}

TEST_CASE("engine_ndigits")
{
    SUBCASE("N = 1")
    {
        using engine = lexy::engine_ndigits<1, digit_set>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == digit_set::error_code::error);

        constexpr auto one = engine_matches<engine>("1");
        CHECK(one);
        CHECK(one.count == 1);
        constexpr auto two = engine_matches<engine>("12");
        CHECK(two);
        CHECK(two.count == 1);
        constexpr auto three = engine_matches<engine>("123");
        CHECK(three);
        CHECK(three.count == 1);
        constexpr auto four = engine_matches<engine>("1234");
        CHECK(four);
        CHECK(four.count == 1);
        constexpr auto five = engine_matches<engine>("12345");
        CHECK(five);
        CHECK(five.count == 1);
    }
    SUBCASE("N = 2")
    {
        using engine = lexy::engine_ndigits<2, digit_set>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == digit_set::error_code::error);

        constexpr auto one = engine_matches<engine>("1");
        CHECK(!one);
        CHECK(one.count == 1);
        CHECK(one.ec == digit_set::error_code::error);

        constexpr auto two = engine_matches<engine>("12");
        CHECK(two);
        CHECK(two.count == 2);
        constexpr auto three = engine_matches<engine>("123");
        CHECK(three);
        CHECK(three.count == 2);
        constexpr auto four = engine_matches<engine>("1234");
        CHECK(four);
        CHECK(four.count == 2);
        constexpr auto five = engine_matches<engine>("12345");
        CHECK(five);
        CHECK(five.count == 2);
    }
    SUBCASE("N = 4")
    {
        using engine = lexy::engine_ndigits<4, digit_set>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == digit_set::error_code::error);

        constexpr auto one = engine_matches<engine>("1");
        CHECK(!one);
        CHECK(one.count == 1);
        CHECK(one.ec == digit_set::error_code::error);
        constexpr auto two = engine_matches<engine>("12");
        CHECK(!two);
        CHECK(two.count == 2);
        CHECK(two.ec == digit_set::error_code::error);
        constexpr auto three = engine_matches<engine>("123");
        CHECK(!three);
        CHECK(three.count == 3);
        CHECK(three.ec == digit_set::error_code::error);

        constexpr auto four = engine_matches<engine>("1234");
        CHECK(four);
        CHECK(four.count == 4);
        constexpr auto five = engine_matches<engine>("12345");
        CHECK(five);
        CHECK(five.count == 4);
    }
    SUBCASE("N = 5")
    {
        using engine = lexy::engine_ndigits<5, digit_set>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == digit_set::error_code::error);

        constexpr auto one = engine_matches<engine>("1");
        CHECK(!one);
        CHECK(one.count == 1);
        CHECK(one.ec == digit_set::error_code::error);
        constexpr auto two = engine_matches<engine>("12");
        CHECK(!two);
        CHECK(two.count == 2);
        CHECK(two.ec == digit_set::error_code::error);
        constexpr auto three = engine_matches<engine>("123");
        CHECK(!three);
        CHECK(three.count == 3);
        CHECK(three.ec == digit_set::error_code::error);
        constexpr auto four = engine_matches<engine>("1234");
        CHECK(!four);
        CHECK(four.count == 4);
        CHECK(four.ec == digit_set::error_code::error);

        constexpr auto five = engine_matches<engine>("12345");
        CHECK(five);
        CHECK(five.count == 5);
        constexpr auto six = engine_matches<engine>("123456");
        CHECK(six);
        CHECK(six.count == 5);
    }
}

TEST_CASE("engine_ndigits_sep")
{
    SUBCASE("N = 1")
    {
        using engine = lexy::engine_ndigits_sep<1, digit_set, sep>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == digit_set::error_code::error);

        constexpr auto one = engine_matches<engine>("1");
        CHECK(one);
        CHECK(one.count == 1);
        constexpr auto two = engine_matches<engine>("12");
        CHECK(two);
        CHECK(two.count == 1);
        constexpr auto three = engine_matches<engine>("123");
        CHECK(three);
        CHECK(three.count == 1);
        constexpr auto four = engine_matches<engine>("1234");
        CHECK(four);
        CHECK(four.count == 1);
        constexpr auto five = engine_matches<engine>("12345");
        CHECK(five);
        CHECK(five.count == 1);

        constexpr auto leading_sep = engine_matches<engine>("--1");
        CHECK(!leading_sep);
        CHECK(leading_sep.count == 0);
        CHECK(leading_sep.ec == digit_set::error_code::error);
        constexpr auto trailing_sep = engine_matches<engine>("1--");
        CHECK(trailing_sep);
        CHECK(trailing_sep.count == 1);
    }
    SUBCASE("N = 2")
    {
        using engine = lexy::engine_ndigits_sep<2, digit_set, sep>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == digit_set::error_code::error);

        constexpr auto one = engine_matches<engine>("1");
        CHECK(!one);
        CHECK(one.count == 1);
        CHECK(one.ec == digit_set::error_code::error);

        constexpr auto two = engine_matches<engine>("12");
        CHECK(two);
        CHECK(two.count == 2);
        constexpr auto three = engine_matches<engine>("123");
        CHECK(three);
        CHECK(three.count == 2);
        constexpr auto four = engine_matches<engine>("1234");
        CHECK(four);
        CHECK(four.count == 2);
        constexpr auto five = engine_matches<engine>("12345");
        CHECK(five);
        CHECK(five.count == 2);

        constexpr auto with_sep = engine_matches<engine>("1--2");
        CHECK(with_sep);
        CHECK(with_sep.count == 4);

        constexpr auto leading_sep = engine_matches<engine>("--12");
        CHECK(!leading_sep);
        CHECK(leading_sep.count == 0);
        CHECK(leading_sep.ec == digit_set::error_code::error);
        constexpr auto trailing_sep = engine_matches<engine>("12--");
        CHECK(trailing_sep);
        CHECK(trailing_sep.count == 2);
    }
    SUBCASE("N = 4")
    {
        using engine = lexy::engine_ndigits_sep<4, digit_set, sep>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == digit_set::error_code::error);

        constexpr auto one = engine_matches<engine>("1");
        CHECK(!one);
        CHECK(one.count == 1);
        CHECK(one.ec == digit_set::error_code::error);
        constexpr auto two = engine_matches<engine>("12");
        CHECK(!two);
        CHECK(two.count == 2);
        CHECK(two.ec == digit_set::error_code::error);
        constexpr auto three = engine_matches<engine>("123");
        CHECK(!three);
        CHECK(three.count == 3);
        CHECK(three.ec == digit_set::error_code::error);

        constexpr auto four = engine_matches<engine>("1234");
        CHECK(four);
        CHECK(four.count == 4);
        constexpr auto five = engine_matches<engine>("12345");
        CHECK(five);
        CHECK(five.count == 4);

        constexpr auto with_sep = engine_matches<engine>("1--2--3--4");
        CHECK(with_sep);
        CHECK(with_sep.count == 10);

        constexpr auto leading_sep = engine_matches<engine>("--1234");
        CHECK(!leading_sep);
        CHECK(leading_sep.count == 0);
        CHECK(leading_sep.ec == digit_set::error_code::error);
        constexpr auto trailing_sep = engine_matches<engine>("1234--");
        CHECK(trailing_sep);
        CHECK(trailing_sep.count == 4);
    }
    SUBCASE("N = 5")
    {
        using engine = lexy::engine_ndigits_sep<5, digit_set, sep>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == digit_set::error_code::error);

        constexpr auto one = engine_matches<engine>("1");
        CHECK(!one);
        CHECK(one.count == 1);
        CHECK(one.ec == digit_set::error_code::error);
        constexpr auto two = engine_matches<engine>("12");
        CHECK(!two);
        CHECK(two.count == 2);
        CHECK(two.ec == digit_set::error_code::error);
        constexpr auto three = engine_matches<engine>("123");
        CHECK(!three);
        CHECK(three.count == 3);
        CHECK(three.ec == digit_set::error_code::error);
        constexpr auto four = engine_matches<engine>("1234");
        CHECK(!four);
        CHECK(four.count == 4);
        CHECK(four.ec == digit_set::error_code::error);

        constexpr auto five = engine_matches<engine>("12345");
        CHECK(five);
        CHECK(five.count == 5);
        constexpr auto six = engine_matches<engine>("123456");
        CHECK(six);
        CHECK(six.count == 5);

        constexpr auto with_sep = engine_matches<engine>("1--2--3--4--5");
        CHECK(with_sep);
        CHECK(with_sep.count == 13);

        constexpr auto leading_sep = engine_matches<engine>("--12345");
        CHECK(!leading_sep);
        CHECK(leading_sep.count == 0);
        CHECK(leading_sep.ec == digit_set::error_code::error);
        constexpr auto trailing_sep = engine_matches<engine>("12345--");
        CHECK(trailing_sep);
        CHECK(trailing_sep.count == 5);
    }
}

