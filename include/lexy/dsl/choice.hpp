// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CHOICE_HPP_INCLUDED
#define LEXY_DSL_CHOICE_HPP_INCLUDED

#include <lexy/_detail/tuple.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexy
{
struct exhausted_choice
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted choice";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename... R>
struct _chc
// Only make it a branch rule if it doesn't have an unconditional branch.
// A choice rule with an unconditional branch is itself an unconditional branch, which is most
// likely a bug.
: std::conditional_t<(lexy::is_unconditional_branch_rule<R> || ...), rule_base, branch_base>
{
    static constexpr auto _any_unconditional = (lexy::is_unconditional_branch_rule<R> || ...);

    template <typename Context, typename Reader,
              typename Indices = lexy::_detail::make_index_sequence<sizeof...(R)>>
    struct bp;
    template <typename Context, typename Reader, std::size_t... Idx>
    struct bp<Context, Reader, lexy::_detail::index_sequence<Idx...>>
    {
        template <typename Rule>
        using rp = lexy::branch_parser_for<Rule, Context, Reader>;

        lexy::_detail::tuple<rp<R>...> r_parsers;
        std::size_t                    branch_idx;

        constexpr auto try_parse(Context& context, const Reader& reader)
            -> std::conditional_t<_any_unconditional, std::true_type, bool>
        {
            auto try_r = [&](std::size_t idx, auto& parser) {
                if (!parser.try_parse(context, reader))
                    return false;

                branch_idx = idx;
                return true;
            };

            // Need to try each possible branch.
            auto found_branch = (try_r(Idx, r_parsers.template get<Idx>()) || ...);
            if constexpr (_any_unconditional)
            {
                LEXY_ASSERT(found_branch,
                            "it is unconditional, but we still haven't found a rule?!");
                return {};
            }
            else
            {
                return found_branch;
            }
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Need to call finish on the selected branch.
            auto result = false;
            (void)((Idx == branch_idx
                        ? (result
                           = r_parsers.template get<Idx>()
                                 .template finish<NextParser>(context, reader, LEXY_FWD(args)...),
                           true)
                        : false)
                   || ...);
            return result;
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto result = false;
            auto try_r  = [&](auto&& parser) {
                if (!parser.try_parse(context, reader))
                    return false;

                // LEXY_FWD(args) will break MSVC builds targetting C++17
                result = parser.template finish<NextParser>(context, reader, static_cast<Args&&>(args)...);
                return true;
            };

            // Try to parse each branch in order.
            auto found_branch = (try_r(lexy::branch_parser_for<R, Context, Reader>{}) || ...);
            if constexpr (_any_unconditional)
            {
                LEXY_ASSERT(found_branch,
                            "it is unconditional, but we still haven't found a rule?!");
                return result;
            }
            else
            {
                if (found_branch)
                    return result;

                auto err = lexy::error<Reader, lexy::exhausted_choice>(reader.position());
                context.on(_ev::error{}, err);
                return false;
            }
        }
    };
};

template <typename R, typename S>
constexpr auto operator|(R, S)
{
    static_assert(lexy::is_branch_rule<R>, "choice requires a branch condition");
    static_assert(lexy::is_branch_rule<S>, "choice requires a branch condition");
    return _chc<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator|(_chc<R...>, S)
{
    static_assert(lexy::is_branch_rule<S>, "choice requires a branch condition");
    return _chc<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator|(R, _chc<S...>)
{
    static_assert(lexy::is_branch_rule<R>, "choice requires a branch condition");
    return _chc<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator|(_chc<R...>, _chc<S...>)
{
    return _chc<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CHOICE_HPP_INCLUDED

