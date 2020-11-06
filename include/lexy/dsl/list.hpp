// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/separator.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
template <typename Rule>
struct _it : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename... ParentArgs>
        struct _continuation
        {
            template <typename Handler, typename Reader, typename Sink, typename... Args>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink,
                                     ParentArgs&&... pargs, Args&&... args) ->
                typename Handler::result_type
            {
                if constexpr (sizeof...(Args) > 0)
                    sink(LEXY_FWD(args)...); // Forward item args to the sink.
                return NextParser::parse(handler, reader, sink, LEXY_FWD(pargs)...); // Continue.
            }
        };

        template <typename Handler, typename Reader, typename Sink, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, Args&&... args) ->
            typename Handler::result_type
        {
            return Rule::template parser<_continuation<Args...>>::parse(handler, reader, sink,
                                                                        LEXY_FWD(args)...);
        }
    };
};

/// Parses the rule as a list item.
/// Most be used inside of a `make_list()` rule.
template <typename Rule>
LEXY_CONSTEVAL auto item(Rule)
{
    return _it<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Rule>
struct _lst : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Handler, typename Reader, typename Sink, typename... Args>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, Args&&... args)
                -> typename Handler::result_type
            {
                if constexpr (std::is_same_v<typename Sink::return_type, void>)
                {
                    LEXY_MOV(sink).finish();
                    return NextParser::parse(handler, reader, LEXY_FWD(args)...);
                }
                else
                {
                    return NextParser::parse(handler, reader, LEXY_FWD(args)...,
                                             LEXY_MOV(sink).finish());
                }
            }
        };

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            auto sink = handler.list_sink();
            return Rule::template parser<_continuation>::parse(handler, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };
};

/// Matches the rule and stores its values using the list interface.
/// Subrules marked as `item()` will be forwarded to the list sink.
template <typename Rule>
LEXY_CONSTEVAL auto build_list(Rule)
{
    return _lst<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
// This rule is being looped for the list.
template <typename Item, typename Sep>
struct _lstl;
template <typename Item>
struct _lstl<Item, void> : rule_base // no separator
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename Sink, typename... Args>
        LEXY_DSL_FUNC auto parse(_loop_handler<Handler>& handler, Reader& reader, Sink& sink,
                                 Args&&... args) -> typename _loop_handler<Handler>::result_type
        {
            using branch = decltype(branch(Item{}));

            if (!branch::condition_matcher::match(reader))
                // Item doesn't match, done with the list.
                return LEXY_MOV(handler).break_();

            using rule = decltype(item(branch::then()));
            return rule::template parser<NextParser>::parse(handler, reader, sink,
                                                            LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep, bool Capture>
struct _lstl<Item, _sep<Sep, Capture>> : rule_base // normal separator
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename Sink, typename... Args>
        LEXY_DSL_FUNC auto parse(_loop_handler<Handler>& handler, Reader& reader, Sink& sink,
                                 Args&&... args) -> typename _loop_handler<Handler>::result_type
        {
            auto begin = reader.cur();
            if (!Sep::matcher::match(reader))
                // No separator, done with the list.
                return LEXY_MOV(handler).break_();

            if constexpr (Capture)
                sink(lexy::lexeme(reader, begin));
            else
                (void)begin;

            using rule = decltype(item(Item{}));
            return rule::template parser<NextParser>::parse(handler, reader, sink,
                                                            LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep, bool Capture>
struct _lstl<Item, _tsep<Sep, Capture>> : rule_base // trailing separator
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename Sink, typename... Args>
        LEXY_DSL_FUNC auto parse(_loop_handler<Handler>& handler, Reader& reader, Sink& sink,
                                 Args&&... args) -> typename _loop_handler<Handler>::result_type
        {
            using branch = decltype(branch(Item{}));

            auto begin = reader.cur();
            if (!Sep::matcher::match(reader))
                // No separator, done with the list.
                return LEXY_MOV(handler).break_();
            else
                (void)begin;

            if constexpr (Capture)
                sink(lexy::lexeme(reader, begin));

            if (!branch::condition_matcher::match(reader))
                // The condition didn't match, the separator above was the trailing one.
                return LEXY_MOV(handler).break_();

            using rule = decltype(item(branch::then()));
            return rule::template parser<NextParser>::parse(handler, reader, sink,
                                                            LEXY_FWD(args)...);
        }
    };
};

/// Creates a list of items without a separator.
template <typename Item>
LEXY_CONSTEVAL auto list(Item it)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a separator requires a branch condition");
    auto b = branch(it);

    auto head = item(b.then());
    auto tail = loop(_lstl<Item, void>{});
    return b.condition() >> build_list(head + tail);
}

/// Creates a list of items with the specified separator.
template <typename Item, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(Item it, _sep<Pattern, Capture>)
{
    if constexpr (lexy::is_branch_rule<Item>)
    {
        auto b = branch(it);

        auto head = item(b.then());
        auto tail = loop(_lstl<Item, _sep<Pattern, Capture>>{});
        return b.condition() >> build_list(head + tail);
    }
    else
    {
        auto head = item(it);
        auto tail = loop(_lstl<Item, _sep<Pattern, Capture>>{});
        return build_list(head + tail);
    }
}

/// Creates a list of items with the specified separator that can be trailing.
template <typename Item, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(Item it, _tsep<Pattern, Capture>)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a trailing separator requires a branch condition");
    auto b = branch(it);

    auto head = item(b.then());
    auto tail = loop(_lstl<Item, _tsep<Pattern, Capture>>{});
    return b.condition() >> build_list(head + tail);
}
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED
