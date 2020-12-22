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
template <typename Handler, typename Sink, typename... PrevArgs>
struct _list_handler
{
    Handler& _handler;
    Sink&    _sink;

    using parent_result_type = typename Handler::result_type;
    using result_type        = lexy::result<void, parent_result_type>;

    template <typename SubProduction, typename Reader>
    constexpr auto sub_handler(const Reader& reader)
    {
        return _handler.template sub_handler<SubProduction>(reader);
    }

    constexpr auto list_sink()
    {
        return _handler.sink();
    }

    template <typename Error>
    constexpr auto error(Error&& error) &&
    {
        // We report errors to the normal handler.
        return result_type(lexy::result_error, LEXY_MOV(_handler).error(LEXY_FWD(error)));
    }

    template <typename... Args>
    constexpr auto value(PrevArgs&&..., Args&&... args) &&
    {
        if constexpr (sizeof...(Args) > 0)
            _sink(LEXY_FWD(args)...); // Forward item args to the sink.
        return result_type(lexy::result_value);
    }

    // This is the final parser of a list that produces the value and reverts back to the original
    // handler.
    template <typename NextParser>
    struct parser
    {
        template <typename Reader>
        LEXY_DSL_FUNC auto parse(_list_handler& self, Reader& reader, PrevArgs&&... args) ->
            typename Handler::result_type
        {
            if constexpr (std::is_same_v<typename Sink::return_type, void>)
            {
                LEXY_MOV(self._sink).finish();
                return NextParser::parse(self._handler, reader, LEXY_FWD(args)...);
            }
            else
            {
                return NextParser::parse(self._handler, reader, LEXY_FWD(args)...,
                                         LEXY_MOV(self._sink).finish());
            }
        }
    };
};

