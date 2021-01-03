// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PRODUCTION_HPP_INCLUDED
#define LEXY_PRODUCTION_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/type_name.hpp>

#ifdef LEXY_IGNORE_DEPRECATED_LIST
#    define LEXY_DEPRECATED_LIST
#else
#    define LEXY_DEPRECATED_LIST                                                                   \
        [[deprecated("Production::list has been deprecated; use ::value instead.")]]
#endif

namespace lexy
{
template <typename Production>
using _detect_value = decltype(&Production::value);
template <typename Production>
using _detect_list = decltype(&Production::list);

template <typename Production, bool HasList = _detail::is_detected<_detect_list, Production>,
          bool HasValue = _detail::is_detected<_detect_value, Production>>
struct _prod_value;
template <typename Production>
struct _prod_value<Production, true, true>
{
    // Before:
    //    static constexpr auto list = sink;
    //    static constexpr auto value = callback;
    // After:
    //    static constexpr auto value = sink >> callback;
    // Define LEXY_IGNORE_DEPRECATED_LIST to fix later.
    LEXY_DEPRECATED_LIST
    static constexpr auto get = Production::list >> Production::value;
};
template <typename Production>
struct _prod_value<Production, true, false>
{
    // Before:
    //    static constexpr auto list = sink;
    // After:
    //    static constexpr auto value = sink;
    // Define LEXY_IGNORE_DEPRECATED_LIST to fix later.
    LEXY_DEPRECATED_LIST
    static constexpr auto get = Production::list;
};
template <typename Production>
struct _prod_value<Production, false, true>
{
    static constexpr auto get = Production::value;
};
template <typename Production>
struct _prod_value<Production, false, false>
{
    static_assert(_detail::error<Production>, "missing Production::value member");
    static constexpr auto get = Production::value;
};
} // namespace lexy

namespace lexy
{
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
        static constexpr auto get = _prod_value<Production>::get;
        using type                = std::decay_t<decltype(get)>;
    };
};
} // namespace lexy

#endif // LEXY_PRODUCTION_HPP_INCLUDED

