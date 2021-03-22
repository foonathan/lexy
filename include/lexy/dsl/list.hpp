// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/separator.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
// Final parser for the list.
// It will create the final value.
template <typename NextParser, typename... PrevArgs>
struct _list_finish
{
    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... prev_args, Sink& sink,
                             Args&&... args)
    {
        if constexpr (std::is_same_v<typename Sink::return_type, void>)
        {
            LEXY_MOV(sink).finish();
            return NextParser::parse(context, reader, LEXY_FWD(prev_args)..., LEXY_FWD(args)...);
        }
        else
        {
            return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                     LEXY_MOV(sink).finish(), LEXY_FWD(args)...);
        }
    }
};

// Final parser for a list item.
// It will forward the arguments to the sink.
struct _list_sink
{
    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC bool parse(Context&, Reader&, Sink& sink, Args&&... args)
    {
        if constexpr (sizeof...(Args) > 0)
            sink(LEXY_FWD(args)...);
        return true;
    }
};

// Loop to parse all items.
template <typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the separator.
            auto sep_pos     = reader.cur();
            using sep_parser = lexy::rule_parser<typename Sep::rule, _list_sink>;
            if (auto result = sep_parser::try_parse(context, reader, sink);
                result == lexy::rule_try_parse_result::backtracked)
                break;
            else if (result == lexy::rule_try_parse_result::canceled)
                return false;

            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if constexpr (Item::is_branch)
            {
                // Try parsing the next item.
                if (auto result = item_parser::try_parse(context, reader, sink);
                    result == lexy::rule_try_parse_result::backtracked)
                {
                    // If trailing seperators are allowed, this does nothing.
                    // Otherwise, we report the error but can trivially recover.
                    Sep::report_trailing_error(context, reader, sep_pos);
                    break;
                }
                else if (result == lexy::rule_try_parse_result::canceled)
                    return false;
            }
            else
            {
                (void)sep_pos;
                // If the item isn't a branch, we parse it unconditionally.
                // This only happens with `sep()`.
                if (!item_parser::parse(context, reader, sink))
                    return false;
            }
        }

        return _list_finish<NextParser, PrevArgs...>::parse(context, reader, LEXY_FWD(args)...,
                                                            sink);
    }
};
template <typename Item, typename NextParser, typename... PrevArgs>
struct _list_loop<Item, void, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the item.
            using item_parser = lexy::rule_parser<Item, _list_sink>;
            if (auto result = item_parser::try_parse(context, reader, sink);
                result == lexy::rule_try_parse_result::backtracked)
                break;
            else if (result == lexy::rule_try_parse_result::canceled)
                return false;
        }

        return _list_finish<NextParser, PrevArgs...>::parse(context, reader, LEXY_FWD(args)...,
                                                            sink);
    }
};

// Loop to parse all list items when we have a terminator.
template <typename Term, typename Item, typename Sep, typename RecoveryLimit, typename NextParser,
          typename... PrevArgs>
struct _list_loop_term
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        // We're using an enum together with a switch to compensate a lack of goto in constexpr.
        // The simple state machine goes as follows on well-formed input:
        // terminator -> separator -> separator_trailing_check -> item -> terminator
        //
        // The interesting case is error recovery.
        // There we skip over characters until we either found the terminator, separator or item.
        // We then set the enum to jump to the appropriate state of the state machine.
        enum class state
        {
            terminator,
            separator,
            separator_trailing_check,
            item,
            recovery,
        } state
            = state::terminator;

        auto sep_pos = reader.cur();
        while (true)
        {
            switch (state)
            {
            case state::terminator:
                using term_parser = lexy::rule_parser<Term, _list_finish<NextParser, PrevArgs...>>;
                if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                    result != lexy::rule_try_parse_result::backtracked)
                {
                    // We had the terminator, so the list is done.
                    return static_cast<bool>(result);
                }
                // Parse the following list separator next.
                state = state::separator;
                break;

            case state::separator:
                if constexpr (!std::is_void_v<Sep>)
                {
                    sep_pos = reader.cur();

                    using sep_parser = typename lexy::rule_parser<typename Sep::rule, _list_sink>;
                    if (!sep_parser::parse(context, reader, sink))
                        return false;

                    // Check for a trailing separator next.
                    state = state::separator_trailing_check;
                }
                else
                {
                    // No separator, immediately parse item next.
                    (void)sep_pos;
                    state = state::item;
                }
                break;
            case state::separator_trailing_check:
                if constexpr (!std::is_void_v<Sep>)
                {
                    if (auto result
                        = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                        result != lexy::rule_try_parse_result::backtracked)
                    {
                        // If trailing seperators are allowed, this does nothing.
                        // Otherwise, we report the error but can trivially recover.
                        Sep::report_trailing_error(context, reader, sep_pos);
                        // We had the terminator, so the list is done.
                        return static_cast<bool>(result);
                    }

                    // Now parse the item next.
                    state = state::item;
                }
                break;

            case state::item:
                using item_parser = typename lexy::rule_parser<Item, _list_sink>;
                if (item_parser::parse(context, reader, sink))
                    // Loop back and try again for the next item.
                    state = state::terminator;
                else
                    // Recover the error.
                    state = state::recovery;
                break;

            case state::recovery:
                while (true)
                {
                    // Recovery succeeds when we reach the next separator.
                    if constexpr (!std::is_void_v<Sep>)
                    {
                        sep_pos = reader.cur();

                        using sep_parser =
                            typename lexy::rule_parser<typename Sep::rule, _list_sink>;
                        if (auto result = sep_parser::try_parse(context, reader, sink);
                            result == lexy::rule_try_parse_result::ok)
                        {
                            // Continue the list with the trailing separator check.
                            state = state::separator_trailing_check;
                            break;
                        }

                        // Here we either try something else or start recovering from a failed
                        // separator.
                    }
                    // When we don't have a separator, but the item is a branch, we also succeed
                    // when we reach the next item.
                    else if constexpr (Item::is_branch)
                    {
                        if (auto result = item_parser::try_parse(context, reader, sink);
                            result == lexy::rule_try_parse_result::ok)
                        {
                            // Continue the list with the next terminator check.
                            state = state::terminator;
                            break;
                        }

                        // Here we either try something else or start recovering from yet another
                        // failed item.
                    }

                    // Recovery succeeds when we reach the terminator.
                    if (auto result
                        = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                        result != lexy::rule_try_parse_result::backtracked)
                    {
                        // We're now done with the entire list.
                        return static_cast<bool>(result);
                    }

                    // Recovery fails when we reach the limit.
                    using limit = typename decltype(RecoveryLimit{}.get_limit())::token_engine;
                    if (lexy::engine_peek<limit>(reader))
                        return false;

                    // Consume one character and try again.
                    reader.bump();
                }
                break;
            }
        }

        return false; // unreachable
    }
};
} // namespace lexyd