// Parse items of the list after the initial item and setup.
template <typename Item, typename Sep>
struct _lstl;
template <typename Item>
struct _lstl<Item, void> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& list_handler, Reader& reader, Args&&... args) ->
            typename Handler::parent_result_type
        {
            while (true)
            {
                lexy::branch_matcher<Item, Reader> branch{};
                if (!branch.match(reader))
                    // No longer match additional items, done with list.
                    break;

                auto result = branch.template parse<lexy::final_parser>(list_handler, reader,
                                                                        LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            return Handler::template parser<NextParser>::parse(list_handler, reader,
                                                               LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep>
struct _lstl<Item, _sep<Sep>> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& list_handler, Reader& reader, Args&&... args) ->
            typename Handler::parent_result_type
        {
            while (true)
            {
                // Check whether we have a separator.
                lexy::branch_matcher<Sep, Reader> sep{};
                if (!sep.match(reader))
                    break;

                // Parse the separator.
                auto result = sep.template parse<lexy::final_parser>(list_handler, reader,
                                                                     LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();

                // Parse item.
                result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                          LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            return Handler::template parser<NextParser>::parse(list_handler, reader,
                                                               LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep>
struct _lstl<Item, _tsep<Sep>> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& list_handler, Reader& reader, Args&&... args) ->
            typename Handler::parent_result_type
        {
            while (true)
            {
                // Check whether we have a separator.
                lexy::branch_matcher<Sep, Reader> sep{};
                if (!sep.match(reader))
                    break;

                // Parse the separator.
                auto result = sep.template parse<lexy::final_parser>(list_handler, reader,
                                                                     LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();

                lexy::branch_matcher<Item, Reader> branch{};
                if (!branch.match(reader))
                    // No longer match additional items, done with list.
                    break;

                result = branch.template parse<lexy::final_parser>(list_handler, reader,
                                                                   LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            return Handler::template parser<NextParser>::parse(list_handler, reader,
                                                               LEXY_FWD(args)...);
        }
    };
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

        template <typename NextParser, typename Handler, typename... Args>
        constexpr auto parse(Handler& handler, Reader& reader, Args&&... args)
        {
            // Obtain sink and create list handler.
            auto                                            sink = handler.list_sink();
            _list_handler<Handler, decltype(sink), Args...> list_handler{handler, sink};

            // Parse the initial item.
            auto result
                = _impl.template parse<lexy::final_parser>(list_handler, reader, LEXY_FWD(args)...);
            if (!result)
                return LEXY_MOV(result).error();

            // Continue with the rest of the items.
            using continuation = typename _lstl<Item, Sep>::template parser<NextParser>;
            return continuation::parse(list_handler, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // Obtain sink and create list handler.
            auto                                            sink = handler.list_sink();
            _list_handler<Handler, decltype(sink), Args...> list_handler{handler, sink};

            // Parse the initial item.
            auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                           LEXY_FWD(args)...);
            if (!result)
                return LEXY_MOV(result).error();

            // Continue with the rest of the items.
            using continuation = typename _lstl<Item, Sep>::template parser<NextParser>;
            return continuation::parse(list_handler, reader, LEXY_FWD(args)...);
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
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            auto                                            sink = handler.list_sink();
            _list_handler<Handler, decltype(sink), Args...> list_handler{handler, sink};

            // Parse initial item.
            auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                           LEXY_FWD(args)...);
            if (!result)
                return LEXY_MOV(result).error();

            // Parse remaining items.
            lexy::branch_matcher<Terminator, Reader> term{};
            while (!term.match(reader))
            {
                auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                               LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            // Get value.
            if constexpr (std::is_same_v<typename decltype(sink)::return_type, void>)
            {
                LEXY_MOV(sink).finish();
                return term.template parse<NextParser>(handler, reader, LEXY_FWD(args)...);
            }
            else
            {
                return term.template parse<NextParser>(handler, reader, LEXY_FWD(args)...,
                                                       LEXY_MOV(sink).finish());
            }
        }
    };
};
template <typename Terminator, typename Item, typename Sep>
struct _lstt<Terminator, Item, _sep<Sep>> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            auto                                            sink = handler.list_sink();
            _list_handler<Handler, decltype(sink), Args...> list_handler{handler, sink};

            // Parse initial item.
            auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                           LEXY_FWD(args)...);
            if (!result)
                return LEXY_MOV(result).error();

            // Parse remaining items.
            lexy::branch_matcher<Terminator, Reader> term{};
            while (!term.match(reader))
            {
                // Parse separator.
                result = Sep::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                         LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();

                // Parse item.
                result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                          LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            // Get value.
            if constexpr (std::is_same_v<typename decltype(sink)::return_type, void>)
            {
                LEXY_MOV(sink).finish();
                return term.template parse<NextParser>(handler, reader, LEXY_FWD(args)...);
            }
            else
            {
                return term.template parse<NextParser>(handler, reader, LEXY_FWD(args)...,
                                                       LEXY_MOV(sink).finish());
            }
        }
    };
};
template <typename Terminator, typename Item, typename Sep>
struct _lstt<Terminator, Item, _tsep<Sep>> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            auto                                            sink = handler.list_sink();
            _list_handler<Handler, decltype(sink), Args...> list_handler{handler, sink};

            // Parse initial item.
            auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                           LEXY_FWD(args)...);
            if (!result)
                return LEXY_MOV(result).error();

            // Parse remaining items.
            lexy::branch_matcher<Terminator, Reader> term{};
            while (!term.match(reader))
            {
                // Parse separator.
                result = Sep::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                         LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();

                // Check for trailing separator.
                if (term.match(reader))
                    break;

                // Parse item.
                result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                          LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            // Get value.
            if constexpr (std::is_same_v<typename decltype(sink)::return_type, void>)
            {
                LEXY_MOV(sink).finish();
                return term.template parse<NextParser>(handler, reader, LEXY_FWD(args)...);
            }
            else
            {
                return term.template parse<NextParser>(handler, reader, LEXY_FWD(args)...,
                                                       LEXY_MOV(sink).finish());
            }
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

