// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>
#include <lexy/lexeme.hpp>

#ifdef LEXY_IGNORE_DEPRECATED_CONTEXT
#    define LEXY_DEPRECATED_CONTEXT
#else
#    define LEXY_DEPRECATED_CONTEXT                                                                \
        [[deprecated("old context_* has been replaced by `dsl::context_lexeme`")]]
#endif

namespace lexyd
{
template <template <typename Reader> typename Lexeme, typename NextParser, typename... PrevArgs>
struct _cap_cont
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... prev_args,
                             typename Reader::iterator begin, Args&&... args)
    {
        auto end = reader.cur();
        return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                 Lexeme<typename Reader::canonical_reader>(begin, end),
                                 LEXY_FWD(args)...);
    }
};

template <template <typename Reader> typename Lexeme, typename Rule, typename NextParser>
struct _cap_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        using continuation = _cap_cont<Lexeme, NextParser, Args...>;
        return lexy::rule_parser<Rule, continuation>::parse(context, reader, LEXY_FWD(args)...,
                                                            reader.cur());
    }
};

template <typename Reader>
struct _context
{
    lexy::lexeme<Reader> lexeme;

    constexpr _context() noexcept = default;
    constexpr _context(typename Reader::iterator begin, typename Reader::iterator end) noexcept
    : lexeme(begin, end)
    {}

    template <typename U>
    static constexpr bool is = std::is_same_v<std::decay_t<U>, _context>;

    // Returns the last context argument.
    template <typename... Args>
    static constexpr auto get(Args&&... args)
    {
        _context<Reader> result;
        // We're immediately invoking a lambda that sets result if the type matches for each
        // argument.
        auto lambda = [&](auto&& arg) {
            if constexpr (is<decltype(arg)>)
                result = arg;
        };
        (lambda(args), ...);
        return result;
    }
};
} // namespace lexyd

namespace lexyd
{
template <typename Rule>
struct _ctx_push : rule_base
{
    template <typename NextParser>
    using parser = _cap_parser<_context, Rule, NextParser>;
};

/// Pushes whatever the rule captures onto the context stack.
template <typename Rule>
LEXY_DEPRECATED_CONTEXT LEXY_CONSTEVAL auto context_push(Rule)
{
    return _ctx_push<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
struct _ctx_drop : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            template <typename Context, typename Reader, typename Head, typename... Tail>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args, Head&& head,
                                     Tail&&... tail)
            {
                using context_t                = _context<typename Reader::canonical_reader>;
                constexpr auto head_is_context = context_t::template is<Head>;
                constexpr auto tail_is_context = (context_t::template is<Tail> || ...);

                if constexpr (head_is_context && !tail_is_context)
                {
                    // Head is a context argument and it is the last one; remove it and we're done.
                    return NextParser::parse(context, reader, LEXY_FWD(args)..., LEXY_FWD(tail)...);
                }
                else
                {
                    // Either Head is a context, but there is a later one, or Head isn't a context.
                    // In either case, we're keeping Head.
                    static_assert(sizeof...(Tail) > 0, "missing previous context_push()");
                    return _continuation<Args..., Head>::parse(context, reader, LEXY_FWD(args)...,
                                                               LEXY_FWD(head), LEXY_FWD(tail)...);
                }
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Initially no argument is kept.
            return _continuation<>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Removes the last pushed context from the stack.
constexpr auto context_drop = _ctx_drop{};
} // namespace lexyd

namespace lexy
{
/// Error when the previously pushed context does not match the popped one.
struct context_mismatch
{
    static LEXY_CONSTEVAL auto name()
    {
        return "context mismatch";
    }
};
} // namespace lexy

namespace lexyd
{
/// Checks that the pushed and popped context are exactly equal.
struct context_eq
{
    template <typename Reader>
    constexpr bool operator()(lexy::lexeme<Reader> lhs, lexy::lexeme<Reader> rhs) const
    {
        return lexy::_detail::equal_lexemes(lhs, rhs);
    }
};

/// Checks that the pushed and popped contexts have the same length.
struct context_eq_length
{
    template <typename Reader>
    constexpr bool operator()(lexy::lexeme<Reader> lhs, lexy::lexeme<Reader> rhs) const
    {
        return lexy::_detail::range_size(lhs.begin(), lhs.end())
               == lexy::_detail::range_size(rhs.begin(), rhs.end());
    }
};

template <typename Rule, typename Eq, typename Error>
struct _ctx_top
{
    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            // We're ignoring any values created by the rule.
            //
            template <typename Context, typename Reader, typename... RuleArgs>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args,
                                     const _context<typename Reader::canonical_reader>& popped,
                                     RuleArgs&&...)
            {
                auto pushed = _context<typename Reader::canonical_reader>::get(LEXY_FWD(args)...);
                if (Eq{}(pushed.lexeme, popped.lexeme))
                    // We've parsed the same argument that we've pushed, continue.
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                else
                {
                    auto err = lexy::make_error<Reader, Error>(popped.lexeme.begin(),
                                                               popped.lexeme.end());
                    context.error(err);
                    return false;
                }
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // We capture the rule and pass it to the continuation to process.
            return _cap_parser<_context, Rule, _continuation<Args...>>::parse(context, reader,
                                                                              LEXY_FWD(args)...);
        }
    };

    /// Sets the error if the context doesn't match.
    template <typename E>
    LEXY_CONSTEVAL auto error()
    {
        return _ctx_top<Rule, Eq, E>{};
    }
};

/// Captures what the Rule matches and checks that it is equal to the last context pushed onto the
/// stack.
/// The context is kept on the stack.
template <typename Eq = context_eq, typename Rule>
LEXY_DEPRECATED_CONTEXT LEXY_CONSTEVAL auto context_top(Rule)
{
    return _ctx_top<Rule, Eq, lexy::context_mismatch>{};
}

template <typename Rule, typename Eq, typename Error>
struct _ctx_pop : decltype(_ctx_top<Rule, Eq, Error>{} + context_drop)
{
    /// Sets the error if the context doesn't match.
    template <typename E>
    LEXY_CONSTEVAL auto error()
    {
        return _ctx_pop<Rule, Eq, E>{};
    }
};

/// Captures what the Rule matches and checks that it is equal to the last context pushed onto the
/// stack.
/// The context is removed on the stack.
template <typename Eq = context_eq, typename Rule>
LEXY_DEPRECATED_CONTEXT LEXY_CONSTEVAL auto context_pop(Rule)
{
    return _ctx_pop<Rule, Eq, lexy::context_mismatch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_HPP_INCLUDED

