// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/input/base.hpp>

#define LEXY_DSL_FUNC LEXY_FORCE_INLINE static constexpr

//=== rule ===//
#if 0
struct ParseContext
{
    using result_type = ...;

    /// Returns a sub-context for an inner production.
    template <typename Production>
    ParseContext sub_context();

    /// Returns a sink to construct a list.
    Sink list_sink();

    /// Called when an error ocurred.
    template <typename Input, typename Error>
    result_type error(const Input& input, Error&& error) &&;

    /// Called when parsing was succesful.
    template <typename ... Args>
    result_type value(Args&&... args) &&;
};

struct Rule : rule_base
{
    // Whether or not the rule has a matcher, in which case it is a pattern.
    static constexpr bool has_matcher;

    struct matcher
    {
        // If matches, consumes characters from input and return true.
        // If it doesn't match, leave input as-is and return false.
        template <typename Input>
        LEXY_DSL_FUNC bool match(Input& input);
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename ... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args)
            -> typename Context::result_type
        {
            if (/* match input */)
                return NextParser::parse(context, input, LEXY_FWD(args)..., /* rule arguments */);
            else
                return LEXY_MOV(context).error(input, /* error */);
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
    // Returns true, if match succesful and leave input after the consumed characters.
    // Otherwise, return false and leaves input at the error position.
    template <typename Input>
    LEXY_DSL_FUNC bool match(Input& input);

    // Returns an error object describing the error.
    // The input is in the state the match function left it, `pos` is the position before calling match.
    template <typename Input>
    LEXY_DSL_FUNC auto error(const Input& input, typename Input::iterator pos);
};
#endif

namespace lexyd
{
struct _atom_base : rule_base
{};
template <typename Atom>
struct atom_base : _atom_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Input>
        LEXY_DSL_FUNC bool match(Input& input)
        {
            auto reset = input;
            if (Atom::match(input))
                return true;

            input = LEXY_MOV(reset);
            return false;
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            if constexpr (std::is_same_v<decltype(Atom::error(input, input.cur())), void>)
            {
                Atom::match(input);
                return NextParser::parse(context, input, LEXY_FWD(args)...);
            }
            else
            {
                if (auto pos = input.cur(); Atom::match(input))
                    return NextParser::parse(context, input, LEXY_FWD(args)...);
                else
                    return LEXY_MOV(context).error(input, Atom::error(input, pos));
            }
        }
    };
};
} // namespace lexyd

namespace lexy
{
template <typename T>
constexpr bool is_atom = std::is_base_of_v<dsl::_atom_base, T>;
} // namespace lexy

//=== infrastructure ===//
namespace lexy
{
/// The final parser in the chain of NextParsers, forwarding everything to the context.
struct final_parser
{
    template <typename Context, typename Input, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Input&, Args&&... args) ->
        typename Context::result_type
    {
        return LEXY_MOV(context).value(LEXY_FWD(args)...);
    }
};
} // namespace lexy

#endif // LEXY_DSL_BASE_HPP_INCLUDED
