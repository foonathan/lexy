// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Pattern>
struct _sep
{};

/// Defines a separator pattern for a list.
template <typename Pattern>
LEXY_CONSTEVAL auto sep(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _sep<Pattern>{};
}

template <typename Pattern>
struct _tsep
{};

/// Defines a separator pattern for a list that can be trailing.
template <typename Pattern>
LEXY_CONSTEVAL auto trailing_sep(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _tsep<Pattern>{};
}
} // namespace lexyd

namespace lexyd
{
// Continuation after the entire list has been parsed.
template <typename NextParser>
struct _list_done
{
    template <typename Context, typename Input, typename ListCallback, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Input& input, ListCallback& callback, Args&&... args)
        -> typename Context::result_type
    {
        if constexpr (std::is_same_v<typename ListCallback::return_type, void>)
        {
            LEXY_MOV(callback).finish();
            return NextParser::parse(context, input, LEXY_FWD(args)...);
        }
        else
        {
            return NextParser::parse(context, input, LEXY_FWD(args)...,
                                     LEXY_MOV(callback).finish());
        }
    }
};

// Continuation after a list item has been parsed.
template <typename NextParser, typename... ParentArgs>
struct _list_item_done
{
    template <typename Context, typename Input, typename ListCallback, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Input& input, ListCallback& callback,
                             ParentArgs&&... pargs, Args&&... args) -> typename Context::result_type
    {
        callback.item(LEXY_FWD(args)...); // Forward item args to the callback.
        return NextParser::parse(context, input, callback, LEXY_FWD(pargs)...); // Continue.
    }
};

// Parses the next item if there is one.
template <typename Item, typename Sep, typename NextParser>
struct _list_loop_parser;
template <typename Item, typename NextParser>
struct _list_loop_parser<Item, void, NextParser> // no separator
{
    template <typename Context, typename Input, typename ListCallback, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Input& input, ListCallback& callback, Args&&... args)
        -> typename Context::result_type
    {
        // We parse other items while the condition matches.
        if (Item::condition_matcher::match(input))
        {
            // We parse item, then the done continuation, and then we jump back here.
            using continuation = _list_item_done<_list_loop_parser, Args...>;
            return Item::template then_parser<continuation>::parse(context, input, callback,
                                                                   LEXY_FWD(args)...);
        }
        else
        {
            return _list_done<NextParser>::parse(context, input, callback, LEXY_FWD(args)...);
        }
    }
};
template <typename Item, typename Sep, typename NextParser>
struct _list_loop_parser<Item, _sep<Sep>, NextParser> // normal separator
{
    template <typename Context, typename Input, typename ListCallback, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Input& input, ListCallback& callback, Args&&... args)
        -> typename Context::result_type
    {
        // We parse other items while the separator matches.
        if (Sep::matcher::match(input))
        {
            // We parse item, then the done continuation, and then we jump back here.
            using continuation = _list_item_done<_list_loop_parser, Args...>;
            return Item::template parser<continuation>::parse(context, input, callback,
                                                              LEXY_FWD(args)...);
        }
        else
        {
            return _list_done<NextParser>::parse(context, input, callback, LEXY_FWD(args)...);
        }
    }
};
template <typename Item, typename Sep, typename NextParser>
struct _list_loop_parser<Item, _tsep<Sep>, NextParser> // trailing separator
{
    template <typename Context, typename Input, typename ListCallback, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Input& input, ListCallback& callback, Args&&... args)
        -> typename Context::result_type
    {
        // We parse other items while the separator and condition matches.
        // If only the separator matches but not the condition, this means we've just read the
        // trailing separator.
        if (Sep::matcher::match(input) && Item::condition_matcher::match(input))
        {
            // We parse item, then the done continuation, and then we jump back here.
            using continuation = _list_item_done<_list_loop_parser, Args...>;
            return Item::template then_parser<continuation>::parse(context, input, callback,
                                                                   LEXY_FWD(args)...);
        }
        else
        {
            return _list_done<NextParser>::parse(context, input, callback, LEXY_FWD(args)...);
        }
    }
};
} // namespace lexyd

namespace lexyd
{
template <typename Item, typename Sep>
struct _list : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            auto&& callback = context.list_callback();

            // We need the first item in either case.
            // So we parse it unconditionally, using the item done continuation to pass its
            // arguments to the callback. Once that's done we continue with the proper loop.
            using loop         = _list_loop_parser<Item, Sep, NextParser>;
            using continuation = _list_item_done<loop, Args...>;
            return Item::template parser<continuation>::parse(context, input, callback,
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
    return _list<decltype(branch(item)), void>{};
}

/// Creates a list of items with the specified separator.
template <typename Item, typename Pattern>
LEXY_CONSTEVAL auto list(Item, _sep<Pattern>)
{
    return _list<Item, _sep<Pattern>>{};
}

/// Creates a list of items with the specified separator that can be trailing.
template <typename Item, typename Pattern>
LEXY_CONSTEVAL auto list(Item, _tsep<Pattern>)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a trailing separator requires a branch condition");
    return _list<Item, _tsep<Pattern>>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Item, typename Sep>
struct _opt_list : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            auto&& callback = context.list_callback();

            // The Item is a branch, so we can check whether we have at least one.
            if (Item::condition_matcher::match(input))
            {
                // We parse the remainder of Item and jump in the loop.
                using loop         = _list_loop_parser<Item, Sep, NextParser>;
                using continuation = _list_item_done<loop, Args...>;
                return Item::template then_parser<continuation>::parse(context, input, callback,
                                                                       LEXY_FWD(args)...);
            }
            else
            {
                // We're done.
                return _list_done<NextParser>::parse(context, input, callback, LEXY_FWD(args)...);
            }
        }
    };
};

/// Creates a (possibly empty) list of items without a separator.
template <typename Item>
LEXY_CONSTEVAL auto opt_list(Item item)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "opt_list() without a separator requires a branch condition");
    return _opt_list<decltype(branch(item)), void>{};
}

/// Creates a (possibly empty) list of items with the specified separator.
template <typename Item, typename Pattern>
LEXY_CONSTEVAL auto opt_list(Item, _sep<Pattern>)
{
    return _opt_list<Item, _sep<Pattern>>{};
}

/// Creates a (possibly empty) list of items with the specified separator that can be trailing.
template <typename Item, typename Pattern>
LEXY_CONSTEVAL auto opt_list(Item, _tsep<Pattern>)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "opt_list() without a trailing separator requires a branch condition");
    return _opt_list<Item, _tsep<Pattern>>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

