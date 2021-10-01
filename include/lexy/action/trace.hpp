// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_TRACE_HPP_INCLUDED
#define LEXY_ACTION_TRACE_HPP_INCLUDED

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/action/base.hpp>
#include <lexy/callback/noop.hpp>
#include <lexy/token.hpp>
#include <lexy/visualize.hpp>
#include <lexy_ext/input_location.hpp> // implementation detail only

//=== debug_event ===//
namespace lexy::parse_events
{
/// Debug event was triggered.
/// Arguments: pos, str
struct debug_event
{};
} // namespace lexy::parse_events

namespace lexyd
{
template <typename CharT, CharT... C>
struct _debug : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<>;
            context.on(_ev::debug_event{}, reader.position(), str);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

#if LEXY_HAS_NTTP
template <lexy::_detail::string_literal Str>
constexpr auto debug = lexy::_detail::to_type_string<_debug, Str>{};
#endif

#define LEXY_DEBUG(Str)                                                                            \
    LEXY_NTTP_STRING(::lexyd::_debug, Str) {}
} // namespace lexyd

//=== trace ====//
namespace lexy
{
namespace _ev = lexy::parse_events;

template <typename OutputIt, typename Input, typename TokenKind = void>
class trace_handler
{
    using location_finder = lexy_ext::input_location_finder<Input>;
    using location        = typename location_finder::location;

    struct label_t
    {
        const LEXY_CHAR_OF_u8* line;
        const LEXY_CHAR_OF_u8* event;
        const LEXY_CHAR_OF_u8* finish_event;
        const LEXY_CHAR_OF_u8* cancel_event;
    };

    label_t label() const
    {
        return _opts.is_set(visualize_use_unicode) ? label_t{u8"│  ", u8"├──", u8"┴", u8"└"}
                                                   : label_t{u8"  ", u8"- ", u8"- finish", u8"-"};
    }

    enum class prefix
    {
        event,
        cancel,
        finish,
    };

