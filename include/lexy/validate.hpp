// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_VALIDATE_HPP_INCLUDED
#define LEXY_VALIDATE_HPP_INCLUDED

#include <lexy/callback.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/production.hpp>
#include <lexy/result.hpp>

namespace lexy
{
template <typename Production, typename Input, typename Callback>
class _validate_handler
{
public:
    constexpr explicit _validate_handler(const Input& input, const input_reader<Input>& reader,
                                         Callback cb)
    : _err_ctx(input, reader.cur()), _callback(cb)
    {}

    using result_type = result<void, typename Callback::return_type>;

    template <typename SubProduction>
    constexpr auto sub_handler(const input_reader<Input>& reader)
    {
        return _validate_handler<SubProduction, Input, Callback>(_err_ctx.input(), reader,
                                                                 _callback);
    }

    constexpr auto list_sink()
    {
        return noop.sink();
    }

    template <typename Error>
    constexpr auto error(Error&& error) &&
    {
        return lexy::invoke_as_result<result_type>(lexy::result_error, _callback, _err_ctx,
                                                   LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr auto value(Args&&...) &&
    {
        return result_type(lexy::result_value);
    }

private:
    error_context<Production, Input> _err_ctx;
    LEXY_EMPTY_MEMBER Callback       _callback;
};

template <typename Production, typename Input, typename Callback>
constexpr auto validate(const Input& input, Callback callback)
{
    auto reader = input.reader();

    using handler_t = _validate_handler<Production, Input, Callback>;
    handler_t handler(input, reader, callback);

    using traits = production_traits<Production>;
    return traits::rule::type::template parser<final_parser>::parse(handler, reader);
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED

