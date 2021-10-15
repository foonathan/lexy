// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_REPORT_ERROR_HPP_INCLUDED
#define LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

#include <cstdio>
#include <lexy/_detail/assert.hpp>
#include <lexy/error.hpp>
#include <lexy/visualize.hpp>
#include <lexy_ext/input_location.hpp>

namespace lexy_ext::_detail
{
// Advances the iterator to the beginning of the next code point.
template <typename Encoding, typename Iterator>
constexpr Iterator find_cp_boundary(Iterator cur, Iterator end)
{
    // [maybe_unused] c breaks MSVC builds targetting C++17
    auto is_cp_boundary = [](auto c) {
        (void)c;
        if constexpr (std::is_same_v<Encoding, lexy::utf8_encoding>)
            return (c & 0b1100'0000) != (0b10 << 6);
        else if constexpr (std::is_same_v<Encoding, lexy::utf16_encoding>)
            return (c & 0b1111'1100'0000'0000) != (0b110111 << 10);
        else
            // This encoding doesn't have continuation code units, so everything is a boundary.
            return std::true_type{};
    };

    while (cur != end && !is_cp_boundary(*cur))
        ++cur;
    return cur;
}

// Split the context of the location into three parts: the one before underlined, the underlined
// one, and the one after. If underlined covers multiple lines, limit to the one of the context or
// the newline afterwards.
template <typename Location, typename Reader>
constexpr auto split_context(const Location& location, const lexy::lexeme<Reader>& underlined)
{
    using encoding     = typename Reader::encoding;
    const auto context = location.context();

    struct result_t
    {
        lexy::lexeme<Reader> before;
        lexy::lexeme<Reader> underlined;
        lexy::lexeme<Reader> after;
    } result;

    // The before part starts at the context and continues until the beginning of the underline.
    // We do not advance the beginning of the underline to the next code point boundary:
    // If the error occurs inside a code point, this should be visible.
    result.before = {context.begin(), underlined.begin()};

    // Check whether we have underlined.begin() > context.end().
    auto underline_after_end
        = lexy::_detail::min_range_end(context.begin(), underlined.begin(), context.end())
          == context.end();
    if (underline_after_end)
    {
        // The underlined part is inside the newline.
        auto newline = location.newline();
        LEXY_PRECONDITION(lexy::_detail::precedes(newline.begin(), underlined.begin()));

        // The end of the underlined part is either the end of the underline or the newline.
        // Due to the nature of newlines, we don't need to advance to the next code point boundary.
        auto underlined_end
            = lexy::_detail::min_range_end(underlined.begin(), underlined.end(), newline.end());

        // Use the trimmed underline, and nothing comes after it.
        result.underlined = {underlined.begin(), underlined_end};
        result.after      = {underlined_end, underlined_end};
    }
    else
    {
        // The end of the underlined part is either the end of the underline or the end of the
        // context...
        auto underlined_end
            = lexy::_detail::min_range_end(underlined.begin(), underlined.end(), context.end());
        // ... advanced to the next code point boundary.
        // This ensures that something like `dsl::ascii::alpha` does not split a non-ASCII code
        // point.
        underlined_end = find_cp_boundary<encoding>(underlined_end, context.end());

        // Use the trimmed underline, everything that remains (if any) is part of the context.
        result.underlined = {underlined.begin(), underlined_end};
        result.after      = {underlined_end, context.end()};
    }

    return result;
}

enum class annotation_kind
{
    primary,
    secondary,
};

template <typename Input>
struct error_writer
{
    lexy::visualization_options opts;

    const auto* column() const
    {
        if (opts.is_set(lexy::visualize_use_unicode))
            return u8"│";
        else
            return u8"|";
    }

    const auto* underline(annotation_kind kind) const
    {
        switch (kind)
        {
        case annotation_kind::primary:
            return "^";
        case annotation_kind::secondary:
            return "~";
        }

        return "";
    }

    //=== writers ===//
    template <typename OutputIt, typename Writer>
    OutputIt write_message(OutputIt out, const Writer& message)
    {
        using namespace lexy::_detail;

        out    = write_color<color::red, color::bold>(out, opts);
        out    = write_str(out, "error: ");
        out    = write_color<color::reset>(out, opts);
        out    = message(out, opts);
        *out++ = '\n';

        return out;
    }

    template <typename OutputIt>
    OutputIt write_empty_annotation(OutputIt out) const
    {
        using namespace lexy::_detail;

        out    = write_str(out, "     ");
        out    = write_str(out, column());
        *out++ = '\n';
        return out;
    }

    template <typename OutputIt, typename Location, typename Writer>
    OutputIt write_annotation(OutputIt out, annotation_kind kind, const Location& location,
                              const lexy::lexeme_for<Input>& _underlined,
                              const Writer&                  message) const
    {
        using namespace lexy::_detail;

        auto colorize_underline = [&](OutputIt out) {
            switch (kind)
            {
            case annotation_kind::primary:
                return write_color<color::red, color::bold>(out, opts);
            case annotation_kind::secondary:
                return write_color<color::yellow>(out, opts);
            }

            return out;
        };

        auto [before, underlined, after] = split_context(location, _underlined);

        //=== Line with file contents ===//
        // Location column.
        out    = write_format(out, "%4zd ", location.line_nr());
        out    = write_str(out, column());
        *out++ = ' ';

        // Print before underlined normally.
        out = lexy::visualize_to(out, before, opts);

        // Print underlined colored.
        out = colorize_underline(out);
        out = lexy::visualize_to(out, underlined, opts.reset(lexy::visualize_use_color));
        out = write_color<color::reset>(out, opts);

        // Print after underlined normally.
        out    = lexy::visualize_to(out, after, opts);
        *out++ = '\n';

        //==== Line with annotation ===//
        // Initial column.
        out    = write_str(out, "     ");
        out    = write_str(out, column());
        *out++ = ' ';

        // Indent until the underline.
        auto indent_count = lexy::visualization_display_width(before, opts);
        for (auto i = 0u; i != indent_count; ++i)
            *out++ = ' ';

        // Colorize.
        out = colorize_underline(out);

        // Then underline.
        auto underline_count = lexy::visualization_display_width(underlined, opts);
        if (underline_count == 0)
            underline_count = 1;
        for (auto i = 0u; i != underline_count; ++i)
            out = write_str(out, underline(kind));
        *out++ = ' ';

        // Print the message.
        out    = message(out, opts.reset(lexy::visualize_use_color));
        *out++ = '\n';

        return write_color<color::reset>(out, opts);
    }
};
} // namespace lexy_ext::_detail

namespace lexy_ext::_detail
{
template <typename OutputIt, typename Production, typename Input, typename Reader, typename Tag>
OutputIt write_error(OutputIt out, const lexy::error_context<Production, Input>& context,
                     const lexy::error<Reader, Tag>& error, lexy::visualization_options opts)
{
    _detail::error_writer<Input> writer{opts};

    // Convert the context location and error location into line/column information.
    lexy_ext::input_location_finder finder(context.input());

    auto context_location = finder.find(context.position());
    auto location         = finder.find(error.position(), context_location);

    // Write the main error headline.
    out = writer.write_message(out, [&](OutputIt out, lexy::visualization_options) {
        out = lexy::_detail::write_str(out, "while parsing ");
        out = lexy::_detail::write_str(out, context.production());
        return out;
    });
    out = writer.write_empty_annotation(out);

    // Write an annotation for the context.
    if (location.line_nr() != context_location.line_nr())
    {
        out = writer.write_annotation(out, annotation_kind::secondary, context_location,
                                      {context.position(), 1},
                                      [&](OutputIt out, lexy::visualization_options) {
                                          return lexy::_detail::write_str(out, "beginning here");
                                      });
        out = writer.write_empty_annotation(out);
    }

    // Write the main annotation.
    if constexpr (std::is_same_v<Tag, lexy::expected_literal>)
    {
        auto string = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string());

        out = writer.write_annotation(out, annotation_kind::primary, location,
                                      {error.position(), error.index() + 1},
                                      [&](OutputIt out, lexy::visualization_options opts) {
                                          out = lexy::_detail::write_str(out, "expected '");
                                          out = lexy::visualize_to(out, string, opts);
                                          out = lexy::_detail::write_str(out, "'");
                                          return out;
                                      });
    }
    else if constexpr (std::is_same_v<Tag, lexy::expected_keyword>)
    {
        auto string = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string());

        out = writer.write_annotation(out, annotation_kind::primary, location,
                                      {error.begin(), error.end()},
                                      [&](OutputIt out, lexy::visualization_options opts) {
                                          out = lexy::_detail::write_str(out, "expected keyword '");
                                          out = lexy::visualize_to(out, string, opts);
                                          out = lexy::_detail::write_str(out, "'");
                                          return out;
                                      });
    }
    else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
    {
        out = writer.write_annotation(out, annotation_kind::primary, location,
                                      {error.position(), 1},
                                      [&](OutputIt out, lexy::visualization_options) {
                                          out = lexy::_detail::write_str(out, "expected '");
                                          out = lexy::_detail::write_str(out,
                                                                         error.character_class());
                                          out = lexy::_detail::write_str(out, "' character");
                                          return out;
                                      });
    }
    else
    {
        auto underlined = [&] {
            if (error.begin() == error.end())
                return lexy::lexeme_for<Input>(error.position(), 1);
            else
                return lexy::lexeme_for<Input>(error.begin(), error.end());
        }();

        out = writer.write_annotation(out, annotation_kind::primary, location, underlined,
                                      [&](OutputIt out, lexy::visualization_options) {
                                          return lexy::_detail::write_str(out, error.message());
                                      });
    }

    return out;
}
} // namespace lexy_ext::_detail

namespace lexy_ext
{
struct _report_error
{
    struct _sink
    {
        std::size_t _count;

        using return_type = std::size_t;

        template <typename Production, typename Input, typename Reader, typename Tag>
        void operator()(const lexy::error_context<Production, Input>& context,
                        const lexy::error<Reader, Tag>&               error)
        {
            _detail::write_error(lexy::cfile_output_iterator{stderr}, context, error,
                                 {lexy::visualize_fancy});
            ++_count;
        }

        std::size_t finish() &&
        {
            if (_count != 0)
                std::fputs("\n", stderr);
            return _count;
        }
    };

    constexpr auto sink() const
    {
        return _sink{};
    }
};

// The error callback that prints to stderr.
constexpr auto report_error = _report_error{};
} // namespace lexy_ext

#endif // LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

