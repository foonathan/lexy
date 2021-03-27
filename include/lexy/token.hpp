// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_TOKEN_HPP_INCLUDED
#define LEXY_TOKEN_HPP_INCLUDED

#include <climits>
#include <cstdint>
#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
enum predefined_token_kind : std::uint_least16_t
{
    unknown_token_kind              = UINT_LEAST16_MAX,
    eof_token_kind                  = UINT_LEAST16_MAX - 1,
    _smallest_predefined_token_kind = eof_token_kind,
};

constexpr const char* _kind_name(predefined_token_kind kind) noexcept
{
    switch (kind)
    {
    case unknown_token_kind:
        return "token";
    case eof_token_kind:
        return "EOF";
    }

    return ""; // unreachable
}
} // namespace lexy

namespace lexy
{
template <typename Token, auto Kind, typename Next>
struct _tk_map
{
    template <typename T>
    static LEXY_CONSTEVAL auto lookup(T)
    {
        if constexpr (std::is_same_v<typename T::token_type, Token>)
            return Kind;
        else
            return Next::lookup(T{});
    }

    template <decltype(Kind) NewKind, typename T>
    LEXY_CONSTEVAL auto map(T) const
    {
        static_assert(lexy::is_token<T>, "cannot map non-token to token kind");
        static_assert(!std::is_same_v<typename T::token_type, Token>, "already inserted");
        return _tk_map<typename T::token_type, NewKind, _tk_map>{};
    }
};

struct _tk_map_empty
{
    template <typename T>
    static LEXY_CONSTEVAL auto lookup(T)
    {
        return unknown_token_kind;
    }

    template <auto TokenKind, typename T>
    LEXY_CONSTEVAL auto map(T) const
    {
        static_assert(lexy::is_token<T>, "cannot map non-token to token kind");
        return _tk_map<typename T::token_type, TokenKind, _tk_map_empty>{};
    }
};

inline constexpr auto token_kind_map = _tk_map_empty{};

/// A mapping of token rule to token kind; specialize for your own kinds.
template <typename TokenKind>
constexpr auto token_kind_map_for = token_kind_map;
} // namespace lexy

namespace lexy
{
template <typename TokenKind>
using _detect_token_kind_name = decltype(token_kind_name(TokenKind{}));

/// What sort of token it is.
template <typename TokenKind = void>
class token_kind
{
    static_assert(std::is_enum_v<TokenKind>, "invalid type for TokenKind");

public:
    //=== constructors ===//
    /// Creates an unknown token kind.
    constexpr token_kind() noexcept : token_kind(unknown_token_kind) {}

    /// Creates a predefined token kind.
    constexpr token_kind(predefined_token_kind value) noexcept : _value(value) {}

    /// Creates the token kind with the specified value.
    constexpr token_kind(TokenKind value) noexcept : _value(static_cast<std::uint_least16_t>(value))
    {
        auto as_int = std::underlying_type_t<TokenKind>(value);
        LEXY_PRECONDITION(0 <= as_int && as_int < _smallest_predefined_token_kind);
    }

    /// Creates the token kind of a token rule.
    template <typename TokenRule, typename = std::enable_if_t<lexy::is_token<TokenRule>>>
    constexpr token_kind(TokenRule) noexcept : token_kind()
    {
        // Look for internal mapping first.
        auto token_rule_kind = TokenRule::token_kind();
        if constexpr (std::is_same_v<decltype(token_rule_kind), TokenKind>)
            // The token has an associated kind.
            *this = token_kind(token_rule_kind);
        else
        {
            // Look for an external mapping.
            auto result = token_kind_map_for<TokenKind>.lookup(TokenRule{});
            *this       = token_kind(result);
        }
    }

    //=== access ===//
    constexpr explicit operator bool() const noexcept
    {
        return _value != unknown_token_kind;
    }

    constexpr bool is_predefined() const noexcept
    {
        return _value >= _smallest_predefined_token_kind;
    }

    constexpr const char* name() const noexcept
    {
        if (is_predefined())
            return _kind_name(static_cast<predefined_token_kind>(_value));
        else if constexpr (lexy::_detail::is_detected<_detect_token_kind_name, TokenKind>)
            return token_kind_name(get());
        else
            // We only have a generic name.
            return "token";
    }

    constexpr TokenKind get() const noexcept
    {
        return static_cast<TokenKind>(_value);
    }

    //=== comparision ===//
    friend constexpr bool operator==(token_kind lhs, token_kind rhs) noexcept
    {
        return lhs._value == rhs._value;
    }
    friend constexpr bool operator!=(token_kind lhs, token_kind rhs) noexcept
    {
        return lhs._value != rhs._value;
    }

    //=== raw access ===//
    static constexpr std::uint_least16_t to_raw(token_kind<TokenKind> kind) noexcept
    {
        return kind._value;
    }
    static constexpr token_kind<TokenKind> from_raw(std::uint_least16_t kind) noexcept
    {
        return token_kind<TokenKind>(kind);
    }

private:
    constexpr explicit token_kind(std::uint_least16_t kind) noexcept : _value(kind) {}

