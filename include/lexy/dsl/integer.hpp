// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_INTEGER_HPP_INCLUDED
#define LEXY_DSL_INTEGER_HPP_INCLUDED

#include <limits>

#include <lexy/_detail/assert.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/digit.hpp>

namespace lexy
{
// Number of digits to express the given value.
template <typename Integer>
constexpr std::size_t _digit_count(int radix, Integer value)
{
    LEXY_PRECONDITION(value >= Integer(0));

    if (value == 0)
        return 1;

    std::size_t result = 0;
    while (value > 0)
    {
        value = Integer(value / Integer(radix));
        ++result;
    }
    return result;
}

template <typename T>
struct integer_traits
{
    using _limits = std::numeric_limits<T>;
    static_assert(_limits::is_integer);

    using type = T;

    static constexpr auto is_bounded = _limits::is_bounded;

    template <int Radix>
    static constexpr std::size_t max_digit_count = _digit_count(Radix, _limits::max());

    template <int Radix>
    static constexpr void add_digit_unchecked(type& result, unsigned digit)
    {
        result = T(result * T(Radix) + T(digit));
    }
    template <int Radix>
    static constexpr bool add_digit_checked(type& result, unsigned digit)
    {
        // result *= Radix
        constexpr auto max_per_radix = _limits::max() / Radix;
        if (result > max_per_radix)
            return false;
        result = T(result * Radix);

        // result += value
        if (result > T(_limits::max() - digit))
            return false;
        result = T(result + T(digit));

        return true;
    }
};

template <>
struct integer_traits<code_point>
{
    using type = code_point;

    static constexpr auto is_bounded = true;

    template <int Radix>
    static constexpr std::size_t max_digit_count = _digit_count(Radix, 0x10'FFFF);

    template <int Radix>
    static constexpr void add_digit_unchecked(type& result, unsigned digit)
    {
        std::uint_least32_t value = result.value();
        integer_traits<std::uint_least32_t>::add_digit_unchecked<Radix>(value, digit);
        result = code_point(value);
    }
    template <int Radix>
    static constexpr bool add_digit_checked(type& result, unsigned digit)
    {
        std::uint_least32_t value = result.value();
        if (!integer_traits<std::uint_least32_t>::add_digit_checked<Radix>(value, digit))
            return false;
        result = code_point(value);
        return result.is_valid();
    }
};

template <typename T>
struct unbounded
{};
template <typename T>
struct integer_traits<unbounded<T>>
{
    using type                       = typename integer_traits<T>::type;
    static constexpr auto is_bounded = false;

    template <int Radix>
    static constexpr void add_digit_unchecked(type& result, unsigned digit)
    {
        integer_traits<T>::template add_digit_unchecked<Radix>(result, digit);
    }
};
} // namespace lexy

namespace lexy
{
struct integer_overflow
{
    static LEXY_CONSTEVAL auto name()
    {
        return "integer overflow";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename T, typename Pattern, typename Base, bool HasSep>
struct _integer : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Handler, typename Reader, typename... Args>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader,
                                     const typename Reader::iterator begin, Args&&... args) ->
                typename Handler::result_type
            {
                using error_type
                    = lexy::error<typename Reader::canonical_reader, lexy::integer_overflow>;

                using traits         = lexy::integer_traits<T>;
                using result_type    = typename traits::type;
                constexpr auto radix = Base::radix;

                auto       cur    = begin;
                const auto end    = reader.cur();
                auto       result = result_type(0);

                if constexpr (traits::is_bounded)
                {
                    constexpr auto max_digit_count = traits::template max_digit_count<radix>;
                    static_assert(max_digit_count > 1);

                    // First skip over leading zeroes.
                    while (true)
                    {
                        if (cur == end)
                            // The number is zero.
                            return NextParser::parse(handler, reader, LEXY_FWD(args)..., result);

                        const auto digit = Base::value(*cur++);
                        if (digit == 0 || digit >= radix)
                            continue; // Zero or digit separator.

                        // Found the first non-zero digit.
                        // This can't overflow, we've asserted that the maximal digit count is at
                        // least one. We assign because we had zero and are only taking this line
                        // once.
                        result = result_type(digit);
                        break;
                    }
                    // At this point, we've parsed exactly one non-zero digit.

                    // Handle max_digit_count - 1 digits without checking for overflow.
                    // We cannot overflow, as the maximal value has one digit more.
                    for (std::size_t digit_count = 1; digit_count < max_digit_count - 1;
                         ++digit_count)
                    {
                        // Find the next digit.
                        auto digit = 0u;
                        while (true)
                        {
                            if (cur == end)
                                // No more digits.
                                return NextParser::parse(handler, reader, LEXY_FWD(args)...,
                                                         result);

                            digit = Base::value(*cur++);
                            if constexpr (!HasSep)
                                break;
                            else if (digit < radix)
                                break;
                        }

                        traits::template add_digit_unchecked<radix>(result, digit);
                    }

                    // Handle the final digit, if there is any, while checking for overflow.
                    {
                        // Find it.
                        auto digit = 0u;
                        while (true)
                        {
                            if (cur == end)
                                // No more digits.
                                return NextParser::parse(handler, reader, LEXY_FWD(args)...,
                                                         result);

                            digit = Base::value(*cur++);
                            if constexpr (!HasSep)
                                break;
                            else if (digit < radix)
                                break;
                        }

                        if (!traits::template add_digit_checked<radix>(result, digit))
                            return LEXY_MOV(handler).error(reader, error_type(begin, end));
                    }

                    // If we're having any more digits, this is a guaranteed overflow.
                    if (cur != end)
                        return LEXY_MOV(handler).error(reader, error_type(begin, end));
                }
                else
                {
                    // For an unbounded integer, we just parse digits until we're out of digits.
                    while (true)
                    {
                        // Find the next digit.
                        auto digit = 0u;
                        while (true)
                        {
                            if (cur == end)
                                // No more digits.
                                return NextParser::parse(handler, reader, LEXY_FWD(args)...,
                                                         result);

                            digit = Base::value(*cur++);
                            if (digit < Base::radix)
                                break; // Found a digit.
                        }

                        traits::template add_digit_unchecked<radix>(result, digit);
                    }
                }

                return NextParser::parse(handler, reader, LEXY_FWD(args)..., result);
            }
        };

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // Parse the digit pattern with the special continuation.
            return Pattern::template parser<_continuation>::parse(handler, reader, reader.cur(),
                                                                  LEXY_FWD(args)...);
        }
    };
};

/// Parses the digits matched by the pattern into an integer type.
template <typename T, typename Base, typename Pattern>
LEXY_CONSTEVAL auto integer(Pattern)
{
    // We assume it has a separator.
    return _integer<T, Pattern, Base, true>{};
}
template <typename T, typename Base, typename Sep, bool LeadingZero>
LEXY_CONSTEVAL auto integer(_digits<Base, Sep, LeadingZero>)
{
    return _integer<T, _digits<Base, Sep, LeadingZero>, Base, !std::is_void_v<Sep>>{};
}
template <typename T, typename Base, std::size_t N, typename Sep, bool LeadingZero>
LEXY_CONSTEVAL auto integer(_ndigits<N, Base, Sep, LeadingZero>)
{
    return _integer<T, _ndigits<N, Base, Sep, LeadingZero>, Base, !std::is_void_v<Sep>>{};
}
} // namespace lexyd

namespace lexyd
{
/// Matches the number of a code point.
template <std::size_t N, typename Base = hex>
constexpr auto code_point_id = integer<lexy::code_point>(n_digits<N, Base>);
} // namespace lexyd

#endif // LEXY_DSL_INTEGER_HPP_INCLUDED
