// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_COMBINATION_HPP_INCLUDED
#define LEXY_DSL_COMBINATION_HPP_INCLUDED

#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
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
template <typename Reader, std::size_t N, typename Sink>
struct _comb_state
{
    static constexpr auto max_count = N;

    // Whether or not the rule with the specified index was already matched.
    bool handled[N];
    // How many we already did.
    std::size_t count;
    // Beginning of the current repetition.
    typename Reader::iterator pos;
    // The sink to store values.
    Sink sink;
};

// Added at the end of each branch to update the combination state.
template <typename E, std::size_t Idx, typename... PrevArgs>
struct _comb_it : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, std::size_t N, typename Sink, typename... Args>
        LEXY_DSL_FUNC auto parse(_loop_handler<Handler>& handler, Reader& reader,
                                 _comb_state<Reader, N, Sink>& state, PrevArgs&&... pargs,
                                 Args&&... args) -> typename _loop_handler<Handler>::result_type
        {
            if (!state.handled[Idx])
            {
                state.handled[Idx] = true;
                state.count++;
                state.pos = reader.cur();
                if constexpr (sizeof...(Args) > 0)
                    state.sink(LEXY_FWD(args)...);

                if (state.count == state.max_count)
                    return LEXY_MOV(handler).break_();
                else
                    return NextParser::parse(handler, reader, state, LEXY_FWD(pargs)...);
            }
            else
            {
                using tag = std::conditional_t<std::is_void_v<E>, lexy::combination_duplicate, E>;
                auto e    = lexy::make_error<Reader, tag>(state.pos, reader.cur());
                return LEXY_MOV(handler).error(reader, e);
            }
        }
    };
};

template <bool Partial, typename E, typename... R>
struct _comb : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename... Args, std::size_t... Idx>
        static LEXY_CONSTEVAL auto _comb_choice(lexy::_detail::index_sequence<Idx...>)
        {
            if constexpr (Partial)
                return ((R{} >> _comb_it<E, Idx, Args...>{}) | ... | (else_ >> break_));
            else
                return ((R{} >> _comb_it<E, Idx, Args...>{}) | ...);
        }

        struct _continuation
        {
            template <typename Handler, typename Reader, typename Sink, typename... Args>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader,
                                     _comb_state<Reader, sizeof...(R), Sink>& state, Args&&... args)
                -> typename Handler::result_type
            {
                if constexpr (std::is_same_v<typename Sink::return_type, void>)
                {
                    LEXY_MOV(state.sink).finish();
                    return NextParser::parse(handler, reader, LEXY_FWD(args)...);
                }
                else
                {
                    return NextParser::parse(handler, reader, LEXY_FWD(args)...,
                                             LEXY_MOV(state.sink).finish());
                }
            }
        };

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            constexpr auto N = sizeof...(R);

            using state_t = _comb_state<Reader, N, decltype(handler.list_sink())>;
            state_t state{{}, 0, reader.cur(), handler.list_sink()};

            constexpr auto comb_choice
                = _comb_choice<Args...>(lexy::_detail::make_index_sequence<N>{});
            constexpr auto l = loop(comb_choice);
            return decltype(l)::template parser<_continuation>::parse(handler, reader, state,
                                                                      LEXY_FWD(args)...);
        }
    };
};

/// Matches each of the rules in an arbitrary order.
/// Only matches each rule exactly once.
template <typename Error = void, typename... R>
LEXY_CONSTEVAL auto combination(R...)
{
    static_assert((lexy::is_branch_rule<R> && ...), "combination() requires a branch rule");
    return _comb<false, Error, R...>{};
}

/// Matches some of the rules in an arbitrary order.
/// Only matches a rule at most once.
template <typename Error = void, typename... R>
LEXY_CONSTEVAL auto partial_combination(R...)
{
    static_assert((lexy::is_branch_rule<R> && ...), "partial_combination() requires a branch rule");
    return _comb<true, Error, R...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_COMBINATION_HPP_INCLUDED

