// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Id, int InitialValue>
struct _ctx_ccreate : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            // Add the counter to the context.
            auto counter_ctx = context.insert(Id{}, InitialValue);
            return NextParser::parse(counter_ctx, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Delta>
struct _ctx_cadd : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            // Add the flag to the context.
            context.get(Id{}) += Delta;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Rule, int Sign>
struct _ctx_cpush : rule_base
{
    template <typename NextParser>
    struct parser
    {
        struct _cont
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC auto parse(Context& context, Reader& reader,
                                     typename Reader::iterator begin, Args&&... args) ->
                typename Context::result_type
            {
                auto end    = reader.cur();
                auto length = lexy::_detail::range_size(begin, end);

                context.get(Id{}) += int(length) * Sign;

                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            return lexy::rule_parser<Rule, _cont>::parse(context, reader, reader.cur(),
                                                         LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Value, int Sign>
struct _ctx_ccomp : branch_base
{
    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = false;

        template <typename Context>
        constexpr bool match(Context& context, Reader&)
        {
            if constexpr (Sign == 0)
                return context.get(Id{}) == Value;
            else if constexpr (Sign > 0)
                return context.get(Id{}) > Value;
            else
                return context.get(Id{}) < Value;
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Tag, int Value>
struct _ctx_crequire : rule_base
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
struct _ctx_counter
{
    struct id
    {};

    template <int InitialValue = 0>
    LEXY_CONSTEVAL auto create() const
    {
        return _ctx_ccreate<id, InitialValue>{};
    }

    LEXY_CONSTEVAL auto inc() const
    {
        return _ctx_cadd<id, +1>{};
    }
    LEXY_CONSTEVAL auto dec() const
    {
        return _ctx_cadd<id, -1>{};
    }

    template <typename Rule>
    LEXY_CONSTEVAL auto push(Rule) const
    {
        return _ctx_cpush<id, Rule, +1>{};
    }
    template <typename Rule>
    LEXY_CONSTEVAL auto pop(Rule) const
    {
        return _ctx_cpush<id, Rule, -1>{};
    }

    template <int Value>
    LEXY_CONSTEVAL auto check_eq() const
    {
        return _ctx_ccomp<id, Value, 0>{};
    }
    template <int Value>
    LEXY_CONSTEVAL auto check_lt() const
    {
        return _ctx_ccomp<id, Value, -1>{};
    }
    template <int Value>
    LEXY_CONSTEVAL auto check_gt() const
    {
        return _ctx_ccomp<id, Value, +1>{};
    }

    template <typename ErrorTag>
    LEXY_CONSTEVAL auto require() const
    {
        return _ctx_crequire<id, ErrorTag, 0>{};
    }
    template <int Value, typename ErrorTag>
    LEXY_CONSTEVAL auto require() const
    {
        return _ctx_crequire<id, ErrorTag, Value>{};
    }
};

/// Declares an integer counter that is added to the parsing context.
template <typename Id>
constexpr auto context_counter = _ctx_counter<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED

