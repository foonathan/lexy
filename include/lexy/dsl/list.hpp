// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/separator.hpp>

namespace lexyd
{
// Continuation after the entire list has been parsed.
template <typename NextParser>
struct _list_done
{
    template <typename Handler, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, Args&&... args) ->
        typename Handler::result_type
    {
        if constexpr (std::is_same_v<typename Sink::return_type, void>)
        {
            LEXY_MOV(sink).finish();
            return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
        else
        {
            return NextParser::parse(handler, reader, LEXY_FWD(args)..., LEXY_MOV(sink).finish());
        }
    }
};

// Continuation after a list item has been parsed.
template <typename NextParser, typename... ParentArgs>
struct _list_item_done
{
    template <typename Handler, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, ParentArgs&&... pargs,
                             Args&&... args) -> typename Handler::result_type
    {
        if constexpr (sizeof...(Args) > 0)
            sink(LEXY_FWD(args)...); // Forward item args to the sink.
        return NextParser::parse(handler, reader, sink, LEXY_FWD(pargs)...); // Continue.
    }
};

// Parses the next item if there is one.
template <typename Item, typename Sep, typename NextParser>
struct _list_loop_parser;
template <typename Item, typename NextParser>
struct _list_loop_parser<Item, void, NextParser> // no separator
{
    template <typename Handler, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, Args&&... args) ->
        typename Handler::result_type
    {
        // We parse other items while the condition matches.
        if (Item::condition_matcher::match(reader))
        {
            // We parse item, then the done continuation, and then we jump back here.
            using continuation = _list_item_done<_list_loop_parser, Args...>;
            return Item::template then_parser<continuation>::parse(handler, reader, sink,
                                                                   LEXY_FWD(args)...);
        }
        else
        {
            return _list_done<NextParser>::parse(handler, reader, sink, LEXY_FWD(args)...);
        }
    }
};
template <typename Item, typename Sep, typename NextParser>
struct _list_loop_parser<Item, _sep<Sep>, NextParser> // normal separator
{
    template <typename Handler, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, Args&&... args) ->
        typename Handler::result_type
    {
        // We parse other items while the separator matches.
        if (Sep::matcher::match(reader))
        {
            // We parse item, then the done continuation, and then we jump back here.
            using continuation = _list_item_done<_list_loop_parser, Args...>;
            return Item::template parser<continuation>::parse(handler, reader, sink,
                                                              LEXY_FWD(args)...);
        }
        else
        {
            return _list_done<NextParser>::parse(handler, reader, sink, LEXY_FWD(args)...);
        }
    }
};
template <typename Item, typename Sep, typename NextParser>
struct _list_loop_parser<Item, _tsep<Sep>, NextParser> // trailing separator
{
    template <typename Handler, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, Args&&... args) ->
        typename Handler::result_type
    {
        // We parse other items while the separator and condition matches.
        // If only the separator matches but not the condition, this means we've just read the
        // trailing separator.
        if (Sep::matcher::match(reader) && Item::condition_matcher::match(reader))
        {
            // We parse item, then the done continuation, and then we jump back here.
            using continuation = _list_item_done<_list_loop_parser, Args...>;
            return Item::template then_parser<continuation>::parse(handler, reader, sink,
                                                                   LEXY_FWD(args)...);
        }
        else
        {
            return _list_done<NextParser>::parse(handler, reader, sink, LEXY_FWD(args)...);
        }
    }
};
} // namespace lexyd

namespace lexyd
{
template <typename First, typename Item, typename Sep>
struct _list : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            auto sink = handler.list_sink();

            // We need the first item in either case.
            // So we parse it unconditionally, using the item done continuation to pass its
            // arguments to the sink. Once that's done we continue with the proper loop.
            using loop         = _list_loop_parser<Item, Sep, NextParser>;
            using continuation = _list_item_done<loop, Args...>;
            return First::template parser<continuation>::parse(handler, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };
};

/// Creates a list of items without a separator.
template <typename Item>
LEXY_CONSTEVAL auto list(Item item)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a separator requires a branch condition");
    using branch_rule = decltype(branch(item));

    // We turn the condition of the first item into a condition for the entire list.
    return
        typename branch_rule::condition{} >> _list<typename branch_rule::then, branch_rule, void>{};
}

/// Creates a list of items with the specified separator.
template <typename Item, typename Pattern>
LEXY_CONSTEVAL auto list(Item item, _sep<Pattern>)
{
    if constexpr (lexy::is_branch_rule<Item>)
    {
        using branch_rule = decltype(branch(item));

        // We turn the condition of the first item into a condition for the entire list.
        return typename branch_rule::condition{}
               >> _list<typename branch_rule::then, branch_rule, _sep<Pattern>>{};
    }
    else
        // We don't have a condition.
        return _list<Item, Item, _sep<Pattern>>{};
}

/// Creates a list of items with the specified separator that can be trailing.
template <typename Item, typename Pattern>
LEXY_CONSTEVAL auto list(Item item, _tsep<Pattern>)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a trailing separator requires a branch condition");
    using branch_rule = decltype(branch(item));

    // We turn the condition of the first item into a condition for the entire list.
    return typename branch_rule::condition{}
           >> _list<typename branch_rule::then, branch_rule, _tsep<Pattern>>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

