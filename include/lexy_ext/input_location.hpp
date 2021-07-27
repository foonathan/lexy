// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED
#define LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/newline.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy_ext
{
// Fake token that counts code units without verification.
struct _unchecked_code_unit
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            invalid = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (reader.eof())
                return error_code::invalid;

            reader.bump();
            return error_code();
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
          typename TokenLine = std::decay_t<decltype(lexy::dsl::newline)>>
class input_location_finder
{
    using engine_column = typename TokenColumn::token_engine;
    using engine_line   = typename TokenLine::token_engine;

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
            return {_reader.cur(), _eol};
        }

        /// The newline after the line, if there is any.
        constexpr lexy::lexeme_for<Input> newline() const
        {
            auto reader = _reader;
            // Advance to EOl.
            while (reader.cur() != _eol)
                reader.bump();
            // Bump newline.
            lexy::engine_try_match<engine_line>(reader);
            return {_eol, reader.cur()};
        }

    private:
        constexpr location(lexy::input_reader<Input> reader, std::size_t line, std::size_t column)
        : _reader(LEXY_MOV(reader)), _eol(), _line(line), _column(column)
        {
            // Find EOL.
            for (auto reader = _reader; true; reader.bump())
            {
                if (reader.eof() || lexy::engine_peek<engine_line>(reader))
                {
                    _eol = reader.cur();
                    break;
                }
            }
        }

        // The reader starts at the beginning of the given line.
        lexy::input_reader<Input> _reader;
        iterator                  _eol;
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
            if (reader.cur() == pos)
            {
                // We found the position of the error.
                break;
            }
            else if (lexy::engine_try_match<engine_line>(reader))
            {
                // We're at a new line.
                ++cur_line;
                cur_column = 1;
                line_start = reader;
            }
            else if (lexy::engine_try_match<engine_column>(reader))
            {
                // Next column.
                ++cur_column;
            }
            else if (reader.eof())
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

