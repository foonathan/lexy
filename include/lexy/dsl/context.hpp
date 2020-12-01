// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/capture.hpp>

namespace lexyd
{
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
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    using parser = _cap_parser<_context, Rule, NextParser>;
};

/// Pushes whatever the rule captures onto the context stack.
template <typename Rule>
LEXY_CONSTEVAL auto context_push(Rule)
{
    return _ctx_push<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
struct _ctx_drop : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            template <typename Handler, typename Reader, typename Head, typename... Tail>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args, Head&& head,
                                     Tail&&... tail) -> typename Handler::result_type
            {
                using context_t                = _context<typename Reader::canonical_reader>;
                constexpr auto head_is_context = context_t::template is<Head>;
                constexpr auto tail_is_context = (context_t::template is<Tail> || ...);

                if constexpr (head_is_context && !tail_is_context)
                {
                    // Head is a context argument and it is the last one; remove it and we're done.
                    return NextParser::parse(handler, reader, LEXY_FWD(args)..., LEXY_FWD(tail)...);
                }
                else
                {
                    // Either Head is a context, but there is a later one, or Head isn't a context.
                    // In either case, we're keeping Head.
                    static_assert(sizeof...(Tail) > 0, "missing previous context_push()");
                    return _continuation<Args..., Head>::parse(handler, reader, LEXY_FWD(args)...,
                                                               LEXY_FWD(head), LEXY_FWD(tail)...);
                }
            }
        };

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // Initially no argument is kept.
            return _continuation<>::parse(handler, reader, LEXY_FWD(args)...);
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
        using iterator = typename Reader::iterator;
        if constexpr (std::is_pointer_v<iterator>)
            if (lhs.size() != rhs.size())
                return false;

        auto lhs_cur = lhs.begin();
        auto rhs_cur = rhs.begin();
        while (lhs_cur != lhs.end() && rhs_cur != rhs.end())
        {
            if (*lhs_cur != *rhs_cur)
                return false;
            ++lhs_cur;
            ++rhs_cur;
        }
        return lhs_cur == lhs.end() && rhs_cur == rhs.end();
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
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            // We're ignoring any values created by the rule.
            //
            template <typename Handler, typename Reader, typename... RuleArgs>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args,
                                     const _context<typename Reader::canonical_reader>& popped,
                                     RuleArgs&&...) -> typename Handler::result_type
            {
                auto pushed = _context<typename Reader::canonical_reader>::get(LEXY_FWD(args)...);
                if (Eq{}(pushed.lexeme, popped.lexeme))
                    // We've parsed the same argument that we've pushed, continue.
                    return NextParser::parse(handler, reader, LEXY_FWD(args)...);
                else
                {
                    auto e = lexy::make_error<Reader, Error>(popped.lexeme.begin(),
                                                             popped.lexeme.end());
                    return LEXY_MOV(handler).error(reader, e);
                }
            }
        };

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // We capture the rule and pass it to the continuation to process.
            return _cap_parser<_context, Rule, _continuation<Args...>>::parse(handler, reader,
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
LEXY_CONSTEVAL auto context_top(Rule)
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
LEXY_CONSTEVAL auto context_pop(Rule)
{
    return _ctx_pop<Rule, Eq, lexy::context_mismatch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_HPP_INCLUDED
