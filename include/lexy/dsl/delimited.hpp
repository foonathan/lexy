// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED

#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/symbol.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
/// The reader ends before the closing delimiter was found.
struct missing_delimiter
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing delimiter";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Char, typename Reader>
struct _del_chars
{
    lexy::token_parser_for<Char, Reader> parser;
    typename Reader::iterator            begin;

    constexpr _del_chars(const Reader& reader) : parser(reader), begin(reader.position()) {}

    constexpr bool try_parse(Reader& reader)
    {
        parser      = lexy::token_parser_for<Char, Reader>(reader);
        auto result = parser.try_parse(reader);
        reader.set_position(parser.end);
        return result;
    }

    template <typename Context, typename Sink>
    constexpr void finish(Context& context, Sink& sink, typename Reader::iterator end)
    {
        if (begin == end)
            return;

        context.on(_ev::token{}, typename Char::token_type{}, begin, end);
        sink(lexy::lexeme<Reader>(begin, end));
    }
};

template <typename Close, typename Char, typename Limit, typename... Escapes>
struct _del : rule_base
{
    template <typename CloseParser, typename Context, typename Reader, typename Sink>
    static constexpr bool _loop(CloseParser& close, Context& context, Reader& reader, Sink& sink)
    {
        auto                     del_begin = reader.position();
        _del_chars<Char, Reader> cur_chars(reader);
        while (!close.try_parse(context.control_block, reader))
        {
            close.cancel(context);

            // Check for missing delimiter.
            if (lexy::branch_parser_for<Limit, Reader> limit{};
                limit.try_parse(context.control_block, reader))
            {
                // We're done, so finish the current characters.
                auto end = reader.position();
                cur_chars.finish(context, sink, end);

                auto err = lexy::error<Reader, lexy::missing_delimiter>(del_begin, end);
                context.on(_ev::error{}, err);
                return false;
            }
            else
            {
                limit.cancel(context);
            }

            // Check for escape sequences.
            if ((Escapes::_try_parse(context, reader, sink, cur_chars) || ...))
                // We had an escape sequence, so do nothing in this iteration.
                continue;

            // Parse the next character.
            if (auto begin = reader.position(); !cur_chars.try_parse(reader))
            {
                // It has failed, so finish the current character sequence and report an error.
                cur_chars.finish(context, sink, begin);
                cur_chars.parser.report_error(context, reader);

                // Recover from it; this is always possible,.
                auto end = reader.position();
                context.on(_ev::recovery_start{}, end);

                if (begin == end)
                {
                    // The character didn't consume anything, so we manually discard one code unit.
                    LEXY_ASSERT(reader.peek() != Reader::encoding::eof(),
                                "EOF should be checked before calling this");
                    reader.bump();
                    end = reader.position();
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end);
                }

                context.on(_ev::recovery_finish{}, end);

                // We start the next character after error recovery.
                cur_chars.begin = end;
            }
        }

        // Finish the currently active character sequence.
        cur_chars.finish(context, sink, reader.position());
        return true;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.value_callback().sink();

            // Parse characters until we have the closing delimiter.
            lexy::branch_parser_for<Close, Reader> close{};
            if (!_loop(close, context, reader, sink))
                return false;

            // We're done, finish the sink and then the closing delimiter.
            if constexpr (std::is_same_v<typename decltype(sink)::return_type, void>)
            {
                LEXY_MOV(sink).finish();
                return close.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                return close.template finish<NextParser>(context, reader, LEXY_FWD(args)...,
                                                         LEXY_MOV(sink).finish());
            }
        }
    };
};

struct _escape_base
{};

