// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_FOLD_HPP_INCLUDED
#define LEXY_CALLBACK_FOLD_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{
template <typename T, typename Arg, bool Inplace, typename Op>
struct _fold
{
    Arg                  _init;
    LEXY_EMPTY_MEMBER Op _op;

    struct _sink_callback
    {
        T  _result;
        Op _op;

        using return_type = T;

        template <typename... Args>
        constexpr void operator()(Args&&... args)
        {
            if constexpr (Inplace)
                _detail::invoke(_op, _result, LEXY_FWD(args)...);
            else
                _result = _detail::invoke(_op, LEXY_MOV(_result), LEXY_FWD(args)...);
        }

        constexpr T finish() &&
        {
            return LEXY_MOV(_result);
        }
    };

    constexpr auto sink() const
    {
        return _sink_callback{T(_init), _op};
    }
};

/// Sink that folds all the arguments with the binary operation op.
template <typename T, typename Arg = T, typename Op>
constexpr auto fold(Arg&& init, Op&& op)
{
    return _fold<T, std::decay_t<Arg>, false, std::decay_t<Op>>{LEXY_FWD(init), LEXY_FWD(op)};
}

/// Sink that folds all the arguments with the binary operation op that modifies the
/// result in-place.
template <typename T, typename Arg = T, typename Op>
constexpr auto fold_inplace(Arg&& init, Op&& op)
{
    return _fold<T, std::decay_t<Arg>, true, std::decay_t<Op>>{LEXY_FWD(init), LEXY_FWD(op)};
}
} // namespace lexy

namespace lexy
{
/// Sink that counts all arguments.
constexpr auto count
    = fold_inplace<std::size_t>(0u, [](std::size_t& result, auto&&...) { ++result; });
} // namespace lexy

#endif // LEXY_CALLBACK_FOLD_HPP_INCLUDED