    std::uint_least16_t _value;
};
template <>
class token_kind<void>
{
public:
    /// Creates an unknown token kind.
    constexpr token_kind() noexcept : token_kind(unknown_token_kind) {}

    constexpr token_kind(predefined_token_kind value) noexcept : _value(value) {}

    /// Creates the token kind with the specified value.
    constexpr token_kind(int value) noexcept : _value(static_cast<std::uint_least16_t>(value))
    {
        LEXY_PRECONDITION(0 <= value && value < _smallest_predefined_token_kind);
    }

    /// Creates the token kind of a token rule.
    template <typename TokenRule, typename = std::enable_if_t<lexy::is_token<TokenRule>>>
    constexpr token_kind(TokenRule) noexcept : token_kind()
    {
        auto token_rule_kind = TokenRule::token_kind();
        if constexpr (std::is_integral_v<decltype(token_rule_kind)>)
            // The token has an associated kind.
            *this = token_kind(int(token_rule_kind));
    }

    //=== access ===//
    constexpr explicit operator bool() const noexcept
    {
        return _value != unknown_token_kind;
    }

    constexpr bool is_predefined() const noexcept
    {
        return _value >= _smallest_predefined_token_kind;
    }

    constexpr const char* name() const noexcept
    {
        if (is_predefined())
            return _kind_name(static_cast<predefined_token_kind>(_value));
        else
            // We only have a generic name.
            return "token";
    }

    constexpr int get() const noexcept
    {
        if (is_predefined())
            return -1 - (UINT_LEAST16_MAX - _value);
        else
            return _value;
    }

    //=== comparision ===//
    friend constexpr bool operator==(token_kind lhs, token_kind rhs) noexcept
    {
        return lhs._value == rhs._value;
    }
    friend constexpr bool operator!=(token_kind lhs, token_kind rhs) noexcept
    {
        return lhs._value != rhs._value;
    }

    //=== raw access ===//
    static constexpr std::uint_least16_t to_raw(token_kind<void> kind) noexcept
    {
        return kind._value;
    }
    static constexpr token_kind<void> from_raw(std::uint_least16_t kind) noexcept
    {
        return token_kind<void>(kind);
    }

private:
    constexpr explicit token_kind(std::uint_least16_t kind) noexcept : _value(kind) {}

    std::uint_least16_t _value;
};

template <typename TokenKind, typename = std::enable_if_t<std::is_integral_v<TokenKind>>>
token_kind(TokenKind) -> token_kind<void>;
template <typename TokenKind, typename = std::enable_if_t<std::is_enum_v<TokenKind>>>
token_kind(TokenKind) -> token_kind<TokenKind>;
template <typename TokenRule, typename = std::enable_if_t<lexy::is_token<TokenRule>>>
token_kind(TokenRule)
    -> token_kind<std::conditional_t<std::is_enum_v<decltype(TokenRule::token_kind())>,
                                     decltype(TokenRule::token_kind()), void>>;
} // namespace lexy

namespace lexy
{
/// A parsed token, i.e. its kind and its lexeme.
template <typename Reader, typename TokenKind = void>
class token
{
public:
    explicit constexpr token(token_kind<TokenKind> kind, lexy::lexeme<Reader> lex) noexcept
    : _lexeme(lex), _kind(kind)
    {
        LEXY_PRECONDITION(lex.begin() != typename Reader::iterator());
    }
    explicit constexpr token(token_kind<TokenKind> kind, typename Reader::iterator begin,
                             typename Reader::iterator end) noexcept
    : token(kind, lexy::lexeme<Reader>(begin, end))
    {}

    constexpr token_kind<TokenKind> kind() const noexcept
    {
        return _kind;
    }

    constexpr auto name() const noexcept
    {
        return _kind.name();
    }

    constexpr auto position() const noexcept -> typename Reader::iterator
    {
        return _lexeme.begin();
    }

    constexpr auto lexeme() const noexcept
    {
        return _lexeme;
    }

private:
    lexy::lexeme<Reader>  _lexeme;
    token_kind<TokenKind> _kind;
};

template <typename TokenKind, typename Reader>
token(token_kind<TokenKind>, lexy::lexeme<Reader>) -> token<Reader, TokenKind>;
template <typename TokenKind, typename Reader,
          typename = std::enable_if_t<std::is_integral_v<TokenKind>>>
token(TokenKind, lexy::lexeme<Reader>) -> token<Reader, void>;
template <typename TokenKind, typename Reader,
          typename = std::enable_if_t<std::is_enum_v<TokenKind>>>
token(TokenKind, lexy::lexeme<Reader>) -> token<Reader, TokenKind>;
template <typename TokenRule, typename Reader,
          typename = std::enable_if_t<lexy::is_token<TokenRule>>>
token(TokenRule, lexy::lexeme<Reader>)
    -> token<Reader, std::conditional_t<std::is_enum_v<decltype(TokenRule::token_kind())>,
                                        decltype(TokenRule::token_kind()), void>>;

template <typename Input, typename TokenKind = void>
using token_for = token<lexy::input_reader<Input>, TokenKind>;
} // namespace lexy

#endif // LEXY_TOKEN_HPP_INCLUDED

