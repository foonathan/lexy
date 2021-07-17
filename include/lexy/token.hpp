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
#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/grammar.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
enum predefined_token_kind : std::uint_least16_t
{
    unknown_token_kind              = UINT_LEAST16_MAX,
    whitespace_token_kind           = UINT_LEAST16_MAX - 1,
    position_token_kind             = UINT_LEAST16_MAX - 2,
    eof_token_kind                  = UINT_LEAST16_MAX - 3,
    eol_token_kind                  = UINT_LEAST16_MAX - 4,
    identifier_token_kind           = UINT_LEAST16_MAX - 5,
    _smallest_predefined_token_kind = identifier_token_kind,
};

constexpr const char* _kind_name(predefined_token_kind kind) noexcept
{
    switch (kind)
    {
    case unknown_token_kind:
        return "token";
    case whitespace_token_kind:
        return "whitespace";
    case position_token_kind:
        return "position";
    case eof_token_kind:
        return "EOF";
    case eol_token_kind:
        return "eol";
    case identifier_token_kind:
        return "identifier";
    }

    return ""; // unreachable
}
} // namespace lexy

namespace lexy
{
template <typename TokenKind, typename... Tokens>
struct _tk_map
{
    TokenKind _data[sizeof...(Tokens)];

    template <std::size_t... Idx>
    LEXY_CONSTEVAL explicit _tk_map(lexy::_detail::index_sequence<Idx...>, const TokenKind* data,
                                    TokenKind new_kind)
    // Add new kind at the end.
    : _data{data[Idx]..., new_kind}
    {}

    template <TokenKind Kind, typename Token>
    LEXY_CONSTEVAL auto map(Token) const
    {
        static_assert(lexy::is_token_rule<Token>, "cannot map non-token to token kind");
        return _tk_map<TokenKind, Tokens..., Token>(lexy::_detail::index_sequence_for<Tokens...>{},
                                                    _data, Kind);
    }

    template <typename Token>
    LEXY_CONSTEVAL auto lookup(Token) const
    {
        constexpr auto idx = [] {
            // There is an easier way to do it via fold expressions but clang 6 generates a bogus
            // warning about sequence points.
            // As such, we do the less fancy version of looking for the index in an array.
            bool is_same[]
                = {std::is_same_v<typename Token::token_type, typename Tokens::token_type>...};

            for (std::size_t idx = 0; idx != sizeof...(Tokens); ++idx)
                if (is_same[idx])
                    return idx;

            return sizeof...(Tokens);
        }();
        if constexpr (idx == sizeof...(Tokens))
            return unknown_token_kind;
        else
            return _data[idx];
    }
};

struct _tk_map_empty
{
    template <typename Token>
    static LEXY_CONSTEVAL auto lookup(Token)
    {
        return unknown_token_kind;
    }

    template <auto TokenKind, typename Token>
    LEXY_CONSTEVAL auto map(Token) const
    {
        static_assert(lexy::is_token_rule<Token>, "cannot map non-token to token kind");
        return _tk_map<std::decay_t<decltype(TokenKind)>,
                       Token>(lexy::_detail::index_sequence_for<>{}, nullptr, TokenKind);
    }
};

inline constexpr auto token_kind_map = _tk_map_empty{};

/// A mapping of token rule to token kind; specialize for your own kinds.
template <typename TokenKind>
constexpr auto token_kind_map_for = token_kind_map;

// Prevent user-defined specialization for void.
template <>
inline constexpr auto token_kind_map_for<void> = token_kind_map;
} // namespace lexy

namespace lexy
{
template <typename TokenKind>
using _detect_token_kind_name = decltype(token_kind_name(TokenKind{}));

/// What sort of token it is.
template <typename TokenKind = void>
class token_kind
{
    static_assert(std::is_void_v<TokenKind> || std::is_enum_v<TokenKind>,
                  "invalid type for TokenKind");
    using _underlying_type = lexy::_detail::type_or<TokenKind, int>;

public:
    //=== constructors ===//
    /// Creates an unknown token kind.
    constexpr token_kind() noexcept : token_kind(unknown_token_kind) {}

    /// Creates a predefined token kind.
    constexpr token_kind(predefined_token_kind value) noexcept : _value(value) {}

    /// Creates the token kind with the specified value.
    constexpr token_kind(_underlying_type value) noexcept
    : _value(static_cast<std::uint_least16_t>(value))
    {
        LEXY_PRECONDITION(_underlying_type(0) <= value
                          && value < _underlying_type(_smallest_predefined_token_kind));
    }

    /// Creates the token kind of a token rule.
    template <typename TokenRule, typename = std::enable_if_t<lexy::is_token_rule<TokenRule>>>
    constexpr token_kind(TokenRule) noexcept : token_kind()
    {
        // Look for internal mapping first.
        auto token_rule_kind = TokenRule::token_kind();
        if constexpr ((std::is_enum_v<TokenKind> //
                       && std::is_same_v<decltype(token_rule_kind), TokenKind>)
                      || (std::is_void_v<TokenKind> //
                          && std::is_integral_v<decltype(token_rule_kind)>))
        {
            // The token has an associated kind of the same type.
            *this = token_kind(token_rule_kind);
        }
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
            return token_kind_name(get()); // ADL
        else
            // We only have a generic name.
            return "token";
    }

    constexpr _underlying_type get() const noexcept
    {
        return static_cast<_underlying_type>(_value);
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

template <typename TokenKind, typename = std::enable_if_t<std::is_integral_v<TokenKind>>>
token_kind(TokenKind) -> token_kind<void>;
template <typename TokenKind, typename = std::enable_if_t<std::is_enum_v<TokenKind>>>
token_kind(TokenKind) -> token_kind<TokenKind>;
} // namespace lexy

namespace lexy
{
/// A parsed token, i.e. its kind and its lexeme.
template <typename Reader, typename TokenKind = void>
class token
{
public:
    using encoding  = typename Reader::encoding;
    using char_type = typename encoding::char_type;
    using iterator  = typename Reader::iterator;

    explicit constexpr token(token_kind<TokenKind> kind, lexy::lexeme<Reader> lex) noexcept
    : _lexeme(lex), _kind(kind)
    {
        LEXY_PRECONDITION(lex.begin() != iterator());
    }
    explicit constexpr token(token_kind<TokenKind> kind, iterator begin, iterator end) noexcept
    : token(kind, lexy::lexeme<Reader>(begin, end))
    {}

    constexpr token_kind<TokenKind> kind() const noexcept
    {
        return _kind;
    }

    constexpr const char* name() const noexcept
    {
        return _kind.name();
    }

    constexpr iterator position() const noexcept
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

template <typename Input, typename TokenKind = void>
using token_for = token<lexy::input_reader<Input>, TokenKind>;
} // namespace lexy

#endif // LEXY_TOKEN_HPP_INCLUDED

