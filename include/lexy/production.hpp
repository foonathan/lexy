// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PRODUCTION_HPP_INCLUDED
#define LEXY_PRODUCTION_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/type_name.hpp>

namespace lexy
{
template <typename Production>
using _detect_value = decltype(&Production::value);

template <typename Production>
struct production_traits
{
    static LEXY_CONSTEVAL auto name()
    {
        return _detail::type_name<Production>();
    }

    struct rule
    {
        static constexpr auto get = Production::rule;
        using type                = std::decay_t<decltype(get)>;
    };

    struct value
    {
        static constexpr auto get = [] {
            if constexpr (!_detail::is_detected<_detect_value, Production>)
                static_assert(_detail::error<Production>, "missing Production::value member");

            return Production::value;
        }();
        using type = std::decay_t<decltype(get)>;
    };
};
} // namespace lexy

#endif // LEXY_PRODUCTION_HPP_INCLUDED

