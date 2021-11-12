// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SCAN_HPP_INCLUDED
#define LEXY_DSL_SCAN_HPP_INCLUDED

#include <lexy/_detail/lazy_init.hpp>
#include <lexy/action/base.hpp>
#include <lexy/callback/forward.hpp>
#include <lexy/callback/object.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

//=== rule forward declaration ===//
namespace lexyd
{
template <typename Production>
struct _prd;
template <typename Rule, typename Tag>
struct _peek;

struct _scan;
} // namespace lexyd

namespace lexy::_detail
{
template <typename Derived, typename Reader>
class scanner;
}

//=== scan_result ===//
namespace lexy
{
constexpr struct scan_failed_t
{
} scan_failed;

template <typename T>
class scan_result
{
public:
    using value_type = T;

    constexpr scan_result() = default;
    constexpr scan_result(scan_failed_t) {}

    template <typename U = T, typename = std::enable_if_t<std::is_constructible_v<T, U>>>
    constexpr scan_result(U&& value)
    {
        _value.emplace(LEXY_MOV(value));
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }
    constexpr bool has_value() const noexcept
    {
        return static_cast<bool>(_value);
    }

    constexpr const auto& value() const& noexcept
    {
        return *_value;
    }
    constexpr auto&& value() && noexcept
    {
        return LEXY_MOV(*_value);
    }

    template <typename U = T>
    constexpr U value_or(U&& fallback) const& noexcept
    {
        return _value ? *_value : LEXY_FWD(fallback);
    }
    template <typename U = T>
    constexpr U value_or(U&& fallback) && noexcept
    {
        return _value ? LEXY_MOV(*_value) : LEXY_FWD(fallback);
    }

private:
    _detail::lazy_init<T> _value;

    template <typename Derived, typename Reader>
    friend class _detail::scanner;
};
template <>
class scan_result<void>
{
public:
    using value_type = void;

    constexpr scan_result() = default;
    constexpr scan_result(scan_failed_t) {}
    constexpr scan_result(bool has_value)
    {
        if (has_value)
            _value.emplace();
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }
    constexpr bool has_value() const noexcept
    {
        return static_cast<bool>(_value);
    }

private:
    _detail::lazy_init<void> _value;

    template <typename Derived, typename Reader>
    friend class _detail::scanner;
};

template <typename T>
scan_result(T&&) -> scan_result<std::decay_t<T>>;
} // namespace lexy

//=== scanner implementation ===//
namespace lexy::_detail
{
// The context used for a child production during scanning.
// It forwards all events but overrides the value callback.
template <typename Context,
          typename ValueCallback = lexy::production_value_callback<typename Context::production>>
struct spc_child
{
    using production      = typename Context::production;
    using root_production = typename Context::root_production;
    using value_type      = typename ValueCallback::return_type;

    decltype(Context::handler)       handler;
    decltype(Context::control_block) control_block;
    _detail::lazy_init<value_type>   value;

    constexpr explicit spc_child(decltype(Context::control_block) cb) : control_block(cb) {}

    template <typename ChildProduction>
    constexpr auto sub_context(ChildProduction child)
    {
        using sub_context_t = decltype(LEXY_DECLVAL(Context).sub_context(child));
        return spc_child<sub_context_t>(control_block);
    }

    constexpr auto value_callback()
    {
        return ValueCallback();
    }

    template <typename Event, typename... Args>
    constexpr void on(Event ev, Args&&... args)
    {
        handler.on(control_block->parse_handler, ev, LEXY_FWD(args)...);
    }
};

// The context used for a top-level rule parsing during scanning.
// It forwards all events but overrids the value callback to construct a T.
template <typename T, typename Context>
struct spc
{
    using production      = typename Context::production;
    using root_production = typename Context::root_production;
    using value_type      = T;

    decltype(Context::handler)&      handler;
    decltype(Context::control_block) control_block;
    _detail::lazy_init<T>&           value;

