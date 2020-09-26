// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_INVOKE_HPP_INCLUDED
#define LEXY_DETAIL_INVOKE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

namespace lexy::_detail
{
template <typename ClassT, typename F, typename Object, typename... Args>
constexpr decltype(auto) invoke(F ClassT::*f, Object&& object, Args&&... args)
{
    if constexpr (std::is_member_function_pointer_v<decltype(f)>)
    {
        if constexpr (std::is_base_of_v<ClassT, std::decay_t<Object>>)
            // Invoke member function via object.
            return (LEXY_FWD(object).*f)(LEXY_FWD(args)...);
        else
            // Invoke member function via pointer.
            return ((*LEXY_FWD(object)).*f)(LEXY_FWD(args)...);
    }
    else
    {
        static_assert(std::is_member_object_pointer_v<decltype(f)> && sizeof...(Args) == 0);

        if constexpr (std::is_base_of_v<ClassT, std::decay_t<Object>>)
            // Access via object.
            return LEXY_FWD(object).*f;
        else
            // Access via pointer.
            return (*LEXY_FWD(object)).*f;
    }
}

template <typename F, typename... Args>
constexpr decltype(auto) invoke(F&& f, Args&&... args)
{
    return LEXY_FWD(f)(LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_INVOKE_HPP_INCLUDED

