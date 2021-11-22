// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/scan.hpp>

#include "verify.hpp"
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/integer.hpp>
#include <lexy/dsl/peek.hpp>
#include <lexy/dsl/production.hpp>

namespace
{
struct simple_scan : lexy::scan_production<int>, test_production
{
    // We need to ensure that the input is actually advanced.
    static constexpr auto rule = dsl::scan + dsl::eof;

    struct integer
    {
        static constexpr auto name  = "integer";
        static constexpr auto rule  = dsl::integer<int>(dsl::digits<>);
        static constexpr auto value = lexy::forward<int>;
    };

    struct invalid_integer
    {
        static constexpr auto name = "invalid-integer";
    };

    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        if (scanner.branch(LEXY_LIT("abc")) || scanner.peek(dsl::digit<>))
        {
            auto begin   = scanner.position();
            auto integer = scanner.parse(simple_scan::integer{});
            auto end     = scanner.position();
            if (!scanner)
                return {};

            if (integer.value() < 10)
                scanner.error(invalid_integer{}, begin, end);
            return integer.value();
        }
        else
        {
            // Note that we put the scanner in a failed state, but return a value nonetheless.
            // This means that parsing does not fail.
            scanner.fatal_error(lexy::expected_char_class{}, scanner.begin(), "digit");
            return 0;
        }
    }
};

struct no_value_scan : lexy::scan_production<void>, test_production
{
    struct literal
    {
        static constexpr auto name = "literal";
        static constexpr auto rule = LEXY_LIT("abc");
        // note: no value
    };

    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        scanner.parse(dsl::p<literal>);
        return true;
    }
};

struct state_scan : lexy::scan_production<const char*>, test_production
{
    // Overload is required as we also call lexy::match without a state.
    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>&)
    {
        return nullptr;
    }

    template <typename Reader, typename Context, typename State>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>&, const State& state)
    {
        // The parse state is the test handler itself.
        return state.begin();
    }
};

} // namespace

TEST_CASE("dsl::scan")
{
    // Note: most part of the scanning interface tested by lexy::scan.
    // Focus on interaction with parse events here.
    CHECK(lexy::is_rule<decltype(dsl::scan)>);

    SUBCASE("simple")
    {
        constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                      [](const char*, int value) { return value; });

        auto empty = LEXY_VERIFY_P(simple_scan, "");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit").eof());

        auto abc = LEXY_VERIFY_P(simple_scan, "abc");
        CHECK(abc.status == test_result::fatal_error);
        // clang-format off
        CHECK(abc.trace == test_trace()
                             .literal("abc")
                             .production("integer")
                                 .expected_char_class(3, "digit.decimal")
                                 .recovery().cancel()
                                 .cancel()
                             .cancel());
        // clang-format on
        auto abc_small = LEXY_VERIFY_P(simple_scan, "abc4");
        CHECK(abc_small.status == test_result::recovered_error);
        CHECK(abc_small.value == 4);
        CHECK(abc_small.trace
              == test_trace()
                     .literal("abc")
                     .production("integer")
                     .token("digits", "4")
                     .finish()
                     .error(3, 4, "invalid-integer")
                     .eof());
        auto abc_big = LEXY_VERIFY_P(simple_scan, "abc42");
        CHECK(abc_big.status == test_result::success);
        CHECK(abc_big.value == 42);
        CHECK(abc_big.trace
              == test_trace()
                     .literal("abc")
                     .production("integer")
                     .token("digits", "42")
                     .finish()
                     .eof());

        auto small = LEXY_VERIFY_P(simple_scan, "4");
        CHECK(small.status == test_result::recovered_error);
        CHECK(small.value == 4);
        CHECK(small.trace
              == test_trace()
                     .backtracked("4")
                     .production("integer")
                     .token("digits", "4")
                     .finish()
                     .error(0, 1, "invalid-integer")
                     .eof());
        auto big = LEXY_VERIFY_P(simple_scan, "42");
        CHECK(big.status == test_result::success);
        CHECK(big.value == 42);
        CHECK(big.trace
              == test_trace()
                     .backtracked("4")
                     .production("integer")
                     .token("digits", "42")
                     .finish()
                     .eof());
    }
    SUBCASE("void")
    {
        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY_P(no_value_scan, "");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.trace
              == test_trace().production("literal").expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY_P(no_value_scan, "abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().production("literal").literal("abc"));
    }
    SUBCASE("with state")
    {
        constexpr auto callback = [](const char* begin, const char* value) {
            CHECK(begin == value);
            return 0;
        };

        auto empty = LEXY_VERIFY_P(state_scan, "");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());
    }
}