    constexpr explicit spc(_detail::lazy_init<T>& value, Context& context)
    : handler(context.handler), control_block(context.control_block), value(value)
    {}

    template <typename ChildProduction>
    constexpr auto sub_context(ChildProduction child)
    {
        using sub_context_t = decltype(LEXY_DECLVAL(Context).sub_context(child));
        if constexpr (std::is_void_v<T>)
            return spc_child<sub_context_t, _void_value_callback>(control_block);
        else
            return spc_child<sub_context_t>(control_block);
    }

    constexpr auto value_callback()
    {
        return lexy::construct<T>;
    }

    template <typename Event, typename... Args>
    constexpr void on(Event ev, Args&&... args)
    {
        handler.on(control_block->parse_handler, ev, LEXY_FWD(args)...);
    }
};

template <typename Reader>
struct scanner_input
{
    Reader _impl;

    constexpr auto reader() const&
    {
        return _impl;
    }
};

// The common interface of all scanner types.
template <typename Derived, typename Reader>
class scanner
{
public:
    using encoding = typename Reader::encoding;

    constexpr scanner(const scanner&) noexcept = delete;
    constexpr scanner& operator=(const scanner&) noexcept = delete;

    //=== status ===//
    constexpr explicit operator bool() const noexcept
    {
        return _state == _state_normal;
    }

    constexpr bool is_at_eof() const
    {
        return _reader.peek() == Reader::encoding::eof();
    }

    constexpr auto position() const noexcept -> typename Reader::iterator
    {
        return _reader.position();
    }

    constexpr auto remaining_input() const noexcept
    {
        return scanner_input<Reader>{_reader};
    }

    //=== parsing ===//
    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr void parse(scan_result<T>& result, Rule)
    {
        if (_state == _state_failed)
            return;

        _detail::spc context(result._value, static_cast<Derived&>(*this).context());

        using parser = lexy::parser_for<Rule, lexy::_detail::final_parser>;
        auto success = parser::parse(context, _reader);
        if (!success)
            _state = _state_failed;
    }

    template <typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr void parse(Rule rule)
    {
        scan_result<void> result;
        parse(result, rule);
    }

    //=== branch parsing ===//
    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr bool branch(scan_result<T>& result, Rule)
    {
        static_assert(lexy::is_branch_rule<Rule>);
        if (_state == _state_failed)
            return false;

        _detail::spc context(result._value, static_cast<Derived&>(*this).context());

        lexy::branch_parser_for<Rule, decltype(context), Reader> parser{};
        if (!parser.try_parse(context, _reader))
            return false; // branch wasn't token

        auto success = parser.template finish<lexy::_detail::final_parser>(context, _reader);
        if (!success)
            _state = _state_failed;
        return true; // branch was taken
    }

    template <typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr bool branch(Rule rule)
    {
        scan_result<void> result;
        return branch(result, rule);
    }

    //=== error handling ===//
    class error_recovery_guard
    {
    public:
        error_recovery_guard(const error_recovery_guard&) = delete;
        error_recovery_guard& operator=(const error_recovery_guard&) = delete;

        constexpr void cancel() &&
        {
            auto& context = static_cast<Derived&>(*_self).context();
            context.on(parse_events::recovery_cancel{}, _self->_reader.position());
            _self->_state = _state_failed;
        }

        constexpr void finish() &&
        {
            auto& context = static_cast<Derived&>(*_self).context();
            context.on(parse_events::recovery_finish{}, _self->_reader.position());
            _self->_state = _state_normal;
        }

    private:
        constexpr explicit error_recovery_guard(scanner& self) noexcept : _self(&self)
        {
            auto& context = static_cast<Derived&>(*_self).context();
            context.on(parse_events::recovery_start{}, _self->_reader.position());
            _self->_state = _state_recovery;
        }

        scanner* _self;
        friend scanner;
    };

    constexpr auto error_recovery()
    {
        LEXY_PRECONDITION(_state == _state_failed);
        return error_recovery_guard(*this);
    }

