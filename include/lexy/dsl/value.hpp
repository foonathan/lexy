// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_VALUE_HPP_INCLUDED
#define LEXY_DSL_VALUE_HPP_INCLUDED

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/base.hpp>

namespace lexy
{
struct _match_context;
} // namespace lexy

namespace lexyd
{
template <auto Value>
struct _valc : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_same_v<Context, lexy::_match_context>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., Value);
        }
    };
};

/// Produces the specified value without parsing anything.
template <auto Value>
constexpr auto value_c = _valc<Value>{};
} // namespace lexyd

namespace lexyd
{
template <auto F>
struct _valf : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_same_v<Context, lexy::_match_context>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., F());
        }
    };
};

/// Produces the value returned by the function without parsing anything.
template <auto F>
constexpr auto value_f = _valf<F>{};
} // namespace lexyd

namespace lexyd
{
template <typename T>
struct _valt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_same_v<Context, lexy::_match_context>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., T());
        }
    };
};

/// Produces a default constructed value of the specified type without parsing anything.
template <typename T>
constexpr auto value_t = _valt<T>{};
} // namespace lexyd

namespace lexyd
{
template <typename String>
struct _vals : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_same_v<Context, lexy::_match_context>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
            {
                constexpr auto str = String::get();
                return NextParser::parse(context, reader, LEXY_FWD(args)..., str.data(),
                                         str.size());
            }
        }
    };
};

#if LEXY_HAS_NTTP
/// Produces the string value.
template <lexy::_detail::string_literal Str>
constexpr auto value_str = _vals<lexy::_detail::type_string<Str>>{};
#endif

#define LEXY_VALUE_STR(Str)                                                                        \
    ::lexyd::_vals<LEXY_NTTP_STRING(Str)> {}
} // namespace lexyd

#endif // LEXY_DSL_VALUE_HPP_INCLUDED

