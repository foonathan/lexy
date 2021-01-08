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
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            // Add the flag to the context.
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
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
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
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            context.get(Id{}) = !context.get(Id{});
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id>
struct _ctx_fcheck : branch_base
{
    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = false;

        template <typename Context>
        constexpr bool match(Context& context, Reader&)
        {
            return context.get(Id{});
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Tag, bool Value>
struct _ctx_frequire : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            if (context.get(Id{}) == Value)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
            {
                auto err = lexy::make_error<Reader, Tag>(reader.cur());
                return LEXY_MOV(context).error(err);
            }
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
    LEXY_CONSTEVAL auto create() const
    {
        return _ctx_fcreate<id, InitialValue>{};
    }

    LEXY_CONSTEVAL auto set() const
    {
        return _ctx_fset<id, true>{};
    }
    LEXY_CONSTEVAL auto reset() const
    {
        return _ctx_fset<id, false>{};
    }

    LEXY_CONSTEVAL auto toggle() const
    {
        return _ctx_ftoggle<id>{};
    }

    LEXY_CONSTEVAL auto check() const
    {
        return _ctx_fcheck<id>{};
    }

    template <typename ErrorTag>
    LEXY_CONSTEVAL auto require() const
    {
        return _ctx_frequire<id, ErrorTag, true>{};
    }
    template <bool Value, typename ErrorTag>
    LEXY_CONSTEVAL auto require() const
    {
        return _ctx_frequire<id, ErrorTag, Value>{};
    }
};

/// Declares a flag.
template <typename Id>
constexpr auto context_flag = _ctx_flag<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED

