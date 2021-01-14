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
#include <lexy/result.hpp>

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

struct test_production
{};

template <typename Callback, typename CharT, typename Root>
struct test_handler
{
    const CharT* str;

    constexpr test_handler(const CharT* str) : str(str) {}

    template <typename Production>
    using result_type_for = lexy::result<int, int>;

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
    LEXY_VERIFY_FN result_type_for<Production> finish_production(Production, int, Args&&... args)
    {
        if constexpr (std::is_same_v<Production, Root>)
        {
            auto code = Callback{str}.success(LEXY_FWD(args)...);
            return result_type_for<Production>(lexy::result_value, code);
        }
        else
        {
            auto code = Callback{str}.success(Production{}, LEXY_FWD(args)...);
            return result_type_for<Production>(lexy::result_value, code);
        }
    }

    template <typename Production, typename Error>
    LEXY_VERIFY_FN auto error(Production, int, Error&& error)
    {
        if constexpr (std::is_same_v<Production, Root>)
        {
            auto code = Callback{str}.error(LEXY_FWD(error));
            return result_type_for<Production>(lexy::result_error, code);
        }
        else
        {
            auto code = Callback{str}.error(Production{}, LEXY_FWD(error));
            return result_type_for<Production>(lexy::result_error, code);
        }
    }
};

struct test_final_rule : lexy::dsl::rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            // We sneak in the final input position.
            return NextParser::parse(context, reader, reader.cur(), LEXY_FWD(args)...);
        }
    };
};

template <typename Callback, typename Encoding = test_encoding,
          typename Production = test_production, typename CharT, typename Rule>
LEXY_VERIFY_FN int verify(Rule _rule, const CharT* str, std::size_t size = std::size_t(-1))
{
    auto input = size == std::size_t(-1) ? lexy::zstring_input<Encoding>(str)
                                         : lexy::string_input<Encoding>(str, size);

    using handler_t = test_handler<Callback, CharT, Production>;

    auto                handler = handler_t{str};
    auto                reader  = input.reader();
    lexy::parse_context context(Production{}, handler, reader.cur());

    using rule  = decltype(_rule + test_final_rule{});
    auto result = lexy::rule_parser<rule, lexy::context_value_parser>::parse(context, reader);
    if (result)
        return result.value();
    else
        return result.error();
}

#endif // TEST_DSL_VERIFY_HPP_INCLUDED

