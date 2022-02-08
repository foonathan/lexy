// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/_detail/iterator.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/char_class.hpp>
#include <lexy/dsl/token.hpp>

#ifdef LEXY_IGNORE_DEPRECATED_ALTERNATIVE
#    define LEXY_DEPRECATED_ALTERNATIVE
#else
#    define LEXY_DEPRECATED_ALTERNATIVE                                                            \
        [[deprecated("use dsl::literal_set() or dsl::operator| instead")]]
#endif

namespace lexy
{
struct exhausted_alternatives
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted alternatives";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename... Tokens>
struct _alt : token_base<_alt<Tokens...>>
{
    template <typename Reader>
    struct LEXY_DEPRECATED_ALTERNATIVE tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            auto result = false;

            auto begin                    = reader.position();
            end                           = begin;
            [[maybe_unused]] auto process = [&](auto token, Reader local_reader) {
                // Try to match the current token.
                if (!lexy::try_match_token(token, local_reader))
                    return;

                // Update end to longest match.
                end    = lexy::_detail::max_range_end(begin, end, local_reader.position());
                result = true;
            };
            (process(Tokens{}, reader), ...);

            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::exhausted_alternatives>(reader.position());
            context.on(_ev::error{}, err);
        }
    };
};

// The generic operator/ overload is in char_class.hpp.

template <typename... R, typename S, typename = std::enable_if_t<!lexy::is_literal_set_rule<S>>>
LEXY_DEPRECATED_ALTERNATIVE constexpr auto operator/(_alt<R...>, S)
{
    static_assert(lexy::is_token_rule<S>);
    if constexpr ((lexy::is_char_class_rule<S> && ... && _is_convertible_char_class<R>))
        // If we add a char class, we attempt to turn it into a character class alternative.
        return _calt<decltype(_make_char_class(R{}))..., decltype(_make_char_class(S{}))>{};
    else
        return _alt<R..., S>{};
}
template <typename R, typename... S, typename = std::enable_if_t<!lexy::is_literal_set_rule<R>>>
LEXY_DEPRECATED_ALTERNATIVE constexpr auto operator/(R, _alt<S...>)
{
    static_assert(lexy::is_token_rule<R>);
    if constexpr ((lexy::is_char_class_rule<R> && ... && _is_convertible_char_class<S>))
        // If we add a char class, we attempt to turn it into a character class alternative.
        return _calt<decltype(_make_char_class(R{})), decltype(_make_char_class(S{}))...>{};
    else
        return _alt<R, S...>{};
}
template <typename... R, typename... S>
LEXY_DEPRECATED_ALTERNATIVE constexpr auto operator/(_alt<R...>, _alt<S...>)
{
    return _alt<R..., S...>{};
}
} // namespace lexyd

namespace lexy
{
template <typename H, typename... T>
constexpr auto token_kind_of<lexy::dsl::_alt<H, T...>> = [] {
    constexpr auto is_equal = [](auto a, auto b) {
        if constexpr (std::is_same_v<decltype(a), decltype(b)>)
            return a == b;
        else
            return false;
    };

    constexpr auto kind = lexy::token_kind_of<H>;
    if constexpr ((is_equal(kind, lexy::token_kind_of<T>) && ...))
        return kind;
    else
        return lexy::unknown_token_kind;
}();
} // namespace lexy

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

