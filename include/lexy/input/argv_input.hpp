// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_ARGV_INPUT_HPP_INCLUDED
#define LEXY_INPUT_ARGV_INPUT_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
class argv_iterator;

/// A sentinel for the command-line arguments.
class argv_sentinel : public _detail::sentinel_base<argv_sentinel, argv_iterator>
{};

/// An iterator over the command-line arguments.
class argv_iterator : public _detail::bidirectional_iterator_base<argv_iterator, const char>
{
public:
    constexpr argv_iterator() noexcept : _arg(nullptr), _c(nullptr) {}

    constexpr const char& deref() const noexcept
    {
        return *_c;
    }

    constexpr void increment() noexcept
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
    }
    constexpr void decrement() noexcept
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
    }

    constexpr bool equal(argv_iterator rhs) const noexcept
    {
        return _arg == rhs._arg && _c == rhs._c;
    }
    constexpr bool is_end() const noexcept
    {
        // We're at the end if we're at the last character of the last argument,
        // or have an empty argv range.
        return _c == nullptr || (*_c == '\0' && _arg[1] == nullptr);
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
        std::is_same_v<char_type, char> || Encoding::template is_secondary_char_type<char>(),
        "invalid encoding for argv");

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
        return _detail::range_reader<encoding, argv_iterator>(_begin, _end);
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
struct _argvsep : token_base<_argvsep>
{
    template <typename Encoding>
    using _argv_reader = lexy::_detail::range_reader<Encoding, lexy::argv_iterator>;

    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1,
        };

        template <typename Encoding>
        static constexpr error_code match(_argv_reader<Encoding>& reader)
        {
            if (reader.peek() != lexy::_char_to_int_type<Encoding>('\0'))
                return error_code::error;
            reader.bump();
            return error_code();
        }
        template <typename Reader>
        static constexpr error_code match(Reader&)
        {
            return error_code::error;
        }
    };

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, lexy::expected_char_class>(pos, "argv-separator");
        context.on(_ev::error{}, err);
    }
};

/// Matches the separator between arguments of an argv_input.
constexpr auto argv_separator = _argvsep{};
} // namespace lexyd

#endif // LEXY_INPUT_ARGV_INPUT_HPP_INCLUDED

