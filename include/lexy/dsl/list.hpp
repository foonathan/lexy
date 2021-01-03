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
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        if constexpr (std::is_same_v<typename Sink::return_type, void>)
        {
            LEXY_MOV(sink).finish();
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., LEXY_MOV(sink).finish());
        }
    }

    template <typename Branch, typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC auto parse_branch(Branch& branch, Context& context, Reader& reader,
                                    PrevArgs&&... args, Sink& sink)
    {
        if constexpr (std::is_same_v<typename Sink::return_type, void>)
        {
            LEXY_MOV(sink).finish();
            return branch.template parse<NextParser>(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            return branch.template parse<NextParser>(context, reader, LEXY_FWD(args)...,
                                                     LEXY_MOV(sink).finish());
        }
    }
};

template <typename... PrevArgs>
struct _list_sink
{
    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC auto parse(Context&, Reader&, PrevArgs&&..., Sink& sink, Args&&... args)
    {
        if constexpr (sizeof...(Args) > 0)
            sink(LEXY_FWD(args)...);
        return typename Context::result_type(lexy::result_empty);
    }
};

// Loop to parse all items.
template <typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop;
template <typename Item, typename NextParser, typename... PrevArgs>
struct _list_loop<Item, void, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink) ->
        typename Context::result_type
    {
        using sink_parser = _list_sink<PrevArgs...>;

        while (true)
        {
            lexy::branch_matcher<Item, Reader> branch{};
            if (!branch.match(reader))
                // No longer match additional items, done with list.
                break;

            auto result
                = branch.template parse<sink_parser>(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);
        }

        return _list_finish<NextParser, PrevArgs...>::parse(context, reader, LEXY_FWD(args)...,
                                                            sink);
    }
};
template <typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop<Item, _sep<Sep>, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink) ->
        typename Context::result_type
    {
        using sink_parser = _list_sink<PrevArgs...>;

        while (true)
        {
            // Check whether we have a separator.
            lexy::branch_matcher<Sep, Reader> sep{};
            if (!sep.match(reader))
                break;

            // Parse the separator.
            auto result = sep.template parse<sink_parser>(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);

            // Parse item.
            using item_parser = typename lexy::rule_parser<Item, sink_parser>;
            result            = item_parser::parse(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);
        }

        return _list_finish<NextParser, PrevArgs...>::parse(context, reader, LEXY_FWD(args)...,
                                                            sink);
    }
};
template <typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop<Item, _tsep<Sep>, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink) ->
        typename Context::result_type
    {
        using sink_parser = _list_sink<PrevArgs...>;

        while (true)
        {
            // Check whether we have a separator.
            lexy::branch_matcher<Sep, Reader> sep{};
            if (!sep.match(reader))
                break;

            // Parse the separator.
            auto result = sep.template parse<sink_parser>(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);

            // Parse item.
            lexy::branch_matcher<Item, Reader> branch{};
            if (!branch.match(reader))
                // No longer match additional items, done with list.
                break;

            result = branch.template parse<sink_parser>(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);
        }

        return _list_finish<NextParser, PrevArgs...>::parse(context, reader, LEXY_FWD(args)...,
                                                            sink);
    }
};

// Parse the entire list, i.e. head followed by loop.
template <typename Item, typename Sep>
struct _lst : rule_base
{
    static constexpr auto is_branch = lexy::is_branch<Item>;

    template <typename Reader>
    struct branch_matcher
    {
        lexy::branch_matcher<Item, Reader> _impl;

        static constexpr auto is_unconditional = decltype(_impl)::is_unconditional;

        constexpr bool match(Reader& reader)
        {
            return _impl.match(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.sink();

            // Parse the initial item.
            using sink_parser = _list_sink<Args...>;
            auto result
                = _impl.template parse<sink_parser>(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);

            // Continue with the rest of the items.
            using continuation = _list_loop<Item, Sep, NextParser, Args...>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            auto sink = context.sink();

            // Parse the initial item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink<Args...>>;
            auto result       = item_parser::parse(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);

            // Continue with the rest of the items.
            using continuation = _list_loop<Item, Sep, NextParser, Args...>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
        }
    };
};
} // namespace lexyd

namespace lexyd
{
/// Creates a list of items without a separator.
template <typename Item>
LEXY_CONSTEVAL auto list(Item)
{
    static_assert(lexy::is_branch<Item>, "list() without a separator requires a branch condition");
    return _lst<Item, void>{};
}

/// Creates a list of items with the specified separator.
template <typename Item, typename Sep>
LEXY_CONSTEVAL auto list(Item, _sep<Sep>)
{
    return _lst<Item, _sep<Sep>>{};
}

/// Creates a list of items with the specified separator that can be trailing.
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
template <typename Terminator, typename Item, typename Sep>
struct _lstt;
template <typename Terminator, typename Item>
struct _lstt<Terminator, Item, void> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            using item_parser = typename lexy::rule_parser<Item, _list_sink<Args...>>;
            auto sink         = context.sink();

            // Parse initial item.
            auto result = item_parser::parse(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);

            // Parse remaining items.
            lexy::branch_matcher<Terminator, Reader> term{};
            while (!term.match(reader))
            {
                result = item_parser::parse(context, reader, LEXY_FWD(args)..., sink);
                if (result.has_error())
                    return LEXY_MOV(result);
            }

            return _list_finish<NextParser, Args...>::parse_branch(term, context, reader,
                                                                   LEXY_FWD(args)..., sink);
        }
    };
};
template <typename Terminator, typename Item, typename Sep>
struct _lstt<Terminator, Item, _sep<Sep>> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            using item_parser = typename lexy::rule_parser<Item, _list_sink<Args...>>;
            using sep_parser  = typename lexy::rule_parser<Sep, _list_sink<Args...>>;
            auto sink         = context.sink();

            // Parse initial item.
            auto result = item_parser::parse(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);

            // Parse remaining items.
            lexy::branch_matcher<Terminator, Reader> term{};
            while (!term.match(reader))
            {
                // Parse separator.
                result = sep_parser::parse(context, reader, LEXY_FWD(args)..., sink);
                if (result.has_error())
                    return LEXY_MOV(result);

                // Parse item.
                result = item_parser::parse(context, reader, LEXY_FWD(args)..., sink);
                if (result.has_error())
                    return LEXY_MOV(result);
            }

            return _list_finish<NextParser, Args...>::parse_branch(term, context, reader,
                                                                   LEXY_FWD(args)..., sink);
        }
    };
};
template <typename Terminator, typename Item, typename Sep>
struct _lstt<Terminator, Item, _tsep<Sep>> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            using item_parser = typename lexy::rule_parser<Item, _list_sink<Args...>>;
            using sep_parser  = typename lexy::rule_parser<Sep, _list_sink<Args...>>;
            auto sink         = context.sink();

            // Parse initial item.
            auto result = item_parser::parse(context, reader, LEXY_FWD(args)..., sink);
            if (result.has_error())
                return LEXY_MOV(result);

            // Parse remaining items.
            lexy::branch_matcher<Terminator, Reader> term{};
            while (!term.match(reader))
            {
                // Parse separator.
                result = sep_parser::parse(context, reader, LEXY_FWD(args)..., sink);
                if (result.has_error())
                    return LEXY_MOV(result);

                // Check for trailing separator.
                if (term.match(reader))
                    break;

                // Parse item.
                result = item_parser::parse(context, reader, LEXY_FWD(args)..., sink);
                if (result.has_error())
                    return LEXY_MOV(result);
            }

            return _list_finish<NextParser, Args...>::parse_branch(term, context, reader,
                                                                   LEXY_FWD(args)..., sink);
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

