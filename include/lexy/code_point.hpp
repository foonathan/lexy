// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CODE_POINT_HPP_INCLUDED
#define LEXY_CODE_POINT_HPP_INCLUDED

#include <cstdint>
#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>

#ifndef LEXY_HAS_UNICODE_DATABASE
#    define LEXY_HAS_UNICODE_DATABASE 0
#endif

#if LEXY_HAS_UNICODE_DATABASE
#    define LEXY_UNICODE_CONSTEXPR constexpr
#else
#    define LEXY_UNICODE_CONSTEXPR
#endif

namespace lexy
{
/// A unicode code point.
class code_point
{
public:
    constexpr code_point() noexcept : _value(0xFFFF'FFFF) {}
    constexpr explicit code_point(char32_t value) noexcept : _value(value) {}

    constexpr auto value() const noexcept
    {
        return _value;
    }

    //=== classification ===//
    constexpr bool is_ascii() const noexcept
    {
        return _value <= 0x7F;
    }
    constexpr bool is_bmp() const noexcept
    {
        return _value <= 0xFFFF;
    }
    constexpr bool is_valid() const noexcept
    {
        return _value <= 0x10'FFFF;
    }

    constexpr bool is_control() const noexcept
    {
        return _value <= 0x1F || (0x7F <= _value && _value <= 0x9F);
    }
    constexpr bool is_surrogate() const noexcept
    {
        return 0xD800 <= _value && _value <= 0xDFFF;
    }
    constexpr bool is_private_use() const noexcept
    {
        return (0xE000 <= _value && _value <= 0xF8FF)
               || (0x0F'0000 <= _value && _value <= 0x0F'FFFD)
               || (0x10'0000 <= _value && _value <= 0x10'FFFD);
    }
    constexpr bool is_noncharacter() const noexcept
    {
        // Contiguous range of 32 non-characters.
        if (0xFDD0 <= _value && _value <= 0xFDEF)
            return true;

        // Last two code points of every plane.
        auto in_plane = _value & 0xFFFF;
        return in_plane == 0xFFFE || in_plane == 0xFFFF;
    }

    constexpr bool is_scalar() const noexcept
    {
        return is_valid() && !is_surrogate();
    }

    //=== general category ===//
    enum general_category_t
    {
    // NOLINTNEXTLINE: can't use parentheses here
#define LEXY_UNICODE_CATEGORY(Short, Long) Short, Long = Short

        LEXY_UNICODE_CATEGORY(Lu, uppercase_letter),
        LEXY_UNICODE_CATEGORY(Ll, lowercase_letter),
        LEXY_UNICODE_CATEGORY(Lt, titlecase_letter),
        LEXY_UNICODE_CATEGORY(Lm, modifier_letter),
        LEXY_UNICODE_CATEGORY(Lo, other_letter),

        LEXY_UNICODE_CATEGORY(Mn, nonspacing_mark),
        LEXY_UNICODE_CATEGORY(Mc, spaing_mark),
        LEXY_UNICODE_CATEGORY(Me, enclosing_mark),

        LEXY_UNICODE_CATEGORY(Nd, decimal_number),
        LEXY_UNICODE_CATEGORY(Nl, letter_number),
        LEXY_UNICODE_CATEGORY(No, other_number),

        LEXY_UNICODE_CATEGORY(Pc, connector_punctuation),
        LEXY_UNICODE_CATEGORY(Pd, dash_punctuation),
        LEXY_UNICODE_CATEGORY(Ps, open_punctuation),
        LEXY_UNICODE_CATEGORY(Pe, closing_punctuation),
        LEXY_UNICODE_CATEGORY(Pi, initial_puncutation),
        LEXY_UNICODE_CATEGORY(Pf, final_puncutation),
        LEXY_UNICODE_CATEGORY(Po, other_punctuation),

        LEXY_UNICODE_CATEGORY(Sm, math_symbol),
        LEXY_UNICODE_CATEGORY(Sc, currency_symbol),
        LEXY_UNICODE_CATEGORY(Sk, modifier_symbol),
        LEXY_UNICODE_CATEGORY(So, other_symbol),

        LEXY_UNICODE_CATEGORY(Zs, space_separator),
        LEXY_UNICODE_CATEGORY(Zl, line_separator),
        LEXY_UNICODE_CATEGORY(Zp, paragraph_separator),

        LEXY_UNICODE_CATEGORY(Cc, control),
        LEXY_UNICODE_CATEGORY(Cf, format),
        LEXY_UNICODE_CATEGORY(Cs, surrogate),
        LEXY_UNICODE_CATEGORY(Co, private_use),
        LEXY_UNICODE_CATEGORY(Cn, unassigned),

#undef LEXY_UNICODE_CATEGORY
    };

    template <general_category_t... Cats>
    struct _gc_group
    {
        const char* name;

        friend constexpr bool operator==(_gc_group, general_category_t cat)
        {
            return ((cat == Cats) || ...);
        }
        friend constexpr bool operator==(general_category_t cat, _gc_group)
        {
            return ((cat == Cats) || ...);
        }

        friend constexpr bool operator!=(_gc_group, general_category_t cat)
        {
            return !(_gc_group{} == cat);
        }
        friend constexpr bool operator!=(general_category_t cat, _gc_group)
        {
            return !(_gc_group{} == cat);
        }
    };

#define LEXY_UNICODE_CATEGORY_GROUP(Name, Short, Long, ...)                                        \
    static constexpr auto Short = _gc_group<__VA_ARGS__>{"code-point." Name};                      \
    static constexpr auto Long  = Short

    LEXY_UNICODE_CATEGORY_GROUP("cased-letter", LC, cased_letter, Lu, Ll, Lt);
    LEXY_UNICODE_CATEGORY_GROUP("letter", L, letter, Lu, Ll, Lt, Lm, Lo);
    LEXY_UNICODE_CATEGORY_GROUP("mark", M, mark, Mn, Mc, Me);
    LEXY_UNICODE_CATEGORY_GROUP("number", N, number, Nd, Nl, No);
    LEXY_UNICODE_CATEGORY_GROUP("punctuation", P, punctuation, Pc, Pd, Ps, Pe, Pi, Pf, Po);
    LEXY_UNICODE_CATEGORY_GROUP("symbol", S, symbol, Sm, Sc, Sk, So);
    LEXY_UNICODE_CATEGORY_GROUP("separator", Z, separator, Zs, Zl, Zp);
    LEXY_UNICODE_CATEGORY_GROUP("other", C, other, Cc, Cf, Cs, Co, Cn);

#undef LEXY_UNICODE_CATEGORY_GROUP

    LEXY_UNICODE_CONSTEXPR general_category_t general_category() const;

    //=== comparision ===//
    friend constexpr bool operator==(code_point lhs, code_point rhs) noexcept
    {
        return lhs._value == rhs._value;
    }
    friend constexpr bool operator!=(code_point lhs, code_point rhs) noexcept
    {
        return lhs._value != rhs._value;
    }

private:
    char32_t _value;
};
} // namespace lexy

#if LEXY_HAS_UNICODE_DATABASE
#    include <lexy/_detail/unicode_database.hpp>

constexpr lexy::code_point::general_category_t lexy::code_point::general_category() const
{
    if (!is_valid())
        return general_category_t::unassigned;

    auto idx = _unicode_db::property_index(_value);
    return _unicode_db::category[idx];
}
#endif

#endif // LEXY_CODE_POINT_HPP_INCLUDED