    template <typename TokenRule>
    constexpr bool discard(TokenRule rule)
    {
        static_assert(lexy::is_token_rule<TokenRule>);
        if (_state == _state_failed)
            return false;

        auto begin  = _reader.position();
        auto result = lexy::try_match_token(rule, _reader);
        auto end    = _reader.position();

        if (begin != end)
        {
            auto& context = static_cast<Derived&>(*this).context();
            context.on(parse_events::token{}, lexy::error_token_kind, begin, end);
        }

        return result;
    }

    template <typename Tag, typename... Args>
    constexpr void error(Tag, Args&&... args)
    {
        auto& context = static_cast<Derived&>(*this).context();
        context.on(parse_events::error{}, lexy::error<Reader, Tag>(LEXY_FWD(args)...));
    }

    template <typename Tag, typename... Args>
    constexpr void fatal_error(Tag tag, Args&&... args)
    {
        error(tag, LEXY_FWD(args)...);
        _state = _state_failed;
    }

    //=== convenience ===//
    template <typename Rule>
    constexpr bool peek(Rule)
    {
        static_assert(lexy::is_rule<Rule>);
        return branch(dsl::_peek<Rule, void>{});
    }

    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr auto parse(Rule rule)
    {
        scan_result<T> result;
        parse(result, rule);
        return result;
    }
    template <typename Production, typename = lexy::production_rule<Production>>
    constexpr auto parse(Production = {})
    {
        using value_type = typename lexy::production_value_callback<Production>::return_type;

        scan_result<value_type> result;
        parse(result, lexyd::_prd<Production>{});
        return result;
    }

    template <typename Production, typename T, typename = lexy::production_rule<Production>>
    constexpr bool branch(scan_result<T>& result, Production = {})
    {
        return branch(result, lexyd::_prd<Production>{});
    }

protected:
    constexpr explicit scanner(const Reader& reader) noexcept
    : _reader(reader), _state(_state_normal)
    {}

    constexpr Reader& reader() noexcept
    {
        return _reader;
    }

private:
    Reader _reader;

    enum
    {
        _state_normal,
        _state_failed,   // after a failure
        _state_recovery, // recovery guard active
    } _state;
};
} // namespace lexy::_detail

//=== dsl::scan ===//
namespace lexy
{
template <typename Context, typename Reader>
class rule_scanner : public _detail::scanner<rule_scanner<Context, Reader>, Reader>
{
public:
    using production      = typename Context::production;
    using root_production = typename Context::root_production;

    constexpr std::size_t recursion_depth() const noexcept
    {
        auto& cb = _context->control_block();
        return static_cast<std::size_t>(cb.cur_depth);
    }

    constexpr auto begin() const noexcept -> typename Reader::iterator
    {
        return _begin;
    }

private:
    constexpr explicit rule_scanner(Context& context, Reader reader)
    : _detail::scanner<rule_scanner<Context, Reader>, Reader>(reader), _context(&context),
      _begin(reader.position())
    {}

    constexpr Context& context() noexcept
    {
        return *_context;
    }

    Context*                  _context;
    typename Reader::iterator _begin;

    friend _detail::scanner<rule_scanner<Context, Reader>, Reader>;
    friend lexyd::_scan;
};
} // namespace lexy

namespace lexyd
{
struct _scan : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::rule_scanner scanner(context, reader);
            lexy::scan_result  result = Context::production::scan(scanner);
            reader.set_position(scanner.position());
            if (!result)
                return false;

            if constexpr (std::is_void_v<typename decltype(result)::value_type>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                         LEXY_MOV(result).value());
        }
    };
};

inline constexpr auto scan = _scan{};
} // namespace lexyd

namespace lexy
{
template <typename T>
struct scan_production
{
    using scan_result = lexy::scan_result<T>;

    static constexpr auto rule  = dsl::scan;
    static constexpr auto value = lexy::forward<T>;
};
} // namespace lexy

#endif // LEXY_DSL_SCAN_HPP_INCLUDED

