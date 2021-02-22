// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexyd
{
template <typename Id, int InitialValue>
struct _ctx_ccreate : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
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
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
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
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader,
                                     typename Reader::iterator begin, Args&&... args)
            {
                auto end    = reader.cur();
                auto length = lexy::_detail::range_size(begin, end);

                context.get(Id{}) += int(length) * Sign;

                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, _cont>::parse(context, reader, reader.cur(),
                                                         LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Value, typename R, typename S, typename T>
struct _ctx_ccompare : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (context.get(Id{}) < Value)
                return lexy::rule_parser<R, NextParser>::parse(context, reader, LEXY_FWD(args)...);
            else if (context.get(Id{}) == Value)
                return lexy::rule_parser<S, NextParser>::parse(context, reader, LEXY_FWD(args)...);
            else // context.get(Id{}) > Value
                return lexy::rule_parser<T, NextParser>::parse(context, reader, LEXY_FWD(args)...);
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
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (context.get(Id{}) == Value)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
            {
                auto err = lexy::make_error<Reader, Tag>(reader.cur());
                context.error(err);
                return false;
            }
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Id, int Value>
struct _ctx_counter_require
{
    template <typename Tag>
    static constexpr _ctx_crequire<Id, Tag, Value> error = {};
};

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

    template <int Value, typename R, typename S, typename T>
    LEXY_CONSTEVAL auto compare(R, S, T) const
    {
        return _ctx_ccompare<id, Value, R, S, T>{};
    }

    template <int Value = 0>
    LEXY_CONSTEVAL auto require() const
    {
        return _ctx_counter_require<id, Value>{};
    }

    template <typename Tag>
    LEXY_DEPRECATED_ERROR("replace `counter.require<Tag>()` by `counter.require().error<Tag>`")
    LEXY_CONSTEVAL auto require() const
    {
        return require().template error<Tag>;
    }
    template <int Value, typename Tag>
    LEXY_DEPRECATED_ERROR(
        "replace `counter.require<Value, Tag>()` by `counter.require<Value>().error<Tag>`")
    LEXY_CONSTEVAL auto require() const
    {
        return require<Value>().template error<Tag>;
    }
};

/// Declares an integer counter that is added to the parsing context.
template <typename Id>
constexpr auto context_counter = _ctx_counter<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED

