// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_UNICODE_HPP_INCLUDED
#define LEXY_DSL_UNICODE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/code_point.hpp>

#if LEXY_HAS_UNICODE_DATABASE
#    define LEXY_UNICODE_PROPERTY_PREDICATE(Prop)                                                  \
        constexpr bool operator()(lexy::code_point cp) const                                       \
        {                                                                                          \
            using namespace lexy::_unicode_db;                                                     \
            auto idx  = property_index(cp.value());                                                \
            auto mask = binary_properties[idx];                                                    \
                                                                                                   \
            return (mask & (1 << lexy::_unicode_db::Prop)) != 0;                                   \
        }
#else
#    define LEXY_UNICODE_PROPERTY_PREDICATE(Prop) bool operator()(lexy::code_point cp) const;
#endif

namespace lexyd::unicode
{
inline constexpr auto control = code_point.general_category<lexy::code_point::control>();

//=== whitespace ===//
struct _blank
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.blank";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        if (cp.value() == '\t')
            return true;
        return cp.general_category() == lexy::code_point::space_separator;
    }
};
inline constexpr auto blank = code_point.if_<_blank>();

struct _newline
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.newline";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        // ASCII newlines.
        return cp.value() == '\n'
               || cp.value() == '\r'
               // NEL, PARAGRAPH SEPARATOR, LINE SEPARATOR
               || cp.value() == 0x85 || cp.value() == 0x2029 || cp.value() == 0x2028;
    }
};
inline constexpr auto newline = code_point.if_<_newline>();

struct _other_space
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.other-space";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        // The same as in ASCII.
        return cp.value() == '\f' || cp.value() == '\v';
    }
};
inline constexpr auto other_space = code_point.if_<_other_space>();

struct _space
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.whitespace";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(whitespace)
};
inline constexpr auto space = code_point.if_<_space>();

//=== alpha ===//
struct _lower
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.lowercase";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(lowercase)
};
inline constexpr auto lower = code_point.if_<_lower>();

struct _upper
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.uppercase";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(uppercase)
};
inline constexpr auto upper = code_point.if_<_upper>();

struct _alpha
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.alphabetic";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(alphabetic)
};
inline constexpr auto alpha = code_point.if_<_alpha>();

//=== digit ===//
inline constexpr auto digit = code_point.general_category<lexy::code_point::decimal_number>();

struct _alnum
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.alphabetic-decimal";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        return _alpha{}(cp) || cp.general_category() == lexy::code_point::decimal_number;
    }
};
inline constexpr auto alnum       = code_point.if_<_alnum>();
inline constexpr auto alpha_digit = alnum;

struct _word
{
    struct _join
    {
        LEXY_UNICODE_PROPERTY_PREDICATE(join_control)
    };

    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.word";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        return _alnum{}(cp) || cp.general_category() == lexy::code_point::mark
               || cp.general_category() == lexy::code_point::connector_punctuation || _join{}(cp);
    }
};
inline constexpr auto word = code_point.if_<_word>();

//=== categories ===//
struct _graph
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.graph";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        // everything that isn't control, surrogate, unassigned, or space.
        return !cp.is_control() && !cp.is_surrogate()
               && cp.general_category() != lexy::code_point::unassigned && !_space{}(cp);
    }
};
inline constexpr auto graph = code_point.if_<_graph>();

struct _print
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.print";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        // blank or graph without control
        return !cp.is_control() && (_blank{}(cp) || _graph{}(cp));
    }
};
inline constexpr auto print = code_point.if_<_print>();

struct _char
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.character";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        return cp.general_category() != lexy::code_point::unassigned;
    }
};
inline constexpr auto character = code_point.if_<_char>();
} // namespace lexyd::unicode

namespace lexyd::unicode
{
struct _xid_start
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.XID-start";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(xid_start)
};
inline constexpr auto xid_start = code_point.if_<_xid_start>();

struct _xid_start_underscore
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.XID-start-underscore";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        return cp.value() == '_' || _xid_start{}(cp);
    }
};
inline constexpr auto xid_start_underscore = code_point.if_<_xid_start_underscore>();

struct _xid_continue
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.XID-continue";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(xid_continue)
};
inline constexpr auto xid_continue = code_point.if_<_xid_continue>();
} // namespace lexyd::unicode

#undef LEXY_UNICODE_PROPERTY_PREDICATE

#endif // LEXY_DSL_UNICODE_HPP_INCLUDED

