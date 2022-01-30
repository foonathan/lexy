// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED
#define LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/newline.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy_ext::_detail
{
[[deprecated(
    "`<lexy_ext/input_location.hpp>` is deprecated; migrate to `<lexy/input_location.hpp>`")]] //
static int input_location_hpp_is_deprecated;
static int trigger_input_locatoin_hpp_deprecated = input_location_hpp_is_deprecated;
} // namespace lexy_ext::_detail

namespace lexy_ext
{
// Fake token that counts code units without verification.
struct _unchecked_code_unit
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            if (reader.peek() == Reader::encoding::eof())
                return false;

            reader.bump();
            end = reader.position();
            return true;
        }
    };
};

/// Converts positions (iterators) into locations (line/column nr).
///
/// The unit for line and column numbers can be customized.
/// Every time the corresponding token matches, is the corresponding number increased.
///
/// See https://foonathan.net/2021/02/column/ for a discussion of potential units.
/// Use e.g. `lexy::dsl::code_point` and `lexy::dsl::newline` to count code points.
///
/// By default, it counts code units and newlines.
template <typename Input, typename TokenColumn = _unchecked_code_unit,
          typename TokenLine = LEXY_DECAY_DECLTYPE(lexy::dsl::newline)>
class input_location_finder
{
public:
    using iterator = typename lexy::input_reader<Input>::iterator;

    /// The location in the input.
    class location
    {
    public:
        constexpr std::size_t line_nr() const noexcept
        {
            return _line;
        }
        constexpr std::size_t column_nr() const noexcept
        {
            return _column;
        }

        /// The entire line that contains the position.
        constexpr lexy::lexeme_for<Input> context() const
        {
            auto eol = _reader.position();
            for (auto reader = _reader; true; reader.bump())
            {
                eol = reader.position();
                if (reader.peek() == decltype(reader)::encoding::eof()
                    || lexy::try_match_token(TokenLine{}, reader))
                    break;
            }

            return {_reader.position(), eol};
        }

        /// The newline after the line, if there is any.
        constexpr lexy::lexeme_for<Input> newline() const
        {
            // Advance to EOl.
            for (auto reader = _reader; true; reader.bump())
            {
                auto pos = reader.position();
                if (reader.peek() == decltype(reader)::encoding::eof())
                    return {pos, pos};
                else if (lexy::try_match_token(TokenLine{}, reader))
                    return {pos, reader.position()};
            }

            return {}; // unreachable
        }

    private:
        constexpr location(lexy::input_reader<Input> reader, std::size_t line, std::size_t column)
        : _reader(LEXY_MOV(reader)), _line(line), _column(column)
        {}

        // The reader starts at the beginning of the given line.
        lexy::input_reader<Input> _reader;
        std::size_t               _line, _column;

        friend input_location_finder;
    };

    constexpr explicit input_location_finder(const Input& input) : _reader(input.reader()) {}
    constexpr explicit input_location_finder(const Input& input, TokenColumn, TokenLine)
    : _reader(input.reader())
    {}

    /// The starting location.
    constexpr location beginning() const
    {
        return location(_reader, 1, 1);
    }

    /// Finds the given position, starting at the anchor location.
    /// This is an optimization if you know the position is after the anchor.
    constexpr location find(iterator pos, const location& anchor) const
    {
        auto reader = anchor._reader;

        // We start at the given line in the initial column.
        std::size_t cur_line   = anchor._line;
        std::size_t cur_column = 1;
        auto        line_start = reader;

        // Find the given position.
        while (true)
        {
            if (reader.position() == pos)
            {
                // We found the position of the error.
                break;
            }
            else if (lexy::token_parser_for<TokenLine, decltype(reader)> nl(reader);
                     nl.try_parse(reader))
            {
                LEXY_ASSERT(reader.position() != nl.end, "TokenLine must consume input");
                reader.bump();

                // Check whether our location points inside the newline.
                auto found = false;
                while (reader.position() != nl.end)
                {
                    found |= reader.position() == pos;
                    reader.bump();
                }

                // We pretend the location is at the beginning of the newline.
                if (found)
                    break;

                // We're at a new line.
                ++cur_line;
                cur_column = 1;
                line_start = reader;
            }
            else if (lexy::token_parser_for<TokenColumn, decltype(reader)> column(reader);
                     column.try_parse(reader))
            {
                LEXY_ASSERT(reader.position() != column.end, "TokenColumn must consume input");
                reader.bump();

                // Check whether our location points inside a column.
                auto found = false;
                while (reader.position() != column.end)
                {
                    found |= reader.position() == pos;
                    reader.bump();
                }

                // We pretend the location is at the beginning of the column.
                if (found)
                    break;

                // Next column.
                ++cur_column;
            }
            else if (reader.peek() == decltype(reader)::encoding::eof())
            {
                // We have an OOB error position.
                LEXY_PRECONDITION(false);
                break;
            }
            else
            {
                // Invalid column, just ignore it in the column count.
                reader.bump();
            }
        }

        // Return where we ended up.
        return location(line_start, cur_line, cur_column);
    }

    /// Finds the location of the position.
    constexpr location find(iterator pos) const
    {
        // We start at the beginning of the file with the search.
        auto anchor = beginning();
        return find(pos, anchor);
    }

private:
    lexy::input_reader<Input> _reader;
};

/// Convenience function to find a single location.
template <typename Input, typename TokenColumn, typename TokenLine>
constexpr auto find_input_location(const Input&                                 input,
                                   typename lexy::input_reader<Input>::iterator pos, TokenColumn,
                                   TokenLine)
{
    return input_location_finder<Input, TokenColumn, TokenLine>(input).find(pos);
}
template <typename Input>
constexpr auto find_input_location(const Input&                                 input,
                                   typename lexy::input_reader<Input>::iterator pos)
{
    return input_location_finder<Input>(input).find(pos);
}
} // namespace lexy_ext

#endif // LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED

