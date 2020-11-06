// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_ARGV_INPUT_HPP_INCLUDED
#define LEXY_INPUT_ARGV_INPUT_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/std.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
/// A sentinel for the command-line arguments.
class argv_sentinel
{};

/// An iterator over the command-line arguments.
class argv_iterator
{
public:
    using value_type        = char;
    using reference         = const char&;
    using pointer           = const char*;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    constexpr argv_iterator() noexcept : _arg(nullptr), _c(nullptr) {}

    //=== dereference ===//
    constexpr reference operator*() const noexcept
    {
        return *_c;
    }
    constexpr pointer operator->() const noexcept
    {
        return _c;
    }

    //=== positioning ===//
    constexpr argv_iterator& operator++() noexcept
    {
        LEXY_PRECONDITION(*this != argv_sentinel{});
        if (*_c == '\0')
        {
            // Go to next argument.
            // We will have one, otherwise the precondition above would have fired.
            ++_arg;
            // Update c to the beginning of next argument.
            _c = *_arg;
        }
        else
            ++_c;

        return *this;
    }
    constexpr argv_iterator operator++(int) noexcept
    {
        argv_iterator tmp(*this);
        ++*this;
        return tmp;
    }

    constexpr argv_iterator& operator--() noexcept
    {
        // Check whether we point to the first character of the argument.
        if (_c == *_arg)
        {
            // Go to end of previous argument.
            --_arg;
            _c = *_arg;
            while (*_c != '\0')
                ++_c;
        }
        else
            --_c;
        return *this;
    }
    constexpr argv_iterator operator--(int) noexcept
    {
        argv_iterator tmp(*this);
        --*this;
        return tmp;
    }

    //=== comparison ===//
    friend constexpr bool operator==(argv_iterator lhs, argv_sentinel) noexcept
    {
        // We're at the end if we're at the last character of the last argument,
        // or have an empty argv range.
        return lhs._c == nullptr || (*lhs._c == '\0' && lhs._arg[1] == nullptr);
    }
    friend constexpr bool operator!=(argv_iterator lhs, argv_sentinel) noexcept
    {
        return !(lhs == argv_sentinel{});
    }
    friend constexpr bool operator==(argv_sentinel, argv_iterator rhs) noexcept
    {
        return rhs == argv_sentinel{};
    }
    friend constexpr bool operator!=(argv_sentinel, argv_iterator rhs) noexcept
    {
        return !(rhs == argv_sentinel{});
    }

    friend constexpr bool operator==(argv_iterator lhs, argv_iterator rhs) noexcept
    {
        return lhs._arg == rhs._arg && lhs._c == rhs._c;
    }
    friend constexpr bool operator!=(argv_iterator lhs, argv_iterator rhs) noexcept
    {
        return !(lhs == rhs);
    }

private:
    constexpr explicit argv_iterator(char** argument, char* c) noexcept : _arg(argument), _c(c) {}

    char** _arg;
    char*  _c;

    friend constexpr argv_iterator argv_begin(int argc, char* argv[]) noexcept;
    friend constexpr argv_iterator argv_end(int argc, char* argv[]) noexcept;
};

/// Returns an iterator to the beginning of the command-line arguments.
constexpr argv_iterator argv_begin(int argc, char* argv[]) noexcept
{
    if (argc <= 1)
        // Create an iterator where *arg_ == nullptr, *c_ == nullptr.
        return argv_iterator(&argv[argc], nullptr);
    else
        return argv_iterator(&argv[1], &argv[1][0]);
}

/// Returns an iterator one past the end of the command-line arguments.
constexpr argv_iterator argv_end(int argc, char* argv[]) noexcept
{
    if (argc <= 1)
        // Create an iterator where *arg_ == nullptr, *c_ == nullptr.
        return argv_iterator(&argv[argc], nullptr);
    else
    {
        // Construct an iterator pointing to the nullptr arg.
        // Then decrement it to get the null terminator of the last argument,
        // which is the actual end.
        argv_iterator one_past_end(&argv[argc], nullptr);
        --one_past_end;
        return one_past_end;
    }
}
} // namespace lexy

namespace lexy
{
template <typename Encoding = default_encoding>
class argv_input
{
public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;
    static_assert(
        std::is_same_v<char_type, char> || Encoding::template is_secondary_char_type<char>,
        "invalid encoding for argv");

    using iterator = argv_iterator;

    //=== constructors ===//
    constexpr argv_input() = default;

    /// Iterate over the range of command-line arguments.
    constexpr argv_input(argv_iterator begin, argv_iterator end) noexcept : _begin(begin), _end(end)
    {}

    /// Iterate over the command-line arguments.
    constexpr argv_input(int argc, char* argv[]) noexcept
    : _begin(argv_begin(argc, argv)), _end(argv_end(argc, argv))
    {}

    //=== reader ===//
    constexpr auto reader() const& noexcept
    {
        return _detail::range_reader<encoding, iterator>(_begin, _end);
    }

private:
    argv_iterator _begin, _end;
};

argv_input(int argc, char* argv[])->argv_input<>;
} // namespace lexy

namespace lexy
{
template <typename Encoding = default_encoding>
using argv_lexeme = lexeme_for<argv_input<Encoding>>;

template <typename Tag, typename Encoding = default_encoding>
using argv_error = error_for<argv_input<Encoding>, Tag>;

template <typename Production, typename Encoding = default_encoding>
using argv_error_context = error_context<Production, argv_input<Encoding>>;
} // namespace lexy

namespace lexyd
{
struct _argvsep : atom_base<_argvsep>
{
    template <typename Encoding>
    using _argv_reader = lexy::_detail::range_reader<Encoding, lexy::argv_iterator>;

    template <typename Encoding>
    LEXY_DSL_FUNC bool match(_argv_reader<Encoding>& reader)
    {
        if (reader.peek() != Encoding::to_int_type('\0'))
            return false;
        reader.bump();
        return true;
    }
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader&)
    {
        return false;
    }

    template <typename Reader>
    LEXY_DSL_FUNC auto error(const Reader&, typename Reader::iterator pos)
    {
        return lexy::make_error<Reader, lexy::expected_char_class>(pos, "argv-separator");
    }

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Matches the separator between arguments of an argv_input.
constexpr auto argv_separator = _argvsep{};
} // namespace lexyd

#endif // LEXY_INPUT_ARGV_INPUT_HPP_INCLUDED

