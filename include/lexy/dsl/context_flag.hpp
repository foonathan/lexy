// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Id, bool InitialValue>
struct _ctx_fcreate : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(!Context::contains(Id{}));
            auto flag_ctx = context.insert(Id{}, InitialValue);
            return NextParser::parse(flag_ctx, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, bool Value>
struct _ctx_fset : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            context.get(Id{}) = Value;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id>
struct _ctx_ftoggle : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            context.get(Id{}) = !context.get(Id{});
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, bool Value>
struct _ctx_fis : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser : NextParser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if (context.get(Id{}) != Value)
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        // inherit parse
    };
};

template <typename Id>
struct _ctx_fvalue : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., context.get(Id{}));
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Id>
struct _ctx_flag
{
    struct id
    {};

    template <bool InitialValue = false>
    constexpr auto create() const
    {
        return _ctx_fcreate<id, InitialValue>{};
    }

    constexpr auto set() const
    {
        return _ctx_fset<id, true>{};
    }
    constexpr auto reset() const
    {
        return _ctx_fset<id, false>{};
    }

    constexpr auto toggle() const
    {
        return _ctx_ftoggle<id>{};
    }

    constexpr auto is_set() const
    {
        return _ctx_fis<id, true>{};
    }
    constexpr auto is_reset() const
    {
        return _ctx_fis<id, false>{};
    }

    constexpr auto value() const
    {
        return _ctx_fvalue<id>{};
    }
};

/// Declares a flag.
template <typename Id>
constexpr auto context_flag = _ctx_flag<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED

