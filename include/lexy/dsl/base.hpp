// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/engine/base.hpp>
#include <lexy/error.hpp>
#include <lexy/input/base.hpp>

#define LEXY_DSL_FUNC LEXY_FORCE_INLINE static constexpr

//=== rule ===//
#if 0
struct Handler
{
    using result_type = ...;

    /// Returns a sub-handler for an inner production.
    template <typename Production, typename Reader>
    ParseHandler sub_handler(const Reader& reader);

    /// Returns a sink to construct a list.
    Sink list_sink();

    /// Called when an error ocurred.
    template <typename Error>
    result_type error(Error&& error) &&;

    /// Called when parsing was succesful.
    template <typename ... Args>
    result_type value(Args&&... args) &&;
};

struct Rule : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename ... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args)
            -> typename Handler::result_type
        {
            if (/* match reader */)
                return NextParser::parse(handler, reader, LEXY_FWD(args)..., /* rule arguments */);
            else
                return LEXY_MOV(handler).error(reader, /* error */);
        }
    };
};
#endif

namespace lexyd
{
struct rule_base
{
    static constexpr auto is_branch = false;
};

struct branch_base : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    using parser = NextParser;
};

struct _token_base : rule_base
{};
} // namespace lexyd

namespace lexy
{
// We use a shorthand namespace to decrease symbol size.
namespace dsl = lexyd;

template <typename T>
constexpr bool is_rule = std::is_base_of_v<dsl::rule_base, T>;
template <typename T>
constexpr bool is_token = std::is_base_of_v<dsl::_token_base, T>;

template <typename T>
constexpr bool is_branch = [] {
    if constexpr (is_rule<T>)
        return T::is_branch;
    else
        return false;
}();
} // namespace lexy

namespace lexy
{
template <typename Rule, typename NextParser>
using rule_parser = typename Rule::template parser<NextParser>;
template <typename Branch, typename Reader>
using branch_matcher = typename Branch::template branch_matcher<Reader>;

/// The final parser in the chain of NextParsers, forwarding everything to the handler.
struct final_parser
{
    template <typename Handler, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader&, Args&&... args) ->
        typename Handler::result_type
    {
        return LEXY_MOV(handler).value(LEXY_FWD(args)...);
    }
};
} // namespace lexy

//=== token ===//
namespace lexyd
{
template <typename Derived>
struct token_base : _token_base
{
    static constexpr auto is_branch = true;

    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = false;

        constexpr bool match(Reader& reader)
        {
            return lexy::engine_try_match<typename Derived::token_engine>(reader);
        }

        template <typename NextParser, typename Handler, typename... Args>
        constexpr auto parse(Handler& handler, Reader& reader, Args&&... args)
        {
            return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            using token_engine = typename Derived::token_engine;
            if constexpr (lexy::engine_can_fail<token_engine, Reader>)
            {
                auto position = reader.cur();
                if (auto ec = token_engine::match(reader);
                    ec == typename token_engine::error_code())
                    return NextParser::parse(handler, reader, LEXY_FWD(args)...);
                else
                    return Derived::token_error(handler, reader, ec, position);
            }
            else
            {
                token_engine::match(reader);
                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
            }
        }
    };

    //=== dsl ===//
    template <typename Tag>
    LEXY_CONSTEVAL auto error() const;
};
} // namespace lexyd

#endif // LEXY_DSL_BASE_HPP_INCLUDED

