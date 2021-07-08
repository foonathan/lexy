// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_COMBINATION_HPP_INCLUDED
#define LEXY_DSL_COMBINATION_HPP_INCLUDED

#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/error.hpp>
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
    // Whether or not the state has already been handled.
    const bool* handled;
    // Write the index of the item in here.
    std::size_t idx = 0;
    // Whether or not we should break.
    bool loop_break = false;
};

// Final parser for one item in the combination.
struct _comb_final
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader&, std::size_t idx, Args&&... args)
    {
        auto& state = context.get(_break{});
        state.idx   = idx;
        if constexpr (sizeof...(Args) > 0)
        {
            if (!state.handled[idx])
                // Only call the sink if it is not a duplicate.
                state.sink(LEXY_FWD(args)...);
        }
        return true;
    }
};

// Parser for one item in the combination.
template <std::size_t Idx, typename Rule>
struct _comb_it : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader)
            -> lexy::rule_try_parse_result
        {
            return lexy::rule_parser<Rule, NextParser>::try_parse(context, reader, Idx);
        }

        template <typename Context, typename Reader>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader)
        {
            return lexy::rule_parser<Rule, NextParser>::parse(context, reader, Idx);
        }
    };
};

template <typename DuplicateError, typename ElseRule, typename... R>
struct _comb : rule_base
{
    template <std::size_t... Idx>
    static auto _comb_choice_(lexy::_detail::index_sequence<Idx...>)
    {
        if constexpr (std::is_void_v<ElseRule>)
            return (_comb_it<Idx, R>{} | ...);
        else
            return (_comb_it<Idx, R>{} | ... | ElseRule{});
    }
    using _comb_choice = decltype(_comb_choice_(lexy::_detail::index_sequence_for<R...>{}));

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            constexpr auto N = sizeof...(R);

            auto sink       = context.on(_ev::list{}, reader.cur());
            bool handled[N] = {};
            auto comb_context
                = context.insert(_break{}, _comb_state<decltype(sink)>{sink, handled});
            auto& state = comb_context.get(_break{});

            // Parse all iterations of the choice.
            for (auto count = 0; count < int(N); ++count)
            {
                auto begin = reader.cur();

                using parser = lexy::rule_parser<_comb_choice, _comb_final>;
                if (!parser::parse(comb_context, reader))
                    return false;
                else if (state.loop_break)
                    break; // Partial combination and we're done.

                if (handled[state.idx])
                {
                    using tag = lexy::_detail::type_or<DuplicateError, lexy::combination_duplicate>;
                    auto err  = lexy::make_error<Reader, tag>(begin, reader.cur());
                    context.on(_ev::error{}, err);
                    // We can trivially recover, but need to do another iteration.
                    --count;
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
constexpr auto combination(R...)
{
    static_assert((lexy::is_branch_rule<R> && ...), "combination() requires a branch rule");
    static_assert((!R::is_unconditional_branch && ...),
                  "combination() does not support unconditional branches");
    return _comb<void, void, R...>{};
}

/// Matches some of the rules in an arbitrary order.
/// Only matches a rule at most once.
template <typename... R>
constexpr auto partial_combination(R...)
{
    static_assert((lexy::is_branch_rule<R> && ...), "partial_combination() requires a branch rule");
    static_assert((!R::is_unconditional_branch && ...),
                  "partial_combination() does not support unconditional branches");
    // If the choice no longer matches, we just break.
    return _comb<void, decltype(break_), R...>{};
}

template <typename Tag, typename... R>
LEXY_DEPRECATED_ERROR(
    "replace `combination<Tag>(r...)` by `combination(r...).duplicate_error<Tag>`")
constexpr auto combination(R... r)
{
    return combination(r...).template duplicate_error<Tag>;
}
template <typename Tag, typename... R>
LEXY_DEPRECATED_ERROR(
    "replace `partial_combination<Tag>(r...)` by `partial_combination(r...).duplicate_error<Tag>`")
constexpr auto partial_combination(R... r)
{
    return partial_combination(r...).template duplicate_error<Tag>;
}
} // namespace lexyd

#endif // LEXY_DSL_COMBINATION_HPP_INCLUDED

