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
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
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
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

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
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            return lexy::rule_parser<Rule, _cont>::try_parse(context, reader, reader.cur(),
                                                             LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, _cont>::parse(context, reader, reader.cur(),
                                                         LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Value>
struct _ctx_cis : rule_base
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
struct _ctx_cvalue : rule_base
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

template <typename... Ids>
struct _ctx_ceq;
template <typename H, typename... T>
struct _ctx_ceq<H, T...> : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto value = context.get(H{});
            if (((value != context.get(T{})) || ...))
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto value = context.get(H{});
            if (((value != context.get(T{})) || ...))
            {
                auto err = lexy::error<Reader, lexy::unequal_counts>(reader.cur());
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

