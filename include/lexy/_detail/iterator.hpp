// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_ITERATOR_HPP_INCLUDED
#define LEXY_DETAIL_ITERATOR_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/std.hpp>

namespace lexy::_detail
{
template <typename T>
struct _proxy_pointer
{
    T value;

    constexpr T* operator->() noexcept
    {
        return &value;
    }
};

template <typename Derived, typename T, typename Reference = T&, typename Pointer = const T*>
struct forward_iterator_base
{
    using value_type        = std::remove_cv_t<T>;
    using reference         = Reference;
    using pointer           = lexy::_detail::type_or<Pointer, _proxy_pointer<value_type>>;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    constexpr reference operator*() const noexcept
    {
        return static_cast<const Derived&>(*this).deref();
    }
    constexpr pointer operator->() const noexcept
    {
        if constexpr (std::is_void_v<Pointer>)
            return pointer{**this};
        else
            return &**this;
    }

    constexpr Derived& operator++() noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        derived.increment();
        return derived;
    }
    constexpr Derived operator++(int) noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        auto  copy    = derived;
        derived.increment();
        return copy;
    }

    friend constexpr bool operator==(const Derived& lhs, const Derived& rhs)
    {
        return lhs.equal(rhs);
    }
    friend constexpr bool operator!=(const Derived& lhs, const Derived& rhs)
    {
        return !lhs.equal(rhs);
    }
};

template <typename Derived, typename T, typename Reference = T&, typename Pointer = const T*>
struct bidirectional_iterator_base : forward_iterator_base<Derived, T, Reference, Pointer>
{
    using iterator_category = std::bidirectional_iterator_tag;

    constexpr Derived& operator--() noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        derived.decrement();
        return derived;
    }
    constexpr Derived operator--(int) noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        auto  copy    = derived;
        derived.decrement();
        return copy;
    }
};

template <typename Derived, typename Iterator>
struct sentinel_base
{
    friend constexpr bool operator==(const Iterator& lhs, Derived) noexcept
    {
        return lhs.is_end();
    }
    friend constexpr bool operator!=(const Iterator& lhs, Derived) noexcept
    {
        return !(lhs == Derived{});
    }
    friend constexpr bool operator==(Derived, const Iterator& rhs) noexcept
    {
        return rhs == Derived{};
    }
    friend constexpr bool operator!=(Derived, const Iterator& rhs) noexcept
    {
        return !(rhs == Derived{});
    }
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_ITERATOR_HPP_INCLUDED