    OutputIt write_prefix(OutputIt out, std::size_t cur_depth, const location& loc, prefix p) const
    {
        const auto l = label();

        if (cur_depth > 0)
            *out++ = '\n';

        out = _detail::write_color<_detail::color::faint>(out, _opts);
        out = _detail::write_format(out, "%2zu:%3zu", loc.line_nr(), loc.column_nr());
        out = _detail::write_str(out, ": ");
        out = _detail::write_color<_detail::color::reset>(out, _opts);

        if (cur_depth > 0)
        {
            for (auto i = 0u; i != cur_depth - 1; ++i)
                out = _detail::write_str(out, l.line);

            switch (p)
            {
            case prefix::event:
                out = _detail::write_str(out, l.event);
                break;
            case prefix::cancel:
                out = _detail::write_str(out, l.cancel_event);
                out = _detail::write_color<_detail::color::yellow>(out, _opts);
                if (_opts.is_set(visualize_use_unicode))
                    out = _detail::write_str(out, u8"╳");
                else
                    out = _detail::write_str(out, "x");
                out = _detail::write_color<_detail::color::reset>(out, _opts);
                break;
            case prefix::finish:
                out = _detail::write_str(out, l.finish_event);
                break;
            }
        }

        return out;
    }

public:
    explicit trace_handler(OutputIt out, const Input& input,
                           visualization_options opts = {}) noexcept
    : _out(out), _cur_depth(0), _locations(input), _anchor(_locations.beginning()), _opts(opts)
    {
        LEXY_PRECONDITION(_opts.max_tree_depth <= visualization_options::max_tree_depth_limit);
    }

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    OutputIt get_result_value() && noexcept
    {
        *_out++ = '\n';
        return _out;
    }
    template <typename Production>
    OutputIt get_result_empty() && noexcept
    {
        *_out++ = '\n';
        return _out;
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {
        // The beginning of the previous production.
        // If the current production gets canceled, it needs to be restored.
        location previous_anchor;
    };

    template <typename Production, typename Iterator>
    marker<Production> on(_ev::production_start<Production>, Iterator pos)
    {
        const auto loc = _locations.find(pos, _anchor);

        // All other events are after this point.
        auto previous_anchor = _anchor;
        _anchor              = loc;

        if (_cur_depth <= _opts.max_tree_depth)
        {
            _out = write_prefix(_out, _cur_depth, loc, prefix::event);
            _out = _detail::write_color<_detail::color::bold>(_out, _opts);
            _out = _detail::write_str(_out, lexy::production_name<Production>());
            _out = _detail::write_color<_detail::color::reset>(_out, _opts);

            if (_cur_depth == _opts.max_tree_depth)
            {
                // Print an ellipsis instead of children.
                _out = _detail::write_str(_out, ": ");
                _out = _detail::write_ellipsis(_out, _opts);
            }
            else
            {
                // Prepare for children.
                _out = _detail::write_str(_out, ":");
            }
        }

        ++_cur_depth;
        _last_token.reset();

        return {previous_anchor};
    }

    template <typename Production, typename Iterator>
    auto on(marker<Production>, _ev::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename TK, typename Iterator>
    void on(const marker<Production>&, _ev::token, TK _kind, Iterator begin, Iterator end)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        const auto kind = lexy::token_kind<TokenKind>(_kind);
        const auto loc  = _locations.find(begin, _anchor);

        if (_last_token.merge(Production{}, kind))
        {
            _out = visualize_to(_out, lexy::lexeme_for<Input>(begin, end), _opts | visualize_space);
        }
        else
        {
            _out = write_prefix(_out, _cur_depth, loc, prefix::event);
            _out = _detail::write_color<_detail::color::bold>(_out, _opts);
            _out = _detail::write_str(_out, kind.name());
            _out = _detail::write_color<_detail::color::reset>(_out, _opts);

            if (begin != end)
            {
                _out = _detail::write_str(_out, ": ");
                _out = visualize_to(_out, lexy::lexeme_for<Input>(begin, end),
                                    _opts | visualize_space);
            }
        }

        _last_token.update(kind);
    }

    template <typename Production, typename Reader, typename Tag>
    void on(marker<Production>, _ev::error, const lexy::error<Reader, Tag>& error)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        const auto loc = _locations.find(error.position(), _anchor);

        _out = write_prefix(_out, _cur_depth, loc, prefix::event);
        _out = _detail::write_color<_detail::color::red, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "error");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::red>(_out, _opts);
        _out = _detail::write_str(_out, ": ");

