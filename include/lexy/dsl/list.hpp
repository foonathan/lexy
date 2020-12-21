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

    template <typename Reader, typename Error>
    constexpr auto error(const Reader& reader, Error&& error) &&
    {
        // We report errors to the normal handler.
        return result_type(lexy::result_error, LEXY_MOV(_handler).error(reader, LEXY_FWD(error)));
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
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& list_handler, Reader& reader, Args&&... args) ->
            typename Handler::parent_result_type
        {
            while (true)
            {
                using branch = decltype(branch(Item{}));
                if (!branch::condition_matcher::match(reader))
                    // No longer match additional items, done with list.
                    break;

                using item_parser = typename branch::template then_parser<lexy::final_parser>;
                auto result       = item_parser::parse(list_handler, reader, LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            return Handler::template parser<NextParser>::parse(list_handler, reader,
                                                               LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep, bool Capture>
struct _lstl<Item, _sep<Sep, Capture>> : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& list_handler, Reader& reader, Args&&... args) ->
            typename Handler::parent_result_type
        {
            while (true)
            {
                auto begin = reader.cur();
                if (!Sep::matcher::match(reader))
                    // No separator, done with the list.
                    break;

                // Capture separator if necessary.
                if constexpr (Capture)
                    list_handler._sink(lexy::lexeme(reader, begin));
                else
                    (void)begin;

                // Parse item.
                auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                               LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            return Handler::template parser<NextParser>::parse(list_handler, reader,
                                                               LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep, bool Capture>
struct _lstl<Item, _tsep<Sep, Capture>> : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& list_handler, Reader& reader, Args&&... args) ->
            typename Handler::parent_result_type
        {
            while (true)
            {
                auto begin = reader.cur();
                if (!Sep::matcher::match(reader))
                    // No separator, done with the list.
                    break;

                // Capture separator if necessary.
                if constexpr (Capture)
                    list_handler._sink(lexy::lexeme(reader, begin));
                else
                    (void)begin;

                using branch = decltype(branch(Item{}));
                if (!branch::condition_matcher::match(reader))
                    // No longer match additional items, done with list.
                    break;

                using item_parser = typename branch::template then_parser<lexy::final_parser>;
                auto result       = item_parser::parse(list_handler, reader, LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            return Handler::template parser<NextParser>::parse(list_handler, reader,
                                                               LEXY_FWD(args)...);
        }
    };
};

// Parse the initial item of the list after obtaining the handler, sink etc.
// Then continue with the specified loop production.
template <typename Item, typename Cont>
struct _lsth : rule_base
{
    static constexpr auto has_matcher = false;

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
            return Cont::template parser<NextParser>::parse(list_handler, reader,
                                                            LEXY_FWD(args)...);
        }
    };
};

// Parse the entire list, i.e. head followed by loop.
template <typename Item, typename Sep>
struct _lst : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    using parser = typename _lsth<Item, _lstl<Item, Sep>>::template parser<NextParser>;

    template <typename It = Item, typename = std::enable_if_t<lexy::is_branch_rule<It>>>
    friend LEXY_CONSTEVAL auto branch(_lst)
    {
        auto b = branch(Item{});

        using tail = _lstl<Item, Sep>;
        using head = _lsth<decltype(b.then()), tail>;
        return b.condition() >> head{};
    }
};
} // namespace lexyd

namespace lexyd
{
/// Creates a list of items without a separator.
template <typename Item>
LEXY_CONSTEVAL auto list(Item)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a separator requires a branch condition");
    return _lst<Item, void>{};
}

/// Creates a list of items with the specified separator.
template <typename Item, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(Item, _sep<Pattern, Capture>)
{
    return _lst<Item, _sep<Pattern, Capture>>{};
}

/// Creates a list of items with the specified separator that can be trailing.
template <typename Item, typename Pattern, bool Capture>
LEXY_CONSTEVAL auto list(Item, _tsep<Pattern, Capture>)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a trailing separator requires a branch condition");
    return _lst<Item, _tsep<Pattern, Capture>>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Terminator, typename Item, typename Sep>
struct _lstt;
template <typename Terminator, typename Item>
struct _lstt<Terminator, Item, void> : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            using branch = decltype(branch(Terminator()));

            auto                                            sink = handler.list_sink();
            _list_handler<Handler, decltype(sink), Args...> list_handler{handler, sink};

            // Parse initial item.
            auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                           LEXY_FWD(args)...);
            if (!result)
                return LEXY_MOV(result).error();

            while (true)
            {
                if (branch::condition_matcher::match(reader))
                    break;

                auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                               LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            using continuation = typename branch::template then_parser<NextParser>;
            return decltype(list_handler)::template parser<continuation>::parse(list_handler,
                                                                                reader,
                                                                                LEXY_FWD(args)...);
        }
    };
};
template <typename Terminator, typename Item, typename Sep, bool Capture>
struct _lstt<Terminator, Item, _sep<Sep, Capture>> : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            using branch = decltype(branch(Terminator()));

            auto                                            sink = handler.list_sink();
            _list_handler<Handler, decltype(sink), Args...> list_handler{handler, sink};

            // Parse initial item.
            auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                           LEXY_FWD(args)...);
            if (!result)
                return LEXY_MOV(result).error();

            while (true)
            {
                if (branch::condition_matcher::match(reader))
                    break;

                // Parse separator.
                auto begin = reader.cur();
                result     = Sep::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                         LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();

                // Capture separator if necessary.
                if constexpr (Capture)
                    sink(lexy::lexeme(reader, begin));
                else
                    (void)begin;

                // Parse item.
                result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                          LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            using continuation = typename branch::template then_parser<NextParser>;
            return decltype(list_handler)::template parser<continuation>::parse(list_handler,
                                                                                reader,
                                                                                LEXY_FWD(args)...);
        }
    };
};
template <typename Terminator, typename Item, typename Sep, bool Capture>
struct _lstt<Terminator, Item, _tsep<Sep, Capture>> : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            using branch = decltype(branch(Terminator()));

            auto                                            sink = handler.list_sink();
            _list_handler<Handler, decltype(sink), Args...> list_handler{handler, sink};

            // Parse initial item.
            auto result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                           LEXY_FWD(args)...);
            if (!result)
                return LEXY_MOV(result).error();

            while (true)
            {
                if (branch::condition_matcher::match(reader))
                    break;

                // Parse separator.
                auto begin = reader.cur();
                result     = Sep::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                         LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();

                // Capture separator if necessary.
                if constexpr (Capture)
                    sink(lexy::lexeme(reader, begin));
                else
                    (void)begin;

                if (branch::condition_matcher::match(reader))
                    break;

                // Parse item.
                result = Item::template parser<lexy::final_parser>::parse(list_handler, reader,
                                                                          LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            using continuation = typename branch::template then_parser<NextParser>;
            return decltype(list_handler)::template parser<continuation>::parse(list_handler,
                                                                                reader,
                                                                                LEXY_FWD(args)...);
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