namespace lexyd
{
template <typename Item, typename Sep>
struct _lst : rule_base
{
    static constexpr auto is_branch               = Item::is_branch;
    static constexpr auto is_unconditional_branch = Item::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto sink = context.sink();

            // Try parsing the initial item.
            using item_parser = lexy::rule_parser<Item, _list_sink>;
            if (auto result = item_parser::try_parse(context, reader, sink);
                result != lexy::rule_try_parse_result::ok)
                // Either we backtracked the item, in which case we also backtrack.
                // Or we failed parsing the item, in which case we also fail.
                return result;

            // Continue with the rest of the items.
            using continuation = _list_loop<Item, Sep, NextParser, Args...>;
            return static_cast<lexy::rule_try_parse_result>(
                continuation::parse(context, reader, LEXY_FWD(args)..., sink));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.sink();

            // Parse the initial item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if (!item_parser::parse(context, reader, sink))
                return false;

            // Continue with the rest of the items.
            using continuation = _list_loop<Item, Sep, NextParser, Args...>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
        }
    };
};

/// Parses a list of items without a separator.
template <typename Item>
LEXY_CONSTEVAL auto list(Item)
{
    static_assert(lexy::is_branch<Item>, "list() without a separator requires a branch condition");
    return _lst<Item, void>{};
}

/// Parses a list of items with the specified separator.
template <typename Item, typename Sep, typename Tag>
LEXY_CONSTEVAL auto list(Item, _sep<Sep, Tag>)
{
    return _lst<Item, _sep<Sep, Tag>>{};
}

/// Parses a list of items with the specified separator that can be trailing.
template <typename Item, typename Sep>
LEXY_CONSTEVAL auto list(Item, _tsep<Sep>)
{
    static_assert(lexy::is_branch<Item>,
                  "list() without a trailing separator requires a branch condition");
    return _lst<Item, _tsep<Sep>>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Item, typename Sep>
struct _olst : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using list_parser = lexy::rule_parser<_lst<Item, Sep>, NextParser>;

            // Try parsing the list.
            if (auto result = list_parser::try_parse(context, reader, LEXY_FWD(args)...);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We didn't backtrack, so its result is our result.
                return static_cast<bool>(result);
            }
            else
            {
                // We don't have a list: construct a sink and immediately finish it.
                auto sink = context.sink();
                return _list_finish<NextParser, Args...>::parse(context, reader, LEXY_FWD(args)...,
                                                                sink);
            }
        }
    };
};

/// Parses a list that might be empty.
template <typename Item>
LEXY_CONSTEVAL auto opt_list(Item)
{
    static_assert(lexy::is_branch<Item>, "opt_list() requires a branch condition");
    return _olst<Item, void>{};
}
template <typename Item, typename Sep>
LEXY_CONSTEVAL auto opt_list(Item, Sep)
{
    static_assert(lexy::is_branch<Item>, "opt_list() requires a branch condition");
    return _olst<Item, Sep>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename Item, typename Sep, typename Recover>
struct _lstt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.sink();

            // Parse initial item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if (!item_parser::parse(context, reader, sink))
                return false;

            // Parse the remaining items.
            using continuation = _list_loop_term<Term, Item, Sep, Recover, NextParser, Args...>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
        }
    };
};

template <typename Term, typename Item, typename Sep, typename Recover>
struct _olstt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.sink();

            // Try parsing the terminator.
            using term_parser = lexy::rule_parser<Term, _list_finish<NextParser, Args...>>;
            if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, and thus created an empty list.
                return static_cast<bool>(result);
            }
            else
            {
                // Parse initial item.
                using item_parser = typename lexy::rule_parser<Item, _list_sink>;
                if (!item_parser::parse(context, reader, sink))
                    return false;

                // Parse the remaining items.
                using continuation = _list_loop_term<Term, Item, Sep, Recover, NextParser, Args...>;
                return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
            }
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

