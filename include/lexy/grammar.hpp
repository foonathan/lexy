// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_GRAMMAR_HPP_INCLUDED
#define LEXY_GRAMMAR_HPP_INCLUDED

#include <cstdint>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/type_name.hpp>
#include <lexy/callback/base.hpp>

//=== rule ===//
// We use a shorthand namespace to decrease symbol size.
namespace lexyd
{
struct rule_base
{};

struct branch_base : rule_base
{};

struct unconditional_branch_base : branch_base
{};

struct _token_base
{};
struct _char_class_base
{};
struct _lset_base
{};
struct _lit_base
{};
struct _sep_base
{};
} // namespace lexyd

namespace lexy
{
namespace dsl = lexyd;

template <typename T>
constexpr bool is_rule = std::is_base_of_v<dsl::rule_base, T>;

template <typename T>
constexpr bool is_branch_rule = std::is_base_of_v<dsl::branch_base, T>;
template <typename T>
constexpr bool is_unconditional_branch_rule = std::is_base_of_v<dsl::unconditional_branch_base, T>;

template <typename T>
constexpr bool is_token_rule = std::is_base_of_v<dsl::_token_base, T>;

template <typename T>
constexpr bool is_char_class_rule = std::is_base_of_v<dsl::_char_class_base, T>;
template <typename T>
constexpr bool is_literal_rule = std::is_base_of_v<dsl::_lit_base, T>;
template <typename T>
constexpr bool is_literal_set_rule = std::is_base_of_v<dsl::_lset_base, T>;

template <typename T>
constexpr auto is_separator = std::is_base_of_v<lexy::dsl::_sep_base, T>;
} // namespace lexy

//=== predefined_token_kind ===//
namespace lexy
{
enum predefined_token_kind : std::uint_least16_t
{
    unknown_token_kind = UINT_LEAST16_MAX,

    error_token_kind      = UINT_LEAST16_MAX - 1,
    whitespace_token_kind = UINT_LEAST16_MAX - 2,
    any_token_kind        = UINT_LEAST16_MAX - 3,

    literal_token_kind  = UINT_LEAST16_MAX - 4,
    position_token_kind = UINT_LEAST16_MAX - 5,
    eof_token_kind      = UINT_LEAST16_MAX - 6,

    identifier_token_kind = UINT_LEAST16_MAX - 7,
    digits_token_kind     = UINT_LEAST16_MAX - 8,

    _smallest_predefined_token_kind = digits_token_kind,
};

constexpr const char* _kind_name(predefined_token_kind kind) noexcept
{
    switch (kind)
    {
    case unknown_token_kind:
        return "token";

    case error_token_kind:
        return "error token";
    case whitespace_token_kind:
        return "whitespace";
    case any_token_kind:
        return "any";

    case literal_token_kind:
        return "literal";
    case position_token_kind:
        return "position";
    case eof_token_kind:
        return "EOF";

    case identifier_token_kind:
        return "identifier";
    case digits_token_kind:
        return "digits";
    }

    return ""; // unreachable
}

/// Specialize to define the token kind of a rule.
template <typename TokenRule>
constexpr auto token_kind_of = lexy::unknown_token_kind;

template <typename TokenRule>
constexpr auto token_kind_of<const TokenRule> = token_kind_of<TokenRule>;
} // namespace lexy

//=== production ===//
namespace lexy
{
template <typename Production>
using production_rule = LEXY_DECAY_DECLTYPE(Production::rule);

template <typename Production>
constexpr bool is_production = _detail::is_detected<production_rule, Production>;

/// Base class to indicate that this production is conceptually a token.
/// This inhibits whitespace skipping inside the production.
///
/// When generating a parse tree, it will also merge tokens of the same kind into the same node.
struct token_production
{};

template <typename Production>
constexpr bool is_token_production = std::is_base_of_v<token_production, Production>;

/// Base class to indicate that this production is transparent for the parse tree generation.
/// It will not create a node in the tree, all children will be added to the its parent.
/// If parse tree generation is not used, it has no effect.
struct transparent_production
{};

template <typename Production>
constexpr bool is_transparent_production = std::is_base_of_v<transparent_production, Production>;

template <typename Production>
LEXY_CONSTEVAL const char* production_name()
{
    return _detail::type_name<Production>();
}

template <typename Production>
using _detect_max_recursion_depth = decltype(Production::max_recursion_depth);

template <typename EntryProduction>
LEXY_CONSTEVAL std::size_t max_recursion_depth()
{
    if constexpr (_detail::is_detected<_detect_max_recursion_depth, EntryProduction>)
        return EntryProduction::max_recursion_depth;
    else
        return 1024; // Arbitrary power of two.
}
} // namespace lexy

