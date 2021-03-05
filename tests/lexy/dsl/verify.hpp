// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TEST_DSL_VERIFY_HPP_INCLUDED
#define TEST_DSL_VERIFY_HPP_INCLUDED

#include <doctest/doctest.h>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/lexeme.hpp>
#include <lexy/match.hpp>

#include "../test_encoding.hpp"

#ifdef LEXY_DISABLE_CONSTEXPR_TESTS
#    define LEXY_VERIFY_CALL(...) __VA_ARGS__
#    define LEXY_VERIFY_FN
#    define LEXY_VERIFY_CHECK(...) CHECK(__VA_ARGS__)
#else

#    define LEXY_VERIFY_CALL(...)                                                                  \
        [] {                                                                                       \
            constexpr auto result = __VA_ARGS__;                                                   \
            return result;                                                                         \
        }()

#    define LEXY_VERIFY_FN constexpr

[[noreturn]] inline bool constexpr_check_failure()
{
    throw 0;
}

#    define LEXY_VERIFY_CHECK(...) ((__VA_ARGS__) ? true : constexpr_check_failure())

#endif

#define LEXY_VERIFY(...) LEXY_VERIFY_CALL(verify<callback>(rule, __VA_ARGS__))
#define LEXY_VERIFY_ENCODING(Encoding, ...)                                                        \
    LEXY_VERIFY_CALL(verify<callback, Encoding>(rule, __VA_ARGS__))
#define LEXY_VERIFY_PRODUCTION(Production, ...)                                                    \
    LEXY_VERIFY_CALL(verify<callback, test_encoding, Production>(rule, __VA_ARGS__))

template <typename Tag>
using test_error = lexy::error_for<test_input, Tag>;

inline constexpr auto test_error_count = 4;

struct test_result
{
    bool     recovered;
    int      value;
    unsigned error_count;
    int      error_code[test_error_count];

    constexpr test_result() : recovered(false), value(-1), error_count(0), error_code() {}

    constexpr bool success(int i) const
    {
        return recovered && value == i && error_count == 0;
    }
    constexpr bool fatal(int i) const
    {
        return !recovered && error_count == 1 && error_code[0] == i;
    }

    template <typename... T>
    constexpr bool errors(T... c) const
    {
        if (error_count != sizeof...(c))
            return false;

        auto i = 0;
        return ((error_code[i++] == c) && ...);
    }

    // Simple comparison can be decomposed by doctest.
    constexpr bool operator==(int i) const
    {
        if (i >= 0)
            return success(i);
        else
            return fatal(i);
    }

    friend doctest::String toString(test_result result)
    {
        doctest::String str;
        if (result.recovered)
            str += doctest::toString(result.value);
        else
            str += "error";

        if (result.error_count > 0)
        {
            str += ": ";
            for (auto i = 0u; i != result.error_count; ++i)
                str += doctest::toString(result.error_code[i]) + " ";
        }
        return str;
    }
};

struct test_production
{};

template <typename Callback, typename CharT, typename Root>
struct test_handler
{
    const CharT* str;
    test_result  result;

    constexpr test_handler(const CharT* str) : str(str) {}

    template <typename Production>
    using return_type_for = int;

    template <typename Production>
    LEXY_VERIFY_FN auto get_sink(Production)
    {
        return Callback{str}.list();
    }

    template <typename Production, typename Iterator>
    LEXY_VERIFY_FN auto start_production(Production, Iterator)
    {
        return 0;
    }

    template <typename Kind, typename Iterator>
    LEXY_VERIFY_FN void token(Kind, Iterator, Iterator)
    {}

    template <typename Production, typename... Args>
    LEXY_VERIFY_FN int finish_production(Production, int, Args&&... args)
    {
        if constexpr (std::is_same_v<Production, Root>)
            return Callback{str}.success(LEXY_FWD(args)...);
        else
            return Callback{str}.success(Production{}, LEXY_FWD(args)...);
    }
    template <typename Production>
    constexpr void backtrack_production(Production, int)
    {}

    template <typename Production, typename Error>
    LEXY_VERIFY_FN void error(Production, int, Error&& error)
    {
        LEXY_VERIFY_CHECK(result.error_count
                          < test_error_count); // Multiple errors shouldn't happen here.
        if constexpr (std::is_same_v<Production, Root>)
            result.error_code[result.error_count++] = Callback{str}.error(LEXY_FWD(error));
        else
            result.error_code[result.error_count++]
                = Callback{str}.error(Production{}, LEXY_FWD(error));
    }
};

struct test_final_rule : lexy::dsl::rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // We sneak in the final input position.
            return NextParser::parse(context, reader, reader.cur(), LEXY_FWD(args)...);
        }
    };
};

template <typename Callback, typename Encoding = test_encoding,
          typename Production = test_production, typename CharT, typename Rule>
LEXY_VERIFY_FN test_result verify(Rule _rule, const CharT* str, std::size_t size = std::size_t(-1))
{
    auto input = size == std::size_t(-1) ? lexy::zstring_input<Encoding>(str)
                                         : lexy::string_input<Encoding>(str, size);

    using handler_t = test_handler<Callback, CharT, Production>;

    auto handler = handler_t{str};
    auto reader  = input.reader();

    using rule = decltype(_rule + test_final_rule{});
    if (auto value = lexy::_detail::parse_impl<Production, rule>(handler, reader))
    {
        handler.result.recovered = true;
        handler.result.value     = *value;
    }

    return handler.result;
}

#endif // TEST_DSL_VERIFY_HPP_INCLUDED

