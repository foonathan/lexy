// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED

#include <lexy/_detail/iterator.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/error.hpp>

namespace lexy
{
struct unequal_counts
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unequal counts";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Id, int InitialValue>
struct _ctx_ccreate : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(!Context::contains(Id{}));
            lexy::_detail::parse_context_var counter_ctx(context, Id{}, InitialValue);
            return NextParser::parse(counter_ctx, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Delta>
struct _ctx_cadd : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            context.get(Id{}) += Delta;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Rule, int Sign>
struct _ctx_cpush : _copy_base<Rule>
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           typename Reader::iterator begin, Args&&... args)
        {
            auto end    = reader.position();
            auto length = lexy::_detail::range_size(begin, end);

            context.get(Id{}) += int(length) * Sign;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Context, Reader> rule;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Forward to the rule.
            return rule.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but remember the current reader position.
            return rule.template finish<_pc<NextParser>>(context, reader, reader.position(),
                                                         LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but remember the current reader position.
            using parser = lexy::parser_for<Rule, _pc<NextParser>>;
            return parser::parse(context, reader, reader.position(), LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Value>
struct _ctx_cis : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        constexpr bool try_parse(const Context& context, const Reader&)
        {
            return context.get(Id{}) == Value;
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
struct _ctx_cvalue : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., context.get(Id{}));
        }
    };
};

template <typename... Ids>
struct _ctx_ceq;
template <typename H, typename... T>
struct _ctx_ceq<H, T...> : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        constexpr bool try_parse(const Context& context, const Reader&)
        {
            auto value = context.get(H{});
            return ((value == context.get(T{})) && ...);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto value = context.get(H{});
            if (((value != context.get(T{})) || ...))
            {
                auto err = lexy::error<Reader, lexy::unequal_counts>(reader.position());
                context.on(_ev::error{}, err);
                // Trivially recover.
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
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
    constexpr auto create() const
    {
        return _ctx_ccreate<id, InitialValue>{};
    }

    constexpr auto inc() const
    {
        return _ctx_cadd<id, +1>{};
    }
    constexpr auto dec() const
    {
        return _ctx_cadd<id, -1>{};
    }

    template <typename Rule>
    constexpr auto push(Rule) const
    {
        return _ctx_cpush<id, Rule, +1>{};
    }
    template <typename Rule>
    constexpr auto pop(Rule) const
    {
        return _ctx_cpush<id, Rule, -1>{};
    }

    template <int Value>
    constexpr auto is() const
    {
        return _ctx_cis<id, Value>{};
    }
    constexpr auto is_zero() const
    {
        return is<0>();
    }

    constexpr auto value() const
    {
        return _ctx_cvalue<id>{};
    }
};

/// Declares an integer counter that is added to the parsing context.
template <typename Id>
constexpr auto context_counter = _ctx_counter<Id>{};

/// Takes a branch only if all counters are equal.
template <typename... Ids>
constexpr auto equal_counts(_ctx_counter<Ids>...)
{
    static_assert(sizeof...(Ids) > 1);
    return _ctx_ceq<typename _ctx_counter<Ids>::id...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED

