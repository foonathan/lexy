// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED
#define LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/string_view.hpp>

namespace lexy::_detail
{
template <typename T>
using _detect_name_f = decltype(T::name());
template <typename T>
using _detect_name_v = decltype(T::name);

template <typename T>
constexpr auto _type_name_impl()
{
#if defined(__clang__)

    constexpr auto prefix = string_view("auto lexy::_detail::_type_name_impl() [T = ");
    constexpr auto suffix = string_view("]");

    auto function = string_view(__PRETTY_FUNCTION__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    return function;

#elif defined(__GNUC__)

    constexpr auto prefix
        = string_view("constexpr auto lexy::_detail::_type_name_impl() [with T = ");
    constexpr auto suffix = string_view("]");

    auto function = string_view(__PRETTY_FUNCTION__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    return function;

#elif defined(_MSC_VER)

    constexpr auto prefix = string_view("auto __cdecl lexy::_detail::_type_name_impl<");
    constexpr auto suffix = string_view(">(void)");

    auto function = string_view(__FUNCSIG__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    return function;

#else

    static_assert(_detail::error<T>,
                  "require T::name() or T::name on this compiler to get the name of a type");
    return "";

#endif
}

template <typename T>
LEXY_CONSTEVAL string_view type_name(int namespace_count = 1)
{
    if constexpr (_detail::is_detected<_detect_name_f, T>)
        return string_view(T::name());
    else if constexpr (_detail::is_detected<_detect_name_v, T>)
        return string_view(T::name);
    else
    {
        auto name = _type_name_impl<T>();
        LEXY_ASSERT(name.find('<') == string_view::npos || namespace_count == 0,
                    "cannot strip namespaces from template instantiations");

        for (; namespace_count > 0; --namespace_count)
        {
            auto pos = name.find("::");
            if (pos == string_view::npos)
                break;
            name.remove_prefix(pos + 2);
        }
        return name;
    }
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED

