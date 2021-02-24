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
struct _list_loop;
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
template <typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop<Item, _sep<Sep>, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the separator.
            using sep_parser = lexy::rule_parser<Sep, _list_sink>;
            if (auto result = sep_parser::try_parse(context, reader, sink);
                result == lexy::rule_try_parse_result::backtracked)
                break;
            else if (result == lexy::rule_try_parse_result::canceled)
                return false;

            // Parse item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if (!item_parser::parse(context, reader, sink))
                return false;
        }

        return _list_finish<NextParser, PrevArgs...>::parse(context, reader, LEXY_FWD(args)...,
                                                            sink);
    }
};
template <typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop<Item, _tsep<Sep>, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the separator.
            using sep_parser = lexy::rule_parser<Sep, _list_sink>;
            if (auto result = sep_parser::try_parse(context, reader, sink);
                result == lexy::rule_try_parse_result::backtracked)
                break;
            else if (result == lexy::rule_try_parse_result::canceled)
                return false;

            // Can't assume that we have an item, as we could have a trailing separator.
            // So try parsing an item.
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
template <typename Item, typename Sep, typename Tag, typename NextParser, typename... PrevArgs>
struct _list_loop<Item, _ntsep<Sep, Tag>, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the separator.
            using sep_parser = lexy::rule_parser<Sep, _list_sink>;
            auto sep_begin   = reader.cur();
            if (auto result = sep_parser::try_parse(context, reader, sink);
                result == lexy::rule_try_parse_result::backtracked)
                break;
            else if (result == lexy::rule_try_parse_result::canceled)
                return false;
            auto sep_end = reader.cur();

            // Try parsing an item.
            using item_parser = lexy::rule_parser<Item, _list_sink>;
            if (auto result = item_parser::try_parse(context, reader, sink);
                result == lexy::rule_try_parse_result::backtracked)
            {
                // We had a trailing separator, which is not allowed.
                auto err = lexy::make_error<Reader, Tag>(sep_begin, sep_end);
                context.error(err);
                return false;
            }
            else if (result == lexy::rule_try_parse_result::canceled)
                return false;
        }

        return _list_finish<NextParser, PrevArgs...>::parse(context, reader, LEXY_FWD(args)...,
                                                            sink);
    }
};

// Loop to parse all list items when we have a terminator.
template <typename Term, typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop_term;
template <typename Term, typename Item, typename NextParser, typename... PrevArgs>
struct _list_loop_term<Term, Item, void, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the terminator.
            using term_parser = lexy::rule_parser<Term, _list_finish<NextParser, PrevArgs...>>;
            if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, return that result.
                return static_cast<bool>(result);
            }

            // Parse the next item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if (!item_parser::parse(context, reader, sink))
                return false;
        }

        return false; // unreachable
    }
};
template <typename Term, typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop_term<Term, Item, _sep<Sep>, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the terminator.
            using term_parser = lexy::rule_parser<Term, _list_finish<NextParser, PrevArgs...>>;
            if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, return that result.
                return static_cast<bool>(result);
            }

            // Parse the separator.
            using sep_parser = typename lexy::rule_parser<Sep, _list_sink>;
            if (!sep_parser::parse(context, reader, sink))
                return false;

            // Parse the next item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if (!item_parser::parse(context, reader, sink))
                return false;
        }

        return false; // unreachable
    }
};
template <typename Term, typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop_term<Term, Item, _tsep<Sep>, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the terminator.
            using term_parser = lexy::rule_parser<Term, _list_finish<NextParser, PrevArgs...>>;
            if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, return that result.
                return static_cast<bool>(result);
            }

            // Parse the separator.
            using sep_parser = typename lexy::rule_parser<Sep, _list_sink>;
            if (!sep_parser::parse(context, reader, sink))
                return false;

            // Try parsing the terminator again, to determine whether we have a trailing separator.
            if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, return that result.
                return static_cast<bool>(result);
            }

            // Parse the next item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if (!item_parser::parse(context, reader, sink))
                return false;
        }

        return false; // unreachable
    }
};
template <typename Term, typename Item, typename Sep, typename Tag, typename NextParser,
          typename... PrevArgs>
struct _list_loop_term<Term, Item, _ntsep<Sep, Tag>, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the terminator.
            using term_parser = lexy::rule_parser<Term, _list_finish<NextParser, PrevArgs...>>;
            if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, return that result.
                return static_cast<bool>(result);
            }

            // Parse the separator.
            auto sep_begin   = reader.cur();
            using sep_parser = typename lexy::rule_parser<Sep, _list_sink>;
            if (!sep_parser::parse(context, reader, sink))
                return false;
            auto sep_end = reader.cur();

            // Try parsing the terminator again, to determine whether we have a trailing separator.
            if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had a trailing separator, which is not allowed.
                auto err = lexy::make_error<Reader, Tag>(sep_begin, sep_end);
                context.error(err);
                return false;
            }

            // Parse the next item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if (!item_parser::parse(context, reader, sink))
                return false;
        }

        return false; // unreachable
    }
};

// Parse the entire list, i.e. head followed by loop.
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
} // namespace lexyd

namespace lexyd
{
/// Parses a list of items without a separator.
template <typename Item>
LEXY_CONSTEVAL auto list(Item)
{
    static_assert(lexy::is_branch<Item>, "list() without a separator requires a branch condition");
    return _lst<Item, void>{};
}

/// Parses a list of items with the specified separator.
template <typename Item, typename Sep>
LEXY_CONSTEVAL auto list(Item, _sep<Sep>)
{
    return _lst<Item, _sep<Sep>>{};
}

/// Parses a list of items with the specified separator that can be trailing.
template <typename Item, typename Sep>
LEXY_CONSTEVAL auto list(Item, _tsep<Sep>)
{
    static_assert(lexy::is_branch<Item>,
                  "list() without a trailing separator requires a branch condition");
    return _lst<Item, _tsep<Sep>>{};
}

/// Parses a list of items with the specified separator that cannot be trailing.
template <typename Item, typename Sep, typename Tag>
LEXY_CONSTEVAL auto list(Item, _ntsep<Sep, Tag>)
{
    static_assert(lexy::is_branch<Item>,
                  "list() with forbidden trailing separator requires a branch condition");
    return _lst<Item, _ntsep<Sep, Tag>>{};
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
template <typename Term, typename Item, typename Sep>
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
            using continuation = _list_loop_term<Term, Item, Sep, NextParser, Args...>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
        }
    };
};

template <typename Term, typename Item, typename Sep>
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
                using continuation = _list_loop_term<Term, Item, Sep, NextParser, Args...>;
                return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
            }
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