template <typename Open, typename Close, typename Limit = lexyd::_eof>
struct _delim_dsl
{
    /// Add tokens that will limit the delimited to detect a missing terminator.
    template <typename... Tokens>
    constexpr auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));
        return _delim_dsl<Open, Close, decltype((Limit{} / ... / Tokens{}))>{};
    }

    //=== rules ===//
    /// Sets the content.
    template <typename Char, typename... Escapes>
    constexpr auto operator()(Char, Escapes...) const
    {
        static_assert(lexy::is_token_rule<Char>);
        static_assert((std::is_base_of_v<_escape_base, Escapes> && ...));
        return no_whitespace(open() >> _del<Close, Char, Limit, Escapes...>{});
    }

    //=== access ===//
    /// Matches the open delimiter.
    constexpr auto open() const
    {
        return Open{};
    }
    /// Matches the closing delimiter.
    constexpr auto close() const
    {
        // Close never has any whitespace.
        return Close{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
constexpr auto delimited(Open, Close)
{
    static_assert(lexy::is_branch_rule<Open> && lexy::is_branch_rule<Close>);
    return _delim_dsl<Open, Close>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
constexpr auto delimited(Delim)
{
    static_assert(lexy::is_branch_rule<Delim>);
    return _delim_dsl<Delim, Delim>{};
}

constexpr auto quoted        = delimited(LEXY_LIT("\""));
constexpr auto triple_quoted = delimited(LEXY_LIT("\"\"\""));

constexpr auto single_quoted = delimited(LEXY_LIT("'"));

constexpr auto backticked        = delimited(LEXY_LIT("`"));
constexpr auto double_backticked = delimited(LEXY_LIT("``"));
constexpr auto triple_backticked = delimited(LEXY_LIT("```"));
} // namespace lexyd

namespace lexy
{
struct invalid_escape_sequence
{
    static LEXY_CONSTEVAL auto name()
    {
        return "invalid escape sequence";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Escape, typename... Branches>
struct _escape : _escape_base
{
    template <typename Context, typename Reader, typename Sink, typename Char>
    static constexpr bool _try_parse(Context& context, Reader& reader, Sink& sink,
                                     _del_chars<Char, Reader>& cur_chars)
    {
        auto begin = reader.position();

        // Check whether we're having the initial escape character.
        lexy::branch_parser_for<Escape, Reader> token{};
        if (!token.try_parse(context.control_block, reader))
            // No need to call `.cancel()`; it's a token.
            return false;

        // We do, so finish current character sequence and consume the escape token.
        cur_chars.finish(context, sink, begin);
        // It's a token, so this can't fail.
        token.template finish<lexy::pattern_parser<>>(context, reader);

        // Try to parse the correct branch.
        auto try_parse_branch = [&](auto branch) {
            lexy::branch_parser_for<decltype(branch), Reader> parser{};
            if (!parser.try_parse(context.control_block, reader))
            {
                parser.cancel(context);
                return false;
            }

            // This might fail, but we don't care:
            // it will definitely consume the escape token, and everything that is a valid prefix.
            // The remaining stuff is then just treated as part of the delimited.
            parser.template finish<lexy::sink_parser>(context, reader, sink);
            return true;
        };
        auto found = (try_parse_branch(Branches{}) || ...);

        if constexpr ((lexy::is_unconditional_branch_rule<Branches> || ...))
        {
            LEXY_ASSERT(found, "there is an unconditional branch");
        }
        else if (!found)
        {
            // We haven't found any branch of the escape sequence.
            auto err = lexy::error<Reader, lexy::invalid_escape_sequence>(begin, reader.position());
            context.on(_ev::error{}, err);
        }

        // Restart the current character sequence after the escape sequence.
        cur_chars.begin = reader.position();
        return true;
    }

    /// Adds a generic escape rule.
    template <typename Branch>
    constexpr auto rule(Branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>);
        return _escape<Escape, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the branch.
    template <typename Branch>
    constexpr auto capture(Branch branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>);
        return this->rule(lexy::dsl::capture(branch));
    }

    /// Adds an escape rule that parses the symbol.
    template <const auto& Table, typename Rule>
    constexpr auto symbol(Rule rule) const
    {
        return this->rule(lexyd::symbol<Table>(rule));
    }
    template <const auto& Table>
    constexpr auto symbol() const
    {
        return this->rule(lexyd::symbol<Table>);
    }
};

/// Creates an escape rule.
/// The token is the initial rule to begin,
/// and then you can add rules that match after it.
template <typename EscapeToken>
constexpr auto escape(EscapeToken)
{
    static_assert(lexy::is_token_rule<EscapeToken>);
    return _escape<EscapeToken>{};
}

constexpr auto backslash_escape = escape(lit_c<'\\'>);
constexpr auto dollar_escape    = escape(lit_c<'$'>);
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

