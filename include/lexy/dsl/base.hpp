// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
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
    template <typename Reader, typename Error>
    result_type error(const Reader& reader, Error&& error) &&;

    /// Called when parsing was succesful.
    template <typename ... Args>
    result_type value(Args&&... args) &&;
};

struct Rule : rule_base
{
    // Whether or not the rule has a matcher.
    // A rule with matcher is a pattern.
    static constexpr bool has_matcher;

    struct matcher
    {
        // If matches, consumes characters from reader and return true.
        // If it doesn't match, leave reader as-is and return false.
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader);
    };

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

// We use a shorthand namespace to decrease symbol size.
namespace lexyd
{
struct rule_base
{};
} // namespace lexyd

namespace lexy
{
namespace dsl = lexyd;

template <typename T>
constexpr bool is_rule = std::is_base_of_v<dsl::rule_base, T>;

template <typename T>
constexpr bool is_pattern = [] {
    if constexpr (is_rule<T>)
        return T::has_matcher;
    else
        return false;
}();
} // namespace lexy

//=== atom ===//
#if 0
class Atom : atom_base<Atom>
{
    // Try to match and consume characters.
    // Returns true, if match succesful and leave reader after the consumed characters.
    // Otherwise, return false and leaves reader at the error position.
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader);

    // Returns an error object describing the error.
    // The reader is in the state the match function left it, `pos` is the position before calling match.
    template <typename Reader>
    LEXY_DSL_FUNC auto error(const Reader& reader, typename Reader::iterator pos);
};
#endif

namespace lexyd
{
template <typename Atom>
struct atom_base : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            auto reset = reader;
            if (Atom::match(reader))
                return true;

            reader = LEXY_MOV(reset);
            return false;
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if constexpr (std::is_same_v<decltype(Atom::error(reader, reader.cur())), void>)
            {
                Atom::match(reader);
                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
            }
            else
            {
                if (auto pos = reader.cur(); Atom::match(reader))
                    return NextParser::parse(handler, reader, LEXY_FWD(args)...);
                else
                    return LEXY_MOV(handler).error(reader, Atom::error(reader, pos));
            }
        }
    };
};
} // namespace lexyd

//=== infrastructure ===//
namespace lexy
{
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

#endif // LEXY_DSL_BASE_HPP_INCLUDED

