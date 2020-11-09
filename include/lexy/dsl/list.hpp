// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/choice.hpp>
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
    if constexpr (lexy::is_branch_rule<Rule>)
    {
        auto b = branch(Rule{});
        return b.condition() >> _it<decltype(b.then())>{};
    }
    else
    {
        return _it<Rule>{};
    }
}
template <typename... Rs>
LEXY_CONSTEVAL auto item(_chc<Rs...>)
{
    return (item(Rs{}) | ...);
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
// Takes care of the separator logic in a list.
template <typename Pattern, bool Capture>
struct _lsts : rule_base
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
            if (!Pattern::matcher::match(reader))
                // No separator, done with the list.
                return LEXY_MOV(handler).break_();

            if constexpr (Capture)
                sink(lexy::lexeme(reader, begin));
            else
                (void)begin;

            return NextParser::parse(handler, reader, sink, LEXY_FWD(args)...);
        }
    };
};

// Takes care of the item in a list if it's a branch.
template <typename Item>
struct _lsti : rule_base
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

/// Creates a list of items without a separator.
template <typename Item>
LEXY_CONSTEVAL auto list(Item it)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a separator requires a branch condition");
    auto b = branch(it);

    auto head = item(b.then());
    auto tail = loop(_lsti<Item>{});
    return b.condition() >> build_list(head + tail);
}
template <typename... Items>
LEXY_CONSTEVAL auto list(_chc<Items...> choice)
{
    auto head = item(choice);
    auto tail = loop(item(choice | else_ >> break_));
    return build_list(head + tail);
}

/// Creates a list of items with the specified separator.
template <typename Item, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(Item it, _sep<Pattern, Capture>)
{
    if constexpr (lexy::is_branch_rule<Item>)
    {
        auto b = branch(it);

        auto head = item(b.then());
        auto tail = loop(_lsts<Pattern, Capture>{} + item(it));
        return b.condition() >> build_list(head + tail);
    }
    else
    {
        auto head = item(it);
        auto tail = loop(_lsts<Pattern, Capture>{} + item(it));
        return build_list(head + tail);
    }
}
template <typename... Items, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(_chc<Items...> choice, _sep<Pattern, Capture>)
{
    auto head = item(choice);
    auto tail = loop(_lsts<Pattern, Capture>{} + item(choice));
    return build_list(head + tail);
}

/// Creates a list of items with the specified separator that can be trailing.
template <typename Item, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(Item it, _tsep<Pattern, Capture>)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a trailing separator requires a branch condition");
    auto b = branch(it);

    auto head = item(b.then());
    auto tail = loop(_lsts<Pattern, Capture>{} + _lsti<Item>{});
    return b.condition() >> build_list(head + tail);
}
template <typename... Items, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(_chc<Items...> choice, _tsep<Pattern, Capture>)
{
    auto head = item(choice);
    auto tail = loop(_lsts<Pattern, Capture>{} + item(choice | else_ >> break_));
    return build_list(head + tail);
}
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED
