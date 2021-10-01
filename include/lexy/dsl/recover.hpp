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
// Indicates that this rule already generates the recovery events.
struct _recovery_base : rule_base
{};

// Parses the rule but generates the appropriate recovery_start/finish/cancel events.
template <typename Rule>
struct _recovery_wrapper : _recovery_base
{
    template <typename NextParser>
    struct p
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
                = lexy::parser_for<Rule, _continuation>::parse(context, reader, recovery_finished,
                                                               LEXY_FWD(args)...);
            if (!recovery_finished)
                context.on(_ev::recovery_cancel{}, reader.position());
            return result;
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Token, typename Limit>
struct _find : _recovery_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::recovery_start{}, begin);

            while (true)
            {
                if (lexy::token_parser_for<Token, Reader> token(reader); token.try_parse(reader))
                {
                    // We've found it.
                    break;
                }
                else if (lexy::token_parser_for<decltype(get_limit()), Reader> limit(reader);
                         limit.try_parse(reader))
                {
                    // Haven't found it, recovery fails.
                    auto end = reader.position();
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end);
                    context.on(_ev::recovery_cancel{}, end);
                    return false;
                }
                else
                {
                    // Try again.
                    reader.bump();
                }
            }

            auto end = reader.position();
            context.on(_ev::token{}, lexy::error_token_kind, begin, end);
            context.on(_ev::recovery_finish{}, end);

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
struct _reco : _recovery_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::recovery_start{}, begin);

            // Try to match one of the recovery rules.
            lexy::branch_parser_for<decltype((R{} | ...)), Context, Reader> recovery{};
            while (!recovery.try_parse(context, reader))
            {
<<<<<<< HEAD
                if (lexy::try_match_token(get_limit(), reader))
=======
                if (lexy::token_parser_for<decltype(get_limit()), Reader> limit(reader);
                    limit.try_parse(reader))
>>>>>>> 0f1d4fc2 (tmp: token interface)
                {
                    // We've failed to recover as we've reached the limit.
                    auto end = reader.position();
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end);
                    context.on(_ev::recovery_cancel{}, end);
                    return false;
                }
                else
                {
                    // Try again.
                    reader.bump();
                }
            }

            auto end = reader.position();
            context.on(_ev::token{}, lexy::error_token_kind, begin, end);
            context.on(_ev::recovery_finish{}, end);

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

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool recover(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_void_v<Recover>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (std::is_base_of_v<_recovery_base, Recover>)
                return lexy::parser_for<Recover, NextParser>::parse(context, reader,
                                                                    LEXY_FWD(args)...);
            else
                return lexy::parser_for<_recovery_wrapper<Recover>,
                                        NextParser>::parse(context, reader, LEXY_FWD(args)...);
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
            return _pc<NextParser>::recover(context, reader, LEXY_FWD(args)...);
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
            return _pc<NextParser>::recover(context, reader, LEXY_FWD(args)...);
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

