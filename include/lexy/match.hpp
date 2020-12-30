// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

#include <lexy/callback.hpp>
#include <lexy/production.hpp>
#include <lexy/result.hpp>

namespace lexy
{
struct _match_handler
{
    template <typename Production>
    using result_type_for = lexy::result<void, void>;

    template <typename Production>
    constexpr auto sink(Production)
    {
        return noop.sink();
    }

    template <typename Production, typename Iterator>
    constexpr void start_production(Production, Iterator)
    {}

    template <typename Production, typename... Args>
    constexpr auto finish_production(Production, Args&&...)
    {
        return result_type_for<Production>(lexy::result_value);
    }

    template <typename Production, typename Input, typename Error>
    constexpr auto error(lexy::error_context<Production, Input>&&, Error&&)
    {
        return result_type_for<Production>(lexy::result_error);
    }
};

template <typename Reader, typename Rule>
LEXY_FORCE_INLINE constexpr bool _match_impl(Reader& reader, Rule)
{
    struct input_t
    {
        Reader _reader;

        Reader reader() const
        {
            return _reader;
        }
    } input{reader};

    using context_t = lexy::parse_context<input_t, lexy::_match_handler>;
    context_t context(input);

    lexy::production_context prod_ctx(context, Rule{}, reader.cur());
    return lexy::rule_parser<Rule, lexy::context_value_parser>::parse(prod_ctx, reader).has_value();
}

template <typename Input, typename Rule, typename = std::enable_if_t<is_rule<Rule>>>
LEXY_FORCE_INLINE constexpr bool match(const Input& input, Rule rule)
{
    auto reader = input.reader();
    return _match_impl(reader, rule);
}

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    auto reader = input.reader();
    return _match_impl(reader, lexy::production_traits<Production>::rule::get);
}
} // namespace lexy

#endif // LEXY_MATCH_HPP_INCLUDED

