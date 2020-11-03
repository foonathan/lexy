// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_COMBINATION_HPP_INCLUDED
#define LEXY_DSL_COMBINATION_HPP_INCLUDED

#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
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
    // Whether or not the rule with the specified index was already matched.
    bool handled[N];
    // How many we already did.
    std::size_t count;
    // Beginning of the current repetition.
    typename Reader::iterator pos;
    // The sink to store values.
    Sink sink;
};

// Loops back if necessary.
template <typename CombChoice, typename NextParser>
struct _comb_loop_parser
{
    template <typename Handler, typename Reader, std::size_t N, typename Sink, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, _comb_state<Reader, N, Sink>& state,
                             Args&&... args) -> typename Handler::result_type
    {
        if (state.count == N)
        {
            // We've parsed each alternative exactly once, so we're done.
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
        else
        {
            // Parse one alternative, then recurse back.
            return CombChoice::template parser<_comb_loop_parser>::parse(handler, reader, state,
                                                                         LEXY_FWD(args)...);
        }
    }
};

// Added at the end of each branch to update the combination state.
template <typename E, std::size_t Idx, typename... PrevArgs>
struct _comb_rule : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, std::size_t N, typename Sink, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader,
                                 _comb_state<Reader, N, Sink>& state, PrevArgs&&... pargs,
                                 Args&&... args) -> typename Handler::result_type
        {
            if (!state.handled[Idx])
            {
                state.handled[Idx] = true;
                state.count++;
                state.pos = reader.cur();

                if constexpr (sizeof...(Args) > 0)
                    state.sink(LEXY_FWD(args)...);

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

template <typename E, typename... R>
struct _comb : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename... Args, std::size_t... Idx>
        static LEXY_CONSTEVAL auto _comb_choice(lexy::_detail::index_sequence<Idx...>)
        {
            return ((R{} >> _comb_rule<E, Idx, Args...>{}) | ...);
        }

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            constexpr auto N = sizeof...(R);

            // We continue with the loop parser.
            using comb_choice
                = decltype(_comb_choice<Args...>(lexy::_detail::make_index_sequence<N>{}));
            using continuation = _comb_loop_parser<comb_choice, NextParser>;

            using state_t = _comb_state<Reader, N, decltype(handler.list_sink())>;
            state_t state{{}, 0, reader.cur(), handler.list_sink()};
            return continuation::parse(handler, reader, state, LEXY_FWD(args)...);
        }
    };
};

/// Matches each of the rules in an arbitrary order.
/// Only matches each rule exactly once.
template <typename Error = void, typename... R>
LEXY_CONSTEVAL auto combination(R...)
{
    static_assert((lexy::is_branch_rule<R> && ...), "combination() requires a branch rule");
    return _comb<Error, R...>{};
}
} // namespace lexyd

namespace lexyd
{
// Handles partial rules.
struct _comb_else : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, std::size_t N, typename Sink, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader,
                                 _comb_state<Reader, N, Sink>& state, Args&&... args) ->
            typename Handler::result_type
        {
            // Setting the count to N terminates the loop.
            state.count = N;
            return NextParser::parse(handler, reader, state, LEXY_FWD(args)...);
        }
    };
};

template <typename E, typename... R>
struct _comb_partial : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename... Args, std::size_t... Idx>
        static LEXY_CONSTEVAL auto _comb_choice(lexy::_detail::index_sequence<Idx...>)
        {
            return ((R{} >> _comb_rule<E, Idx, Args...>{}) | ... | (else_ >> _comb_else{}));
        }

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            constexpr auto N = sizeof...(R);

            // We continue with the loop parser.
            using comb_choice
                = decltype(_comb_choice<Args...>(lexy::_detail::make_index_sequence<N>{}));
            using continuation = _comb_loop_parser<comb_choice, NextParser>;

            using state_t = _comb_state<Reader, N, decltype(handler.list_sink())>;
            state_t state{{}, 0, reader.cur(), handler.list_sink()};
            return continuation::parse(handler, reader, state, LEXY_FWD(args)...);
        }
    };
};

/// Matches some of the rules in an arbitrary order.
/// Only matches a rule at most once.
template <typename Error = void, typename... R>
LEXY_CONSTEVAL auto partial_combination(R...)
{
    static_assert((lexy::is_branch_rule<R> && ...), "partial_combination() requires a branch rule");
    return _comb_partial<Error, R...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_COMBINATION_HPP_INCLUDED

