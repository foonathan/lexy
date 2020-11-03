// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ERROR_HPP_INCLUDED
#define LEXY_ERROR_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/type_name.hpp>
#include <lexy/input/base.hpp>

namespace lexy
{
/// Generic failure.
template <typename Reader, typename Tag>
class error
{
    static_assert(is_canonical_reader<Reader>);

public:
    constexpr explicit error(typename Reader::iterator pos) noexcept : _pos(pos), _end(pos) {}
    constexpr explicit error(typename Reader::iterator begin,
                             typename Reader::iterator end) noexcept
    : _pos(begin), _end(end)
    {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr _detail::string_view message() const noexcept
    {
        return _detail::type_name<Tag>();
    }

    constexpr auto begin() const noexcept
    {
        return _pos;
    }
    constexpr auto end() const noexcept
    {
        return _end;
    }

private:
    typename Reader::iterator _pos;
    typename Reader::iterator _end;
};

/// Expected the literal character sequence.
struct expected_literal
{};
template <typename Reader>
class error<Reader, expected_literal>
{
    static_assert(is_canonical_reader<Reader>);

public:
    constexpr explicit error(typename Reader::iterator                              pos,
                             _detail::basic_string_view<typename Reader::char_type> str,
                             std::size_t                                            index) noexcept
    : _pos(pos), _str(str), _idx(index)
    {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr auto string() const noexcept
    {
        return _str;
    }

    constexpr std::size_t index() const noexcept
    {
        return _idx;
    }

    constexpr auto character() const noexcept
    {
        return _str[_idx];
    }

private:
    typename Reader::iterator                              _pos;
    _detail::basic_string_view<typename Reader::char_type> _str;
    std::size_t                                            _idx;
};

/// Expected a character of the specified character class.
struct expected_char_class
{};
template <typename Reader>
class error<Reader, expected_char_class>
{
    static_assert(is_canonical_reader<Reader>);

public:
    constexpr explicit error(typename Reader::iterator pos, const char* name) noexcept
    : _pos(pos), _name(name)
    {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr _detail::string_view character_class() const noexcept
    {
        return _name;
    }

private:
    typename Reader::iterator _pos;
    const char*               _name;
};

template <typename Input, typename Tag>
using error_for = error<input_reader<Input>, Tag>;

template <typename Reader, typename Tag, typename... Args>
constexpr auto make_error(Args&&... args)
{
    return error<typename Reader::canonical_reader, Tag>(LEXY_FWD(args)...);
}
} // namespace lexy

namespace lexy
{
// Contains information about the context of an error.
template <typename Production, typename Input>
class error_context
{
public:
    constexpr explicit error_context(const Input&                           input,
                                     typename input_reader<Input>::iterator pos) noexcept
    : _input(&input), _pos(pos)
    {}
    constexpr explicit error_context(Production, const Input& input,
                                     typename input_reader<Input>::iterator pos) noexcept
    : error_context(input, pos)
    {}

    // The input.
    constexpr const Input& input() const noexcept
    {
        return *_input;
    }

    // The name of the production where the error occurred.
    static LEXY_CONSTEVAL _detail::string_view production()
    {
        return _detail::type_name<Production>();
    }

    // The starting position of the production.
    constexpr auto position() const noexcept
    {
        return _pos;
    }

private:
    const Input*                           _input;
    typename input_reader<Input>::iterator _pos;
};
} // namespace lexy

#endif // LEXY_ERROR_HPP_INCLUDED
