// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Id>
using _ctx_flag = lexy::_detail::parse_context_var<Id, bool>;

template <typename Id, bool InitialValue>
struct _ctx_fcreate : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _ctx_flag<Id> var(InitialValue);
            var.link(context);
            auto result = NextParser::parse(context, reader, LEXY_FWD(args)...);
            var.unlink(context);
            return result;
        }
    };
};

template <typename Id, bool Value>
struct _ctx_fset : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _ctx_flag<Id>::get(context) = Value;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id>
struct _ctx_ftoggle : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto& flag = _ctx_flag<Id>::get(context);
            flag       = !flag;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, bool Value>
struct _ctx_fis : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        constexpr bool try_parse(Context& context, const Reader&)
        {
            return _ctx_flag<Id>::get(context) == Value;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    using p = NextParser;
};

template <typename Id>
struct _ctx_fvalue : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                     _ctx_flag<Id>::get(context));
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Id>
struct _ctx_flag_dsl
{
    template <bool InitialValue = false>
    constexpr auto create() const
    {
        return _ctx_fcreate<Id, InitialValue>{};
    }

    constexpr auto set() const
    {
        return _ctx_fset<Id, true>{};
    }
    constexpr auto reset() const
    {
        return _ctx_fset<Id, false>{};
    }

    constexpr auto toggle() const
    {
        return _ctx_ftoggle<Id>{};
    }

    constexpr auto is_set() const
    {
        return _ctx_fis<Id, true>{};
    }
    constexpr auto is_reset() const
    {
        return _ctx_fis<Id, false>{};
    }

    constexpr auto value() const
    {
        return _ctx_fvalue<Id>{};
    }
};

/// Declares a flag.
template <typename Id>
constexpr auto context_flag = _ctx_flag_dsl<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED

