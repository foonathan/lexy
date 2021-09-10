// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_RECOVER_HPP_INCLUDED
#define LEXY_DSL_RECOVER_HPP_INCLUDED

#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/eof.hpp>

namespace lexyd
{
template <typename Token, typename Limit>
struct _find : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            while (true)
            {
                if (lexy::token_parser_for<Token, Reader> token{}; token.try_parse(reader))
                    // We've found it.
                    break;
                else if (lexy::try_match_token(get_limit(), reader))
                    // Haven't found it.
                    return false;
                else
                    // Try again.
                    reader.bump();
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Fail error recovery if limiting token is found first.
    template <typename... Tokens>
    constexpr auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));

        auto l = (get_limit() / ... / tokens);
        return _find<Token, decltype(l)>{};
    }

    static constexpr auto get_limit()
    {
        if constexpr (std::is_void_v<Limit>)
            return eof;
        else
            return Limit{};
    }
};

/// Recovers once it finds one of the given tokens (without consuming them).
template <typename... Tokens>
constexpr auto find(Tokens... tokens)
{
    static_assert(sizeof...(Tokens) > 0);
    static_assert((lexy::is_token_rule<Tokens> && ...));

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
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<decltype((R{} | ...)), Context, Reader> recovery{};

            // Try to match one of the recovery rules.
            while (!recovery.try_parse(context, reader))
            {
                if (lexy::try_match_token(get_limit(), reader))
                    // We've failed to recover as we've reached the limit.
                    return false;
                else
                    // Try again.
                    reader.bump();
            }

            // Finish with the rule that matched.
            return recovery.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Fail error recovery if Token is found before any of R.
    template <typename... Tokens>
    constexpr auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));

        auto l = (get_limit() / ... / tokens);
        return _reco<decltype(l), R...>{};
    }

    static constexpr auto get_limit()
    {
        if constexpr (std::is_void_v<Limit>)
            return eof;
        else
            return Limit{};
    }
};

/// Discards input until one of the branches matches to recover from an error.
template <typename... Branches>
constexpr auto recover(Branches...)
{
    static_assert(sizeof...(Branches) > 0);
    static_assert((lexy::is_branch_rule<Branches> && ...));
    return _reco<void, Branches...>{};
}
} // namespace lexyd

namespace lexyd
{
// Performs the recovery part of a try rule.
// TODO: make this one public?
template <typename Recover, typename NextParser>
struct _try_recovery
{
    struct _continuation
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           bool& recovery_finished, Args&&... args)
        {
            recovery_finished = true;
            context.on(_ev::recovery_finish{}, reader.position());
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        context.on(_ev::recovery_start{}, reader.position());

        auto recovery_finished = false;
        auto result
            = lexy::parser_for<Recover, _continuation>::parse(context, reader, recovery_finished,
                                                              LEXY_FWD(args)...);
        if (!recovery_finished)
            context.on(_ev::recovery_cancel{}, reader.position());
        return result;
    }
};
template <typename NextParser>
struct _try_recovery<void, NextParser> : NextParser
{};

template <typename Rule, typename Recover>
struct _tryr : _copy_base<Rule>
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           bool& continuation_reached, Args&&... args)
        {
            continuation_reached = true;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Context, Reader> rule;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Forward branching behavior.
            return rule.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish the rule and check whether it reached the continuation.
            auto continuation_reached = false;
            auto result
                = rule.template finish<_pc<NextParser>>(context, reader, continuation_reached,
                                                        LEXY_FWD(args)...);
            if (continuation_reached)
                // Whatever happened, it is not our problem as we've reached the continuation.
                return result;

            // We haven't reached the continuation, so need to recover.
            LEXY_ASSERT(!result, "we've failed without reaching the continuation?!");
            return _try_recovery<Recover, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using parser = lexy::parser_for<Rule, _pc<NextParser>>;

            // Parse the rule and check whether it reached the continuation.
            auto continuation_reached = false;
            auto result = parser::parse(context, reader, continuation_reached, LEXY_FWD(args)...);
            if (continuation_reached)
                // Whatever happened, it is not our problem as we've reached the continuation.
                return result;

            // We haven't reached the continuation, so need to recover.
            LEXY_ASSERT(!result, "we've failed without reaching the continuation?!");
            return _try_recovery<Recover, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Parses Rule, if that fails, continues immediately.
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

