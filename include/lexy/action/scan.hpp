// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_SCAN_HPP_INCLUDED
#define LEXY_ACTION_SCAN_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/action/validate.hpp>
#include <lexy/dsl/scan.hpp>

namespace lexy
{
template <typename ControlProduction>
struct _scp;
template <>
struct _scp<void>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "scanner control production";
    }

    static constexpr auto rule  = dsl::scan;
    static constexpr auto value = lexy::noop;
};
template <typename ControlProduction>
struct _scp : ControlProduction, _scp<void>
{};

template <typename ControlProduction, typename Input, typename ErrorCallback>
class scanner : public _detail::scanner<scanner<ControlProduction, Input, ErrorCallback>,
                                        lexy::input_reader<Input>>
{
    using _impl       = _detail::scanner<scanner<ControlProduction, Input, ErrorCallback>,
                                   lexy::input_reader<Input>>;
    using _handler    = lexy::validate_handler<Input, ErrorCallback>;
    using _production = _scp<ControlProduction>;

public:
    constexpr auto finish() && -> lexy::validate_result<ErrorCallback>
    {
        auto parse_result = static_cast<bool>(*this);

        if (parse_result)
            _context.on(parse_events::production_finish{}, this->position());
        else
            _context.on(parse_events::production_cancel{}, this->position());

        return LEXY_MOV(_cb.parse_handler).get_result_void(parse_result);
    }

private:
    constexpr explicit scanner(const Input& input, const ErrorCallback& callback)
    : _impl(input.reader()),
      _cb(_handler(input, callback), no_parse_state, max_recursion_depth<_production>()),
      _context(&_cb)
    {
        _context.on(parse_events::production_start{}, this->position());
    }

    auto& context() noexcept
    {
        return _context;
    }

    _detail::parse_context_control_block<_handler> _cb;
    _pc<_handler, void, _production>               _context;

    friend _impl;
    template <typename, typename I, typename EC>
    friend constexpr auto scan(const I& input, const EC& callback);
};

template <typename ControlProduction = void, typename Input, typename ErrorCallback>
constexpr auto scan(const Input& input, const ErrorCallback& callback)
{
    return scanner<ControlProduction, Input, ErrorCallback>(input, callback);
}
} // namespace lexy

#endif // LEXY_ACTION_SCAN_HPP_INCLUDED

