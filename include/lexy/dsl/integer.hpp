// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_INTEGER_HPP_INCLUDED
#define LEXY_DSL_INTEGER_HPP_INCLUDED

#include <limits>

#include <lexy/_detail/assert.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/digit.hpp>
#include <lexy/dsl/failure.hpp>

namespace lexy
{
template <typename T>
struct integer_traits
{
    using _limits = std::numeric_limits<T>;
    static_assert(_limits::is_integer);

    using integer_type = T;

    static constexpr auto is_bounded = _limits::is_bounded;
    static constexpr auto max_value  = _limits::max();
};

template <typename T>
struct unbounded
{};
template <typename T>
struct integer_traits<unbounded<T>>
{
    using integer_type               = T;
    static constexpr auto is_bounded = false;
};

struct integer_overflow
{};
} // namespace lexy

namespace lexyd
{
// Number of digits to express the given value.
template <typename Integer>
static constexpr std::size_t _digit_count(unsigned radix, Integer value)
{
    LEXY_PRECONDITION(value >= Integer(0));

    if (value == 0)
        return 1;

    std::size_t result = 0;
    while (value > 0)
    {
        value /= radix;
        ++result;
    }
    return result;
}

template <typename Pattern, typename T>
struct _integer;
template <typename Base, typename Sep, bool LeadingZero, typename T>
struct _integer<_digits<Base, Sep, LeadingZero>, T> : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC auto parse(Context& context, Reader& reader,
                                     typename Reader::iterator begin, Args&&... args) ->
                typename Context::result_type
            {
                using error_type       = lexy::failure<lexy::integer_overflow>::error<Reader>;
                constexpr auto has_sep = !std::is_same_v<Sep, void>;

                using traits         = lexy::integer_traits<T>;
                using integer        = typename traits::integer_type;
                constexpr auto radix = integer(Base::radix);

                auto cur    = begin;
                auto end    = reader.cur();
                auto result = integer(0);

                if constexpr (traits::is_bounded)
                {
                    constexpr auto max_value       = traits::max_value;
                    constexpr auto max_digit_count = _digit_count(radix, max_value);
                    static_assert(max_digit_count > 0);
                    static_assert(max_value >= radix);

                    // First skip over leading zeroes.
                    while (true)
                    {
                        if (cur == end)
                            // The number is zero.
                            return NextParser::parse(context, reader, LEXY_FWD(args)..., result);

                        const auto value = Base::value(*cur++);
                        if (value == 0 || value >= Base::radix)
                            continue; // Zero or digit separator.

                        // Found the first non-zero digit.
                        // This can't overflow, we've asserted that the maximal value is at least
                        // base. We assign because we had zero and are only taking this line once.
                        result = integer(value);
                        break;
                    }
                    // At this point, we've parsed exactly one non-zero digit.

                    // Handle max_digit_count - 1 digits without checking for overflow.
                    // We cannot overflow, as the maximal value has one digit more.
                    for (std::size_t digit_count = 1; digit_count < max_digit_count - 1;
                         ++digit_count)
                    {
                        // Find the next digit.
                        auto value = 0u;
                        while (true)
                        {
                            if (cur == end)
                                // No more digits.
                                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                                         result);

                            value = Base::value(*cur++);
                            if constexpr (!has_sep)
                                break;
                            else if (value < Base::radix)
                                break;
                        }

                        // Add digit.
                        result *= radix;
                        result += integer(value);
                    }

                    // Handle the final digit, if there is any, while checking for overflow.
                    {
                        // Find it.
                        auto value = 0u;
                        while (true)
                        {
                            if (cur == end)
                                // No more digits.
                                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                                         result);

                            value = Base::value(*cur++);
                            if constexpr (!has_sep)
                                break;
                            else if (value < Base::radix)
                                break;
                        }

                        // result *= radix
                        if (result > max_value / radix)
                            return LEXY_MOV(context).error(reader, error_type(begin));
                        result *= radix;

                        // result += value
                        if (result > integer(max_value - value))
                            return LEXY_MOV(context).error(reader, error_type(begin));
                        result += integer(value);
                    }

                    // If we're having any more digits, this is a guranteed overflow.
                    if (cur != end)
                        return LEXY_MOV(context).error(reader, error_type(begin));

                    return NextParser::parse(context, reader, LEXY_FWD(args)..., result);
                }
                else
                {
                    // For an unbounded integer, we just parse digits until we're out of digits.
                    while (true)
                    {
                        // Find the next digit.
                        auto value = 0u;
                        while (true)
                        {
                            if (cur == end)
                                // No more digits.
                                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                                         result);

                            value = Base::value(*cur++);
                            if (value < Base::radix)
                                break; // Found a digit.
                        }

                        // Add it.
                        result *= radix;
                        result += integer(value);
                    }

                    return NextParser::parse(context, reader, LEXY_FWD(args)..., result);
                }
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            // Parse the digit pattern with the special continuation.
            using digits = _digits<Base, Sep, LeadingZero>;
            return digits::template parser<_continuation>::parse(context, reader, reader.cur(),
                                                                 LEXY_FWD(args)...);
        }
    };
};

/// Parses the digits matched by the pattern into an integer type.
template <typename T, typename Base, typename Sep, bool LeadingZero>
LEXY_CONSTEVAL auto integer(_digits<Base, Sep, LeadingZero>)
{
    return _integer<_digits<Base, Sep, LeadingZero>, T>{};
}
} // namespace lexyd

#endif // LEXY_DSL_INTEGER_HPP_INCLUDED
