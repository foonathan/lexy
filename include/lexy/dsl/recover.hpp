// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_RECOVER_HPP_INCLUDED
#define LEXY_DSL_RECOVER_HPP_INCLUDED

#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/engine/find.hpp>

namespace lexyd
{
template <typename Token, typename Limit>
struct _find : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using engine = lexy::engine_find_before<typename Token::token_engine,
                                                    typename Limit::token_engine>;
            if (engine::match(reader) != typename engine::error_code())
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Fail error recovery if limiting token is found first.
    template <typename... Tokens>
    constexpr auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));

        auto l = (Limit{} / ... / tokens);
        return _find<Token, decltype(l)>{};
    }

    constexpr auto get_limit() const
    {
        return Limit{};
    }
};
template <typename Token>
struct _find<Token, void> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using engine = lexy::engine_find<typename Token::token_engine>;
            if (engine::match(reader) != typename engine::error_code())
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Fail error recovery if limiting token is found first.
    template <typename... Tokens>
    constexpr auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));

        auto l = (tokens / ...);
        return _find<Token, decltype(l)>{};
    }

    constexpr auto get_limit() const
    {
        return eof;
    }
};

/// Recovers once it finds one of the given tokens (without consuming them).
template <typename... Tokens>
constexpr auto find(Tokens... tokens)
{
    static_assert(sizeof...(Tokens) > 0);
    static_assert((lexy::is_token<Tokens> && ...));

    auto needle = (tokens / ...);
    return _find<decltype(needle), void>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Limit, typename... R>
struct _reco : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            while (true)
            {
                // Try to match the recovery rules.
                using recovery = lexy::rule_parser<_chc<R...>, NextParser>;
                auto result    = recovery::try_parse(context, reader, LEXY_FWD(args)...);
                if (result != lexy::rule_try_parse_result::backtracked)
                    // We've succesfully recovered; return the recovered result.
                    return static_cast<bool>(result);

                // Cancel recovery when we've reached the limit.
                if (lexy::engine_peek<typename Limit::token_engine>(reader))
                    return false;

                // Consume one character and try again.
                reader.bump();
            }

            return false; // unreachable
        }
    };

    //=== dsl ===//
    /// Fail error recovery if Token is found before any of R.
    template <typename... Tokens>
    constexpr auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));

        auto l = (Limit{} / ... / tokens);
        return _reco<decltype(l), R...>{};
    }

    constexpr auto get_limit() const
    {
        return Limit{};
    }
};

/// Discards input until one of the branches matches to recover from an error.
template <typename... Branches>
constexpr auto recover(Branches...)
{
    static_assert(sizeof...(Branches) > 0);
    static_assert((lexy::is_branch<Branches> && ...));
    return _reco<lexyd::_eof, Branches...>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Rule, typename Recover>
struct _tryr : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, bool& failed, Args&&... args)
            {
                failed = false;
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto failed = true;
            // Try parsing with special continuation that sets failed to false if reached.
            auto result = lexy::rule_parser<Rule, _continuation>::try_parse(context, reader, failed,
                                                                            LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
            {
                // Rule backtracked, which is not a failure.
                return result;
            }
            else if (!failed)
            {
                // Rule didn't fail.
                // It could be the case that some later rule has failed, but that's not our problem.
                return result;
            }
            else
            {
                // Rule has failed, recover.
                // Note that we already took the branch, so we no longer backtrack.
                if constexpr (std::is_void_v<Recover>)
                    return NextParser::parse(context, reader, LEXY_FWD(args)...)
                               ? lexy::rule_try_parse_result::ok
                               : lexy::rule_try_parse_result::canceled;
                else
                    return lexy::rule_parser<Recover, NextParser>::parse(context, reader,
                                                                         LEXY_FWD(args)...)
                               ? lexy::rule_try_parse_result::ok
                               : lexy::rule_try_parse_result::canceled;
            }
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto failed = true;
            // Parse with special continuation that sets failed to false if reached.
            auto result = lexy::rule_parser<Rule, _continuation>::parse(context, reader, failed,
                                                                        LEXY_FWD(args)...);
            if (!failed)
            {
                // Rule didn't fail.
                // It could be the case that some later rule has failed, but that's not our problem.
                return result;
            }
            else
            {
                // Rule has failed, recover.
                if constexpr (std::is_void_v<Recover>)
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                else
                    return lexy::rule_parser<Recover, NextParser>::parse(context, reader,
                                                                         LEXY_FWD(args)...);
            }
        }
    };
};

/// Pares Rule, if that fails, continues immediately.
template <typename Rule>
constexpr auto try_(Rule)
{
    return _tryr<Rule, void>{};
}

/// Parses Rule, if that fails, parses recovery rule.
template <typename Rule, typename Recover>
constexpr auto try_(Rule, Recover)
{
    return _tryr<Rule, Recover>{};
}
} // namespace lexyd

#endif // LEXY_DSL_RECOVER_HPP_INCLUDED

