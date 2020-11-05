// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/separator.hpp>
#include <lexy/lexeme.hpp>
#include <lexy/result.hpp>

namespace lexyd
{
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

template <typename Rule>
struct _list_item : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename Sink, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, Args&&... args) ->
            typename Handler::result_type
        {
            using continuation = _list_item_done<NextParser, Args...>;
            return Rule::template parser<continuation>::parse(handler, reader, sink,
                                                              LEXY_FWD(args)...);
        }
    };
};

/// Parses the rule as a list item.
/// Most be used inside of a `make_list()` rule.
template <typename Rule>
LEXY_CONSTEVAL auto item(Rule)
{
    return _list_item<Rule>{};
}
} // namespace lexyd

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

template <typename Rule>
struct _mklist : rule_base
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

            using continuation = _list_done<NextParser>;
            return Rule::template parser<continuation>::parse(handler, reader, sink,
                                                              LEXY_FWD(args)...);
        }
    };
};

/// Matches the rule and stores its values using the list interface.
/// Subrules marked as `item()` will be forwarded to the list sink.
template <typename Rule>
LEXY_CONSTEVAL auto make_list(Rule)
{
    return _mklist<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
// The handler used for parsing the list item.
// It will package any error nicely and return it to us.
template <typename Handler>
struct _list_item_handler
{
    Handler& _handler;

    using result_type = lexy::optional_error<typename Handler::result_type>;

    template <typename SubProduction, typename Reader>
    constexpr auto sub_handler(const Reader& reader)
    {
        return _handler.template sub_handler<SubProduction>(reader);
    }

    constexpr auto list_sink()
    {
        return _handler.sink();
    }

    template <typename Reader, typename Error>
    constexpr auto error(const Reader& reader, Error&& error) &&
    {
        // We report errors to the normal handler.
        return result_type(lexy::result_error, LEXY_MOV(_handler).error(reader, LEXY_FWD(error)));
    }

    template <typename... Args>
    constexpr auto value(Args&&...) &&
    {
        // We create a default constructed result.
        return result_type(lexy::result_value);
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
        using continuation = _list_item_done<lexy::final_parser, Args...>;
        _list_item_handler<Handler> item_handler{handler};

        // We parse other items while the condition matches.
        while (Item::condition_matcher::match(reader))
        {
            auto result = Item::template then_parser<continuation>::parse(item_handler, reader,
                                                                          sink, LEXY_FWD(args)...);
            if (result.has_error())
                return LEXY_MOV(result).error();
        }

        // We're done with the list, finish by finalizing the sink.
        return _list_done<NextParser>::parse(handler, reader, sink, LEXY_FWD(args)...);
    }
};
template <typename Item, typename Sep, bool Capture, typename NextParser>
struct _list_loop_parser<Item, _sep<Sep, Capture>, NextParser> // normal separator
{
    template <typename Handler, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, Args&&... args) ->
        typename Handler::result_type
    {
        using continuation = _list_item_done<lexy::final_parser, Args...>;
        _list_item_handler<Handler> item_handler{handler};

        while (true)
        {
            auto begin = reader.cur();
            if (!Sep::matcher::match(reader))
                // No separator, done with the list.
                break;

            if constexpr (Capture)
                sink(lexy::lexeme(reader, begin));

            auto result = Item::template parser<continuation>::parse(item_handler, reader, sink,
                                                                     LEXY_FWD(args)...);
            if (result.has_error())
                return LEXY_MOV(result).error();
        }

        return _list_done<NextParser>::parse(handler, reader, sink, LEXY_FWD(args)...);
    }
};
template <typename Item, typename Sep, bool Capture, typename NextParser>
struct _list_loop_parser<Item, _tsep<Sep, Capture>, NextParser> // trailing separator
{
    template <typename Handler, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Sink& sink, Args&&... args) ->
        typename Handler::result_type
    {
        using continuation = _list_item_done<lexy::final_parser, Args...>;
        _list_item_handler<Handler> item_handler{handler};

        while (true)
        {
            auto begin = reader.cur();
            if (!Sep::matcher::match(reader))
                // No separator, done with the list.
                break;

            if constexpr (Capture)
                sink(lexy::lexeme(reader, begin));

            if (!Item::condition_matcher::match(reader))
                // The condition didn't match, the separator above was the trailing one.
                break;

            auto result = Item::template then_parser<continuation>::parse(item_handler, reader,
                                                                          sink, LEXY_FWD(args)...);
            if (result.has_error())
                return LEXY_MOV(result).error();
        }

        return _list_done<NextParser>::parse(handler, reader, sink, LEXY_FWD(args)...);
    }
};

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
    return branch_rule::condition() >> _list<decltype(branch_rule::then()), branch_rule, void>{};
}

/// Creates a list of items with the specified separator.
template <typename Item, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(Item item, _sep<Pattern, Capture>)
{
    if constexpr (lexy::is_branch_rule<Item>)
    {
        using branch_rule = decltype(branch(item));

        // We turn the condition of the first item into a condition for the entire list.
        return branch_rule::condition()
               >> _list<decltype(branch_rule::then()), branch_rule, _sep<Pattern, Capture>>{};
    }
    else
        // We don't have a condition.
        return _list<Item, Item, _sep<Pattern, Capture>>{};
}

/// Creates a list of items with the specified separator that can be trailing.
template <typename Item, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(Item item, _tsep<Pattern, Capture>)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a trailing separator requires a branch condition");
    using branch_rule = decltype(branch(item));

    // We turn the condition of the first item into a condition for the entire list.
    return branch_rule::condition()
           >> _list<decltype(branch_rule::then()), branch_rule, _tsep<Pattern, Capture>>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