        if constexpr (std::is_same_v<Tag, lexy::expected_literal>)
        {
            auto string = _detail::make_literal_lexeme<typename Reader::encoding>(error.string());

            _out = _detail::write_str(_out, "expected '");
            _out = visualize_to(_out, string, _opts);
            _out = _detail::write_str(_out, "'");
        }
        else if constexpr (std::is_same_v<Tag, lexy::expected_keyword>)
        {
            auto string = _detail::make_literal_lexeme<typename Reader::encoding>(error.string());

            _out = _detail::write_str(_out, "expected keyword '");
            _out = visualize_to(_out, string, _opts);
            _out = _detail::write_str(_out, "'");
        }
        else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
        {
            _out = _detail::write_str(_out, "expected '");
            _out = _detail::write_str(_out, error.character_class());
            _out = _detail::write_str(_out, "' character");
        }
        else
        {
            _out = _detail::write_str(_out, error.message());
        }

        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        // We can no longer merge tokens.
        _last_token.reset();
    }

    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::backtracked, Iterator begin, Iterator end)
    {
        if (_cur_depth > _opts.max_tree_depth
            // If we haven't actually consumed any characters, we didn't really backtrack;
            // peeking at the next character is allowed.
            || begin == end)
            return;

        const auto loc = _locations.find(begin, _anchor);

        _out = write_prefix(_out, _cur_depth, loc, prefix::event);
        _out = _detail::write_color<_detail::color::yellow, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "backtracked");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        if (begin != end)
        {
            _out = _detail::write_str(_out, ": ");
            _out = _detail::write_color<_detail::color::yellow>(_out, _opts);
            _out = visualize_to(_out, lexy::lexeme_for<Input>(begin, end),
                                _opts.reset(visualize_use_color) | visualize_space);
            _out = _detail::write_color<_detail::color::reset>(_out, _opts);
        }

        // We can no longer merge tokens.
        _last_token.reset();
    }

    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::recovery_start, Iterator pos)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        const auto loc = _locations.find(pos, _anchor);
        _out           = write_prefix(_out, _cur_depth, loc, prefix::event);
        _out = _detail::write_color<_detail::color::yellow, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "error recovery");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::yellow>(_out, _opts);
        _out = _detail::write_str(_out, ":");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        if (_cur_depth == _opts.max_tree_depth)
        {
            // Print an ellipsis instead of children.
            _out = _detail::write_str(_out, " ");
            _out = _detail::write_ellipsis(_out, _opts);
        }

        // We can no longer merge tokens.
        _last_token.reset();
        // Treat it as an extra level.
        ++_cur_depth;
    }

    template <typename Iterator>
    void on_recovery_end(Iterator pos, bool success)
    {
        if (_cur_depth < _opts.max_tree_depth)
        {
            const auto loc = _locations.find(pos, _anchor);
            _out = write_prefix(_out, _cur_depth, loc, success ? prefix::finish : prefix::cancel);

            if (!success)
            {}
        }

        _last_token.reset();
        --_cur_depth;
    }
    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::recovery_finish, Iterator pos)
    {
        on_recovery_end(pos, true);
    }
    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::recovery_cancel, Iterator pos)
    {
        on_recovery_end(pos, false);
    }

    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::debug_event, Iterator pos, const char* str)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        const auto loc = _locations.find(pos, _anchor);

        _out = write_prefix(_out, _cur_depth, loc, prefix::event);
        _out = _detail::write_color<_detail::color::blue, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "debug");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::blue>(_out, _opts);
        _out = _detail::write_str(_out, ": ");
        _out = _detail::write_str(_out, str);
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        // We can no longer merge tokens.
        _last_token.reset();
    }

    template <typename Production, typename Iterator, typename... Args>
    void on(marker<Production>&&, _ev::production_finish<Production>, Iterator pos, Args&&...)
    {
        if (_cur_depth <= _opts.max_tree_depth)
        {
            const auto loc = _locations.find(pos, _anchor);
            _out           = write_prefix(_out, _cur_depth, loc, prefix::finish);
        }

        --_cur_depth;
    }
    template <typename Production, typename Iterator>
    void on(marker<Production>&& m, _ev::production_cancel<Production>, Iterator pos)
    {
        if (_cur_depth <= _opts.max_tree_depth)
        {
            const auto loc = _locations.find(pos, _anchor);
            _out           = write_prefix(_out, _cur_depth, loc, prefix::cancel);
        }

        --_cur_depth;

        // Restore the anchor as we've backtracked.
        _anchor = m.previous_anchor;
    }

private:
    struct last_token_info
    {
        bool                        first_token;
        lexy::token_kind<TokenKind> kind;

        last_token_info() : first_token(true) {}

        template <typename Production>
        bool merge(Production, lexy::token_kind<TokenKind> new_kind) const
        {
            if (first_token || kind != new_kind)
                return false;

            // We can merge inside a token production or if it's an error token.
            return lexy::is_token_production<Production> || kind == lexy::error_token_kind;
        }

        void update(lexy::token_kind<TokenKind> new_kind)
        {
            first_token = false;
            kind        = new_kind;
        }

        void reset()
        {
            first_token = true;
        }
    };

    OutputIt _out;

    std::size_t     _cur_depth;
    last_token_info _last_token;

    location_finder _locations;
    location        _anchor;

    visualization_options _opts;
};

template <typename Production, typename TokenKind = void, typename OutputIt, typename Input>
OutputIt trace_to(OutputIt out, const Input& input, visualization_options opts = {})
{
    auto reader = input.reader();
    return lexy::do_action<Production>(trace_handler<OutputIt, Input, TokenKind>(out, input, opts),
                                       reader);
}

template <typename Production, typename TokenKind = void, typename Input>
void trace(std::FILE* file, const Input& input, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, opts);
}
} // namespace lexy

#endif // LEXY_ACTION_TRACE_HPP_INCLUDED

