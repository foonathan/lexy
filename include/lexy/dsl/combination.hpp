// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_COMBINATION_HPP_INCLUDED
#define LEXY_DSL_COMBINATION_HPP_INCLUDED

#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexy
{
struct combination_duplicate
{
    static LEXY_CONSTEVAL auto name()
    {
        return "combination duplicate";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Sink>
struct _comb_state
{
    // The sink to store values of the item.
    Sink& sink;
    // Write the index of the item in here.
    int idx = 0;
    // Whether or not we should break.
    bool loop_break = false;
};

// Final parser for one item in the combination.
struct _comb_it
{
    template <typename Context, typename Reader, int Idx, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader&, lexy::id<Idx>, Args&&... args)
    {
        auto& state = context.get(_break{});
        state.idx   = Idx;
        if constexpr (sizeof...(Args) > 0)
            state.sink(LEXY_FWD(args)...);
        return true;
    }
};

template <typename DuplicateError, typename ElseRule, typename... R>
struct _comb : rule_base
{
    template <std::size_t... Idx>
    static auto _comb_choice_(lexy::_detail::index_sequence<Idx...>)
    {
        if constexpr (std::is_void_v<ElseRule>)
            return (id<Idx>(R{}) | ...);
        else
            return (id<Idx>(R{}) | ... | ElseRule{});
    }
    using _comb_choice = decltype(_comb_choice_(lexy::_detail::index_sequence_for<R...>{}));

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            constexpr auto N = sizeof...(R);

            auto  sink         = context.sink();
            bool  handled[N]   = {};
            auto  comb_context = context.insert(_break{}, _comb_state<decltype(sink)>{sink});
            auto& state        = comb_context.get(_break{});

            // Parse all iterations of the choice.
            for (std::size_t count = 0; count < N; ++count)
            {
                auto begin = reader.cur();

                using parser = lexy::rule_parser<_comb_choice, _comb_it>;
                if (!parser::parse(comb_context, reader))
                    return false;
                else if (state.loop_break)
                    break; // Partial combination and we're done.

                if (handled[state.idx])
                {
                    using tag = std::conditional_t<std::is_void_v<DuplicateError>,
                                                   lexy::combination_duplicate, DuplicateError>;
                    auto err  = lexy::make_error<Reader, tag>(begin, reader.cur());
                    context.error(err);
                    return false;
                }
                else
                {
                    handled[state.idx] = true;
                }
            }

            // Obtain the final result and continue.
            if constexpr (std::is_void_v<typename decltype(sink)::return_type>)
            {
                LEXY_MOV(sink).finish();
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                         LEXY_MOV(sink).finish());
            }
        }
    };

    //=== dsl ===//
    template <typename Tag>
    static constexpr _comb<Tag, ElseRule, R...> duplicate_error = {};

    template <typename Tag>
    static constexpr _comb<DuplicateError, _err<Tag, void>, R...> missing_error = {};
};

/// Matches each of the rules in an arbitrary order.
/// Only matches each rule exactly once.
template <typename... R>
LEXY_CONSTEVAL auto combination(R...)
{
    static_assert((lexy::is_branch<R> && ...), "combination() requires a branch rule");
    return _comb<void, void, R...>{};
}

/// Matches some of the rules in an arbitrary order.
/// Only matches a rule at most once.
template <typename... R>
LEXY_CONSTEVAL auto partial_combination(R...)
{
    static_assert((lexy::is_branch<R> && ...), "partial_combination() requires a branch rule");
    // If the choice no longer matches, we just break.
    return _comb<void, decltype(break_), R...>{};
}

template <typename Tag, typename... R>
LEXY_DEPRECATED_ERROR(
    "replace `combination<Tag>(r...)` by `combination(r...).duplicate_error<Tag>`")
LEXY_CONSTEVAL auto combination(R... r)
{
    return combination(r...).template duplicate_error<Tag>;
}
template <typename Tag, typename... R>
LEXY_DEPRECATED_ERROR(
    "replace `partial_combination<Tag>(r...)` by `partial_combination(r...).duplicate_error<Tag>`")
LEXY_CONSTEVAL auto partial_combination(R... r)
{
    return partial_combination(r...).template duplicate_error<Tag>;
}
} // namespace lexyd

#endif // LEXY_DSL_COMBINATION_HPP_INCLUDED

