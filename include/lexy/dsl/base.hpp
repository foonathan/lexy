// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/engine/base.hpp>
#include <lexy/grammar.hpp>
#include <lexy/input/base.hpp>

#define LEXY_DSL_FUNC LEXY_FORCE_INLINE static constexpr

#ifdef LEXY_IGNORE_DEPRECATED_ERROR
#    define LEXY_DEPRECATED_ERROR(msg)
#else
#    define LEXY_DEPRECATED_ERROR(msg) [[deprecated(msg)]]
#endif

//=== parse_events ===//
namespace lexy::parse_events
{
struct _production_event
{};

/// Start of the given production.
/// Arguments: position
/// Returns: new marker
template <typename Production>
struct production_start : _production_event
{};
/// End of the given production.
/// Arguments: position, values
/// Returns: value produced  by production.
template <typename Production>
struct production_finish : _production_event
{};
/// Production is canceled.
/// Arguments: position
template <typename Production>
struct production_cancel : _production_event
{};

/// A parse error occurrs.
/// Arguments: error object
struct error
{};

/// A token was consumed.
/// Arguments: kind, begin, end
struct token
{};
/// Beginning of a list.
/// Arguments: position
/// Returns sink.
struct list
{};

/// The input backtracked from end to begin.
/// Only meaningful for begin != end.
/// Arguments: begin, end
struct backtracked
{};
} // namespace lexy::parse_events

namespace lexyd
{
namespace _ev = lexy::parse_events;
}

//=== parse_context_var ===//
namespace lexy::_detail
{
template <typename Parent, typename Id, typename T>
class parse_context_var
{
public:
    constexpr explicit parse_context_var(Parent& parent, Id, T&& value)
    : _parent(&parent), _value(LEXY_MOV(value))
    {}

    //=== parse context ===//
    using handler         = typename Parent::handler;
    using production      = typename Parent::production;
    using root_production = typename Parent::root_production;

    constexpr auto& production_context()
    {
        return _parent->production_context();
    }

    template <typename Event, typename... Args>
    constexpr auto on(Event ev, Args&&... args)
    {
        return production_context().on(ev, LEXY_FWD(args)...);
    }

    //=== context variables ===//
    template <typename Id2>
    static LEXY_CONSTEVAL bool contains([[maybe_unused]] Id2 id)
    {
        if constexpr (std::is_same_v<Id, Id2>)
            return true;
        else
            return Parent::contains(id);
    }

    template <typename Id2>
    constexpr auto& get([[maybe_unused]] Id2 id)
    {
        if constexpr (std::is_same_v<Id2, Id>)
            return _value;
        else
            return _parent->get(id);
    }

private:
    Parent* _parent;
    T       _value;
};

} // namespace lexy::_detail

//=== parser ===//
namespace lexy
{
template <typename Rule, typename NextParser>
using rule_parser = typename Rule::template parser<NextParser>;

enum class rule_try_parse_result
{
    ok          = int(true),
    canceled    = int(false),
    backtracked = 2,
};

/// A final parser that drops all arguments; creating an empty result.
template <typename Context>
struct discard_parser
{
    template <typename NewContext, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(NewContext&, Reader&, Args&&...)
    {
        static_assert(sizeof...(Args) == 0, "discarded rule must not produce any values");
        static_assert(std::is_same_v<Context, NewContext>,
                      "discarded rule cannot add state to the context");
        return true;
    }
};

// Same as the other overload, but raises the event.
template <typename Matcher, typename Context, typename Reader>
constexpr bool engine_peek(Context& context, Reader reader)
{
    auto begin = reader.cur();
    auto ec    = Matcher::match(reader);
    auto end   = reader.cur();

    context.on(parse_events::backtracked{}, begin, end);
    return ec == typename Matcher::error_code{};
}
} // namespace lexy

//=== whitespace ===//
namespace lexy::_detail
{
template <typename NextParser>
struct automatic_ws_parser;
}

namespace lexy
{
template <typename Context, typename NextParser,
          typename = lexy::production_whitespace<typename Context::production,
                                                 typename Context::root_production>>
struct whitespace_parser : _detail::automatic_ws_parser<NextParser>
{};
// If we know the whitespace rule is void, go to NextParser immediately.
// This is both an optimization and also doesn't require inclusion of whitespace.hpp.
template <typename Context, typename NextParser>
struct whitespace_parser<Context, NextParser, void> : NextParser
{};
} // namespace lexy

#endif // LEXY_DSL_BASE_HPP_INCLUDED

