// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/separator.hpp>

namespace lexyd
{
template <typename Item, typename Sep>
struct _lst : _copy_base<Item>
{
    template <typename Context, typename Reader, typename Sink>
    static constexpr bool _loop(Context& context, Reader& reader, Sink& sink)
    {
        while (true)
        {
            // Parse a separator if necessary.
            [[maybe_unused]] auto sep_pos = reader.position();
            if constexpr (!std::is_void_v<Sep>)
            {
                lexy::branch_parser_for<typename Sep::rule, Context, Reader> sep{};
                if (!sep.try_parse(context, reader))
                    // We didn't have a separator, list is definitely finished.
                    break;

                if (!sep.template finish<lexy::sink_parser>(context, reader, sink))
                    return false;
            }

            // Parse the next item.
            if constexpr (lexy::is_branch_rule<Item>)
            {
                // It's a branch, so try parsing it to detect loop exit.
                lexy::branch_parser_for<Item, Context, Reader> item{};
                if (!item.try_parse(context, reader))
                {
                    // We don't have a next item, exit the loop.
                    // If necessary, we report a trailing separator.
                    if constexpr (!std::is_void_v<Sep>)
                        Sep::report_trailing_error(context, reader, sep_pos);
                    break;
                }

                // We're having an item, finish it.
                if (!item.template finish<lexy::sink_parser>(context, reader, sink))
                    return false;
            }
            else
            {
                // Not a branch, so we need one item.
                if (!lexy::parser_for<Item, lexy::sink_parser>::parse(context, reader, sink))
                    return false;
            }
        }

        return true;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Construct the sink.
            auto sink = context.on(_ev::list{}, reader.position());

            // Parse the first item.
            if (!lexy::parser_for<Item, lexy::sink_parser>::parse(context, reader, sink))
                return false;

            // Parse the remaining items.
            if (!_loop(context, reader, sink))
                return false;

            // We're done with the list, finish the sink and continue.
            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Item, Context, Reader> item;

        constexpr bool try_parse(Context& context, const Reader& reader)
        {
            // We parse a list if we can parse its first item.
            return item.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // At this point, we have a list so construct a sink.
            auto sink = context.on(_ev::list{}, reader.position());

            // Finish the first item, passing all values to the sink.
            if (!item.template finish<lexy::sink_parser>(context, reader, sink))
                return false;

            // Parse the remaining items.
            if (!_loop(context, reader, sink))
                return false;

            // We're done with the list, finish the sink and continue.
            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };
};

/// Parses a list of items without a separator.
template <typename Item>
constexpr auto list(Item)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a separator requires a branch condition");
    return _lst<Item, void>{};
}

/// Parses a list of items with the specified separator.
template <typename Item, typename Sep, typename Tag>
constexpr auto list(Item, _sep<Sep, Tag>)
{
    return _lst<Item, _sep<Sep, Tag>>{};
}

/// Parses a list of items with the specified separator that can be trailing.
template <typename Item, typename Sep>
constexpr auto list(Item, _tsep<Sep>)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a trailing separator requires a branch condition");
    return _lst<Item, _tsep<Sep>>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename Item, typename Sep, typename Recover>
struct _lstt : rule_base
{
    template <typename TermParser, typename Context, typename Reader, typename Sink>
    static constexpr bool _loop(TermParser& term, Context& context, Reader& reader, Sink& sink)
    {
        // We're using an enum together with a switch to compensate a lack of goto in constexpr.
        // The simple state machine goes as follows on well-formed input:
        // terminator -> separator -> separator_trailing_check -> item -> terminator -> ... ->
        // done
        //
        // The interesting case is error recovery.
        // There we skip over characters until we either found the terminator, separator or
        // item. We then set the enum to jump to the appropriate state of the state machine.
        enum class state
        {
            terminator,
            separator,
            separator_trailing_check,
            item,
            recovery,
        } state
            = state::terminator;

        [[maybe_unused]] auto sep_pos = reader.position();
        while (true)
        {
            switch (state)
            {
            case state::terminator:
                if (term.try_parse(context, reader))
                    // We had the terminator, so the list is done.
                    return true;

                // Parse the following list separator next.
                state = state::separator;
                break;

            case state::separator:
                if constexpr (!std::is_void_v<Sep>)
                {
                    sep_pos = reader.position();
                    if (lexy::parser_for<typename Sep::rule, lexy::sink_parser>::parse(context,
                                                                                       reader,
                                                                                       sink))
                    {
                        // Check for a trailing separator next.
                        state = state::separator_trailing_check;
                        break;
                    }
                    else if (sep_pos == reader.position())
                    {
                        // We don't have a separator at all.
                        // Assume it's missing and parse an item instead.

                        if constexpr (lexy::is_branch_rule<Item>)
                        {
                            lexy::branch_parser_for<Item, Context, Reader> item{};
                            if (!item.try_parse(context, reader))
                            {
                                // Not an item, recover.
                                state = state::recovery;
                                break;
                            }

                            if (!item.template finish<lexy::sink_parser>(context, reader, sink))
                            {
                                // Failed item, recover.
                                state = state::recovery;
                                break;
                            }

                            // Continue after an item has been parsed.
                            state = state::terminator;
                            break;
                        }
                        else
                        {
                            // We cannot try and parse an item.
                            // To avoid generating wrong errors, immediately recover.
                            state = state::recovery;
                            break;
                        }
                    }
                    else
                    {
                        // We did have something that looked like a separator initially, but
                        // wasn't one on closer inspection. Enter generic recovery as we've
                        // already consumed input. (If we ignore the case where the item and
                        // separator share a common prefix, we know it wasn't the start of an
                        // item so can't just pretend that there is one).
                        state = state::recovery;
                        break;
                    }
                }
                else
                {
                    // List doesn't have a separator; immediately parse item next.
                    state = state::item;
                    break;
                }

            case state::separator_trailing_check:
                if constexpr (!std::is_void_v<Sep>)
                {
                    // We need to check whether we're having a trailing separator by checking
                    // for a terminating one.
                    if (term.try_parse(context, reader))
                    {
                        // We had the terminator, so the list is done.
                        // Report a trailing separator error if necessary.
                        Sep::report_trailing_error(context, reader, sep_pos);
                        return true;
                    }
                    else
                    {
                        // We didn't have a separator, parse item next.
                        state = state::item;
                        break;
                    }
                }
                break;

            case state::item:
                if (lexy::parser_for<Item, lexy::sink_parser>::parse(context, reader, sink))
                {
                    // Loop back.
                    state = state::terminator;
                    break;
                }
                else
                {
                    // Recover from missing item.
                    state = state::recovery;
                    break;
                }

            case state::recovery:
                context.on(_ev::recovery_start{}, reader.position());
                while (true)
                {
                    // Recovery succeeds when we reach the next separator.
                    if constexpr (!std::is_void_v<Sep>)
                    {
                        sep_pos = reader.position();

                        lexy::branch_parser_for<typename Sep::rule, Context, Reader> sep{};
                        if (sep.try_parse(context, reader)
                            && sep.template finish<lexy::sink_parser>(context, reader, sink))
                        {
                            // Continue the list with the trailing separator check.
                            context.on(_ev::recovery_finish{}, reader.position());
                            state = state::separator_trailing_check;
                            break;
                        }
                        else
                        {
                            // Need to continue recovering.
                            // This might include recovery of an additional separator error.
                        }
                    }
                    // When we don't have a separator, but the item is a branch, we also succeed
                    // when we reach the next item.
                    //
                    // Note that we're doing this check only if we don't have a separator.
                    // If we do have one, the heuristic "end of the invalid item" is better than
                    // "beginning of the next one".
                    else if constexpr (lexy::is_branch_rule<Item>)
                    {
                        lexy::branch_parser_for<Item, Context, Reader> item{};
                        if (item.try_parse(context, reader)
                            && item.template finish<lexy::sink_parser>(context, reader, sink))
                        {
                            // Continue the list with the next terminator check.
                            context.on(_ev::recovery_finish{}, reader.position());
                            state = state::terminator;
                            break;
                        }
                        else
                        {
                            // Need to continue recovering.
                            // This might include recovery of an additional item error.
                        }
                    }

                    // At this point, we couldn't detect the next item.
                    // Recovery succeeds when we reach the terminator.
                    if (term.try_parse(context, reader))
                    {
                        // We're now done with the entire list.
                        context.on(_ev::recovery_finish{}, reader.position());
                        return true;
                    }

                    // At this point, we couldn't detect the next item or a terminator.
                    // Recovery fails when we reach the limit.
                    lexy::branch_parser_for<decltype(Recover{}.get_limit()), Context, Reader>
                        limit{};
                    if (limit.try_parse(context, reader))
                    {
                        // Recovery has failed, propagate error.
                        context.on(_ev::recovery_cancel{}, reader.position());
                        return false;
                    }

                    // Consume one code unit and try again.
                    auto begin = reader.position();
                    reader.bump();
                    auto end = reader.position();
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end);
                }
                break;
            }
        }

        return false; // unreachable
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.on(_ev::list{}, reader.position());

            // Parse initial item.
            using item_parser = lexy::parser_for<Item, lexy::sink_parser>;
            if (!item_parser::parse(context, reader, sink))
                return false;

            lexy::branch_parser_for<Term, Context, Reader> term{};

            // Parse the remaining items.
            if (!_loop(term, context, reader, sink))
                return false;

            // At this point, we just need to finish parsing the terminator.
            return term.template finish<lexy::sink_finish_parser<NextParser>>(context, reader, sink,
                                                                              LEXY_FWD(args)...);
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

