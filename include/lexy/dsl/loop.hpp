// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOP_HPP_INCLUDED
#define LEXY_DSL_LOOP_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/result.hpp>

namespace lexyd
{
// The handler used for parsing the rules inside a loop.
// It will package any error nicely and return it to us.
template <typename Handler>
struct _loop_handler
{
    Handler& _handler;
    bool     _break;

    using result_type = lexy::result<void, typename Handler::result_type>;

    template <typename SubProduction, typename Reader>
    constexpr auto sub_handler(const Reader& reader)
    {
        return _handler.template sub_handler<SubProduction>(reader);
    }

    constexpr auto list_sink()
    {
        return _handler.sink();
    }

    template <typename Reader, typename Error>
    constexpr auto error(const Reader& reader, Error&& error) &&
    {
        // We report errors to the normal handler.
        return result_type(lexy::result_error, LEXY_MOV(_handler).error(reader, LEXY_FWD(error)));
    }

    constexpr auto value() &&
    {
        return result_type(lexy::result_value);
    }

    constexpr auto break_() &&
    {
        _break = true;
        return result_type(lexy::result_value);
    }
};
} // namespace lexyd

namespace lexyd
{
struct _break : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(_loop_handler<Handler>& handler, Reader&, Args&&...) ->
            typename _loop_handler<Handler>::result_type
        {
            return LEXY_MOV(handler).break_();
        }
    };
};

/// Exits a loop().
constexpr auto break_ = _break{};
} // namespace lexyd

namespace lexyd
{
template <typename... PrevArgs>
struct _loop_iter_parser
{
    template <typename Handler, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(_loop_handler<Handler>& handler, Reader&, PrevArgs&&..., Args&&...) ->
        typename _loop_handler<Handler>::result_type
    {
        static_assert(sizeof...(Args) == 0, "looped rule must not add any values");
        return LEXY_MOV(handler).value();
    }
};

template <typename Rule>
struct _loop : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            _loop_handler<Handler> loop_handler{handler, false};
            while (!loop_handler._break)
            {
                using continuation = _loop_iter_parser<Args...>;
                auto result = Rule::template parser<continuation>::parse(loop_handler, reader,
                                                                         LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeatedly matches the rule until a break rule matches.
template <typename Rule>
LEXY_CONSTEVAL auto loop(Rule)
{
    return _loop<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOP_HPP_INCLUDED

