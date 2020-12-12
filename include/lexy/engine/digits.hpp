// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_DIGITS_HPP_INCLUDED
#define LEXY_ENGINE_DIGITS_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Match one or more of the specified digits.
template <typename DigitSet>
struct engine_digits : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet>);

    using error_code = typename DigitSet::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        // We need at least one digit.
        if (auto ec = DigitSet::match(reader); ec != error_code())
            return ec;

        // Match subsequent digits as often as possible.
        while (engine_try_match<DigitSet>(reader))
        {}

        return error_code();
    }
};

/// Match one or more of the specified digits with digit separator in between.
template <typename DigitSet, typename Sep>
struct engine_digits_sep : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet> && lexy::engine_is_matcher<Sep>);

    using error_code = typename DigitSet::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        // We need at least one digit.
        if (auto ec = DigitSet::match(reader); ec != error_code())
            return ec;

        while (true)
        {
            if (engine_try_match<Sep>(reader))
            {
                // After the separator we need another digit.
                if (auto ec = DigitSet::match(reader); ec != error_code())
                    return ec;
            }
            else if (!engine_try_match<DigitSet>(reader))
                // If we didn't have a separator, digit is optional and terminates.
                break;
        }

        return error_code();
    }
};
} // namespace lexy

namespace lexy
{
/// Match one or more of the specified digits, trimmed from unnecessary leading zeroes.
template <typename DigitSet, typename Zero>
struct engine_digits_trimmed : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet> && lexy::engine_is_matcher<Zero>);

    enum class error_code
    {
        leading_zero = 1,
    };

    static constexpr error_code translate(typename DigitSet::error_code ec)
    {
        LEXY_PRECONDITION(ec != typename DigitSet::error_code());
        return error_code(int(ec) + 1);
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        if (engine_try_match<Zero>(reader))
        {
            // Check if it is followed by another digit, which is not allowed.
            if (engine_peek<DigitSet>(reader))
                return error_code::leading_zero;
            else
                return error_code();
        }
        else
        {
            // We need at least one digit.
            if (auto ec = DigitSet::match(reader); ec != typename DigitSet::error_code())
                return translate(ec);

            // Match subsequent digits as often as possible.
            while (engine_try_match<DigitSet>(reader))
            {}

            return error_code();
        }
    }
};

/// Match one or more of the specified digits optionally separated, trimmed from unnecessary leading
/// zeroes.
template <typename DigitSet, typename Zero, typename Sep>
struct engine_digits_trimmed_sep : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<
                      DigitSet> && lexy::engine_is_matcher<Zero> && lexy::engine_is_matcher<Sep>);

    enum class error_code
    {
        leading_zero = 1,
    };

    static constexpr error_code translate(typename DigitSet::error_code ec)
    {
        LEXY_PRECONDITION(ec != typename DigitSet::error_code());
        return error_code(int(ec) + 1);
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        if (engine_try_match<Zero>(reader))
        {
            // Check if it is followed by another digit, which is not allowed.
            if (engine_peek<DigitSet>(reader) || engine_peek<Sep>(reader))
                return error_code::leading_zero;
            else
                return error_code();
        }
        else
        {
            // We need at least one digit.
            if (auto ec = DigitSet::match(reader); ec != typename DigitSet::error_code())
                return translate(ec);

            while (true)
            {
                if (engine_try_match<Sep>(reader))
                {
                    // After the separator we need another digit.
                    if (auto ec = DigitSet::match(reader); ec != typename DigitSet::error_code())
                        return translate(ec);
                }
                else if (!engine_try_match<DigitSet>(reader))
                    // If we didn't have a separator, digit is optional and terminates.
                    break;
            }

            return error_code();
        }
    }
};
} // namespace lexy

namespace lexy
{
/// Matches exactly N digits.
template <std::size_t N, typename DigitSet>
struct engine_ndigits : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet>);
    static_assert(N > 0);

    using error_code = typename DigitSet::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        switch (N)
        {
        case 4:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            // fallthrough
        case 3:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            // fallthrough
        case 2:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            // fallthrough
        case 1:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            break;

        default:
            for (auto i = 0u; i != N; ++i)
            {
                if (auto ec = DigitSet::match(reader); ec != error_code())
                    return ec;
            }
            break;
        }

        return error_code();
    }
};

/// Matches exactly N digits optionally separated.
template <std::size_t N, typename DigitSet, typename Sep>
struct engine_ndigits_sep : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet> && lexy::engine_is_matcher<Sep>);
    static_assert(N > 0);

    using error_code = typename DigitSet::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        switch (N)
        {
        case 4:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            engine_try_match<Sep>(reader);
            // fallthrough
        case 3:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            engine_try_match<Sep>(reader);
            // fallthrough
        case 2:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            engine_try_match<Sep>(reader);
            // fallthrough
        case 1:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            break;

        default:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;

            for (auto i = 1u; i != N; ++i)
            {
                engine_try_match<Sep>(reader);

                if (auto ec = DigitSet::match(reader); ec != error_code())
                    return ec;
            }

            break;
        }

        return error_code();
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_DIGITS_HPP_INCLUDED