namespace lexy
{
template <typename Production>
using _detect_whitespace = decltype(Production::whitespace);

template <typename Production, typename Root>
auto _production_whitespace()
{
    if constexpr (is_token_production<Production>)
    {
        // Token productions don't have whitespace.
        return;
    }
    else if constexpr (lexy::_detail::is_detected<_detect_whitespace, Production>)
    {
        // We have whitespace defined in the production.
        return Production::whitespace;
    }
    else if constexpr (lexy::_detail::is_detected<_detect_whitespace, Root>)
    {
        // We have whitespace defined in the root.
        return Root::whitespace;
    }

    // If we didn't have any cases, function returns void.
}
template <typename Production, typename Root>
using production_whitespace = decltype(_production_whitespace<Production, Root>());
} // namespace lexy

namespace lexy
{
template <typename To, typename... Args>
inline constexpr auto _is_convertible = false;
template <typename To, typename Arg>
inline constexpr auto _is_convertible<To, Arg> = std::is_convertible_v<Arg, To>;
template <>
inline constexpr auto _is_convertible<void> = true;

template <typename Production, typename ParseState = void>
class production_value_callback
{
    using _type = LEXY_DECAY_DECLTYPE(Production::value);

    static auto _return_type_callback()
    {
        if constexpr (lexy::is_callback<_type>)
            return Production::value;
        else if constexpr (lexy::is_sink<_type, ParseState>)
            return Production::value.sink(LEXY_DECLVAL(const ParseState&));
        else
            return Production::value.sink();
    }

public:
    constexpr explicit production_value_callback(const ParseState* state) : _state(state) {}

    template <typename State = ParseState, typename = std::enable_if_t<std::is_void_v<State>>>
    constexpr production_value_callback() : _state(nullptr)
    {}

    template <typename State = ParseState,
              typename       = std::enable_if_t<std::is_same_v<State, ParseState>>>
    constexpr explicit production_value_callback(const State& state) : _state(&state)
    {}

    using return_type = typename decltype(_return_type_callback())::return_type;

    constexpr auto sink() const
    {
        if constexpr (lexy::is_sink<_type, ParseState>)
            return Production::value.sink(*_state);
        else
            return Production::value.sink();
    }

    template <typename... Args>
    constexpr return_type operator()(Args&&... args)
    {
        if constexpr (lexy::is_callback_for<_type, Args&&...>)
        {
            if constexpr (lexy::is_callback_state<_type, ParseState>)
                return Production::value[*_state](LEXY_FWD(args)...);
            else
                return Production::value(LEXY_FWD(args)...);
        }
        else if constexpr (lexy::is_sink<_type> || lexy::is_sink<_type, ParseState>)
        {
            if constexpr (_is_convertible<return_type, Args&&...>)
            {
                // We don't have a matching callback, but it is a single argument that has
                // the correct type already, or we return void and have no arguments.
                // Assume it came from the list sink and return the value without invoking a
                // callback.
                return (LEXY_FWD(args), ...);
            }
            else
            {
                static_assert(_detail::error<Production, Args...>,
                              "missing value callback overload for production; only have sink");
            }
        }
        else
        {
            static_assert(_detail::error<Production, Args...>,
                          "missing value callback overload for production");
        }
    }

private:
    const ParseState* _state;
};
} // namespace lexy

#endif // LEXY_GRAMMAR_HPP_INCLUDED

