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
template <typename Production>
struct production_start
{};
template <typename Production>
struct production_finish
{};
template <typename Production>
struct production_cancel
{};

struct error
{};

struct token
{};
struct list
{};
} // namespace lexy::parse_events

namespace lexyd
{
namespace _ev = lexy::parse_events;
}

//=== parser ===//
namespace lexy
{
template <typename Rule, typename NextParser>
using rule_parser = typename Rule::template parser<NextParser>;

enum class rule_try_parse_result
{
    ok          = true,
    canceled    = false,
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
} // namespace lexy

//=== whitespace ===//
namespace lexyd
{
template <typename Rule>
struct _wsr;
template <>
struct _wsr<void> : rule_base
{
    template <typename NextParser>
    using parser = NextParser;
};
} // namespace lexyd

namespace lexy
{
struct _tag_no_whitespace
{};
struct _tag_whitespace
{};

template <typename Context>
using _ws_rule = std::conditional_t<
    // We need to disable whitespace if the context is already currently parsing whitespace.
    Context::contains(_tag_no_whitespace{}) || Context::contains(_tag_whitespace{}), void,
    lexy::production_whitespace<typename Context::production, typename Context::root_production>>;

template <typename Context, typename NextParser>
using whitespace_parser = rule_parser<lexy::dsl::_wsr<_ws_rule<Context>>, NextParser>;
} // namespace lexy

#endif // LEXY_DSL_BASE_HPP_INCLUDED

