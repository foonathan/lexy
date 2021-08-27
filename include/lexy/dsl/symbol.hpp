// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SYMBOL_HPP_INCLUDED
#define LEXY_DSL_SYMBOL_HPP_INCLUDED

#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/engine/trie.hpp>
#include <lexy/engine/while.hpp>
#include <lexy/error.hpp>
#include <lexy/lexeme.hpp>
#include <lexy/token.hpp>

namespace lexy
{
#define LEXY_SYMBOL(Str) LEXY_NTTP_STRING(Str)

template <typename T, typename... Strings>
class _symbol_table
{
    static auto _char_type()
    {
        if constexpr (sizeof...(Strings) == 0)
            return;
        else
            return std::common_type_t<typename Strings::char_type...>{};
    }

public:
    using char_type   = decltype(_char_type());
    using key_type    = char_type;
    using mapped_type = T;

    struct value_type
    {
        const char_type*   symbol;
        const mapped_type& value;
    };

    //=== modifiers ===//
    LEXY_CONSTEVAL _symbol_table() : _data{} {}

    template <typename SymbolString, typename... Args>
    LEXY_CONSTEVAL auto map(Args&&... args) const
    {
        using next_table = _symbol_table<T, Strings..., SymbolString>;
        if constexpr (empty())
            return next_table(lexy::_detail::make_index_sequence<0>{}, nullptr, LEXY_FWD(args)...);
        else
            return next_table(lexy::_detail::make_index_sequence<size()>{}, _data,
                              LEXY_FWD(args)...);
    }

#if LEXY_HAS_NTTP
    template <lexy::_detail::string_literal SymbolString, typename... Args>
    LEXY_CONSTEVAL auto map(Args&&... args) const
    {
        return map<lexy::_detail::type_string<SymbolString>>(LEXY_FWD(args)...);
    }
#else
#    if (defined(__clang__) && __clang_major__ <= 7)                                               \
        || (defined(__clang__) && defined(__apple_build_version__) && __clang_major__ <= 10)
    template <char C, typename... Args> // Sorry, compiler bug.
#    else
    template <auto C, typename... Args>
#    endif
    LEXY_CONSTEVAL auto map(Args&&... args) const
    {
        return map<lexy::_detail::type_char<C>>(LEXY_FWD(args)...);
    }
#endif

    //=== access ===//
    static constexpr bool empty() noexcept
    {
        return size() == 0;
    }

    static constexpr std::size_t size() noexcept
    {
        return sizeof...(Strings);
    }

    class iterator
    : public lexy::_detail::bidirectional_iterator_base<iterator, value_type, value_type, void>
    {
    public:
        constexpr iterator() noexcept : _table(nullptr), _idx(0) {}

        constexpr value_type deref() const noexcept
        {
            if constexpr (empty())
            {
                LEXY_PRECONDITION(false);
                return value_type{"", LEXY_DECLVAL(T)};
            }
            else
            {
                LEXY_PRECONDITION(_table);
                constexpr const char_type* strings[] = {Strings::get().c_str()...};
                return value_type{strings[_idx], _table->_data[_idx]};
            }
        }

        constexpr void increment() noexcept
        {
            LEXY_PRECONDITION(_idx != sizeof...(Strings));
            ++_idx;
        }
        constexpr void decrement() noexcept
        {
            LEXY_PRECONDITION(_idx != 0);
            --_idx;
        }

        constexpr bool equal(iterator rhs) const noexcept
        {
            LEXY_PRECONDITION(_table == rhs._table);
            return _idx == rhs._idx;
        }

    private:
        constexpr iterator(const _symbol_table* table, std::size_t idx) noexcept
        : _table(table), _idx(idx)
        {}

        const _symbol_table* _table;
        std::size_t          _idx;

        friend _symbol_table;
    };

    constexpr iterator begin() const noexcept
    {
        return iterator(this, 0);
    }
    constexpr iterator end() const noexcept
    {
        return iterator(this, size());
    }

    struct key_index
    {
        std::size_t _value;

        constexpr key_index() noexcept : _value(std::size_t(-1)) {}
        constexpr explicit key_index(std::size_t idx) noexcept : _value(idx)
        {
            LEXY_PRECONDITION(_value < size());
        }

        constexpr explicit operator bool() const noexcept
        {
            return _value < size();
        }

