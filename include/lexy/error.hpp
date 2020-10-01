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
template <typename Tag>
struct failure
{
    template <typename Reader>
    class error
    {
    public:
        constexpr explicit error(typename Reader::iterator pos) noexcept : _pos(pos) {}

        constexpr auto position() const noexcept
        {
            return _pos;
        }

        constexpr _detail::string_view name() const noexcept
        {
            return _detail::type_name<Tag>();
        }

    private:
        typename Reader::iterator _pos;
    };
};

/// Expected the literal character sequence.
struct expected_literal
{
    template <typename Reader>
    class error
    {
    public:
        constexpr explicit error(typename Reader::iterator                              pos,
                                 _detail::basic_string_view<typename Reader::char_type> str,
                                 std::size_t index) noexcept
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
};

/// Expected a character of the specified character class.
struct expected_char_class
{
    template <typename Reader>
    class error
    {
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
};
} // namespace lexy

namespace lexy
{
template <typename Production>
LEXY_CONSTEVAL _detail::string_view production_name(Production)
{
    return _detail::type_name<Production>();
}
} // namespace lexy

#endif // LEXY_ERROR_HPP_INCLUDED

