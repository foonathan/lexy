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
        (([&](auto&& arg) {
             if constexpr (is<decltype(arg)>)
                 result = arg;
         }(LEXY_FWD(args))),
         ...);
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
                constexpr auto head_is_context = _context<Reader>::template is<Head>;
                constexpr auto tail_is_context = (_context<Reader>::template is<Tail> || ...);

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

template <typename Rule, typename Eq>
struct _ctx_top
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            template <typename Handler, typename Reader>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args,
                                     const _context<Reader>& popped) ->
                typename Handler::result_type
            {
                auto pushed = _context<Reader>::get(LEXY_FWD(args)...);
                if (Eq{}(pushed.lexeme, popped.lexeme))
                    // We've parsed the same argument that we've pushed, continue.
                    return NextParser::parse(handler, reader, LEXY_FWD(args)...);
                else
                {
                    using error = lexy::error<Reader, lexy::context_mismatch>;
                    return LEXY_MOV(handler).error(reader, error(popped.lexeme.begin(),
                                                                 popped.lexeme.end()));
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
};

/// Captures what the Rule matches and checks that it is equal to the last context pushed onto the
/// stack.
/// The context is kept on the stack.
template <typename Eq = context_eq, typename Rule>
LEXY_CONSTEVAL auto context_top(Rule)
{
    return _ctx_top<Rule, Eq>{};
}

/// Captures what the Rule matches and checks that it is equal to the last context pushed onto the
/// stack.
/// The context is removed on the stack.
template <typename Eq = context_eq, typename Rule>
LEXY_CONSTEVAL auto context_pop(Rule)
{
    return _ctx_top<Rule, Eq>{} + context_drop;
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_HPP_INCLUDED