        friend constexpr bool operator==(key_index lhs, key_index rhs) noexcept
        {
            return lhs._value == rhs._value;
        }
        friend constexpr bool operator!=(key_index lhs, key_index rhs) noexcept
        {
            return lhs._value != rhs._value;
        }
    };

    template <typename Reader>
    constexpr key_index try_parse(Reader& reader) const
    {
        static_assert(!empty(), "symbol table must not be empty");
        using engine = lexy::engine_trie<_lazy::trie>;

        typename engine::error_code ec{};
        auto                        idx = engine::parse(ec, reader);
        if (ec == typename engine::error_code())
            return key_index(idx);
        else
            return key_index();
    }

    constexpr const T& operator[](key_index idx) const noexcept
    {
        LEXY_PRECONDITION(idx);
        return _data[idx._value];
    }

private:
    struct _lazy
    {
        static constexpr auto trie = lexy::trie<char_type, Strings...>;
    };

    template <std::size_t... Idx, typename... Args>
    constexpr explicit _symbol_table(lexy::_detail::index_sequence<Idx...>, const T* data,
                                     Args&&... args)
    // New data is appended at the end.
    : _data{data[Idx]..., T(LEXY_FWD(args)...)}
    {}

    std::conditional_t<empty(), char, T> _data[empty() ? 1 : size()];

    template <typename, typename...>
    friend class _symbol_table;
};

template <typename T>
constexpr auto symbol_table = _symbol_table<T>{};
} // namespace lexy

namespace lexy
{
struct unknown_symbol
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unknown symbol";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Leading, typename Trailing>
struct _idp;
template <typename Leading, typename Trailing, typename... Reserved>
struct _id;

template <const auto& Table, typename Token, typename Tag>
struct _sym : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Reader save,
                                         Args&&... args) -> lexy::rule_try_parse_result
            {
                auto rule_content = lexy::partial_reader(save, reader.position());

                // We now re-parse what the rule has consumed.
                auto idx = Table.try_parse(rule_content);
                if (!idx || rule_content.peek() != Reader::encoding::eof())
                {
                    // Unknown symbol; backtrack.
                    context.on(_ev::backtracked{}, save.position(), reader.position());
                    reader = LEXY_MOV(save);
                    return lexy::rule_try_parse_result::backtracked;
                }

                // Succesfully parsed a symbol, produce value and continue.
                return static_cast<lexy::rule_try_parse_result>(
                    NextParser::parse(context, reader, LEXY_FWD(args)..., Table[idx]));
            }

            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Reader save, Args&&... args)
            {
                auto begin  = save.position();
                auto end    = reader.position();
                auto result = try_parse(context, reader, save, LEXY_FWD(args)...);
                if (result == lexy::rule_try_parse_result::backtracked)
                {
                    // Handle the error.
                    using tag = lexy::_detail::type_or<Tag, lexy::unknown_symbol>;
                    auto err  = lexy::error<Reader, tag>(begin, end);
                    context.on(_ev::error{}, err);
                    return false;
                }
                else
                {
                    // Propagate result of the NextParser.
                    return static_cast<bool>(result);
                }
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto save = reader;

            // We can safely discard; token does not produce any values.
            using token_parser = lexy::rule_parser<Token, lexy::discard_parser<Context>>;
            auto result        = token_parser::try_parse(context, reader);
            if (result != lexy::rule_try_parse_result::ok)
                return result;

            // Continue parsing with our special continuation.
            return _continuation::try_parse(context, reader, save, LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the token with our special continuation and remember the current reader.
            return lexy::rule_parser<Token, _continuation>::parse(context, reader, Reader(reader),
                                                                  LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    template <typename ErrorTag>
    static constexpr _sym<Table, Token, ErrorTag> error = {};
};

// Optimization for identifiers: instead of parsing an entire identifier (which requires checking
// every character against the char class), parse a symbol and check whether the next character
// would continue the identifier. This is the same optimization that is done for keywords.
template <const auto& Table, typename L, typename T, typename Tag>
struct _sym<Table, _idp<L, T>, Tag> : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using trailing_engine = typename T::token_engine;

            // Try to parse the symbol.
            auto save = reader;
            auto idx  = Table.try_parse(reader);
            // We need a symbol and it must not be the prefix of an identifier.
            if (!idx || lexy::engine_peek<trailing_engine>(reader))
            {
                // We didn't have a symbol, so backtrack.
                context.on(_ev::backtracked{}, save.position(), reader.position());
                reader = LEXY_MOV(save);
                return lexy::rule_try_parse_result::backtracked;
            }

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.on(_ev::token{}, _idp<L, T>::token_kind(), save.position(), reader.position());
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return static_cast<lexy::rule_try_parse_result>(
                continuation::parse(context, reader, LEXY_FWD(args)..., Table[idx]));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using trailing_engine = typename T::token_engine;

            auto begin = reader.position();
            auto idx   = Table.try_parse(reader);
            // We need a symbol and it must not be the prefix of an identifier.
            if (!idx || lexy::engine_peek<trailing_engine>(reader))
            {
                // We didn't have a symbol.
                // But before we can report the error, we need to parse an identifier.
                // Otherwise, we don't call `context.on(_ev::token{}, )` or have the same end as the
                // non-optimized symbol parser.

                if (begin == reader.position())
                {
                    // We need to parse the entire identifier from scratch.
                    // The identifier pattern does not produce a value, so we can safely discard.
                    using id_parser = lexy::rule_parser<_idp<L, T>, lexy::discard_parser<Context>>;
                    if (!id_parser::parse(context, reader))
                        // Didn't have an identifier, so different error.
                        return false;
                }
                else
                {
                    // We're having a prefix of a valid identifier.
                    // As an additional optimization, just need to parse the remaining characters.
                    lexy::engine_while<trailing_engine>::match(reader);
                    context.on(_ev::token{}, _idp<L, T>::token_kind(), begin, reader.position());
                }
                auto end = reader.position();

                // Now we can report the erorr.
                using tag = lexy::_detail::type_or<Tag, lexy::unknown_symbol>;
                auto err  = lexy::error<Reader, tag>(begin, end);
                context.on(_ev::error{}, err);
                return false;
            }
            auto end = reader.position();

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.on(_ev::token{}, _idp<L, T>::token_kind(), begin, end);
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., Table[idx]);
        }
    };

    //=== dsl ===//
    template <typename ErrorTag>
    static constexpr _sym<Table, _idp<L, T>, ErrorTag> error = {};
};

template <const auto& Table, typename Tag>
struct _sym<Table, void, Tag> : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            // Try to parse the symbol.
            auto save = reader;
            auto idx  = Table.try_parse(reader);
            if (!idx)
            {
                // We didn't have a symbol, so backtrack.
                context.on(_ev::backtracked{}, save.position(), reader.position());
                reader = LEXY_MOV(save);
                return lexy::rule_try_parse_result::backtracked;
            }

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.on(_ev::token{}, lexy::identifier_token_kind, save.position(),
                       reader.position());
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return static_cast<lexy::rule_try_parse_result>(
                continuation::parse(context, reader, LEXY_FWD(args)..., Table[idx]));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            auto idx   = Table.try_parse(reader);
            if (!idx)
            {
                // We didn't have a symbol.
                using tag = lexy::_detail::type_or<Tag, lexy::unknown_symbol>;
                auto err  = lexy::error<Reader, tag>(begin);
                context.on(_ev::error{}, err);
                return false;
            }
            auto end = reader.position();

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.on(_ev::token{}, lexy::identifier_token_kind, begin, end);
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., Table[idx]);
        }
    };

    //=== dsl ===//
    template <typename ErrorTag>
    static constexpr _sym<Table, void, ErrorTag> error = {};
};

template <const auto& Table>
struct _sym_dsl : _sym<Table, void, void>
{
    template <typename Token>
    constexpr auto operator()(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return _sym<Table, Token, void>{};
    }
    template <typename L, typename T, typename... R>
    constexpr auto operator()(_id<L, T, R...> id) const
    {
        static_assert(sizeof...(R) == 0,
                      "symbol() must not be used in the presence of reserved identifiers");
        return _sym<Table, decltype(id.pattern()), void>{};
    }
};

/// Parses optional rule, then matches the resulting lexeme against the symbol table.
template <const auto& Table>
constexpr auto symbol = _sym_dsl<Table>{};
} // namespace lexyd

#endif // LEXY_DSL_SYMBOL_HPP_INCLUDED

