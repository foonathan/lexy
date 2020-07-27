// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LITERAL_HPP_INCLUDED
#define LEXY_DSL_LITERAL_HPP_INCLUDED

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/_detail/string_view.hpp>
#include <lexy/dsl/base.hpp>

namespace lexy
{
struct expected_literal
{
    template <typename Input>
    class error
    {
    public:
        constexpr explicit error(typename Input::iterator                              pos,
                                 _detail::basic_string_view<typename Input::char_type> str,
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
        typename Input::iterator                              _pos;
        _detail::basic_string_view<typename Input::char_type> _str;
        std::size_t                                           _idx;
    };
};
} // namespace lexy

namespace lexyd
{
template <typename String>
struct _lit : atom_base<_lit<String>>
{
    template <typename Input>
    LEXY_DSL_FUNC bool match(Input& input)
    {
        static_assert(lexy::char_type_compatible_with_input<Input, typename String::char_type>);

        for (auto c : String::get())
        {
            if (input.peek() != Input::encoding::to_int_type(c))
                return false;
            input.bump();
        }

        return true;
    }

    template <typename Input>
    LEXY_DSL_FUNC auto error(const Input& input, typename Input::iterator pos)
    {
        return lexy::expected_literal::error<Input>(pos, String::get(),
                                                    lexy::_detail::range_size(pos, input.cur()));
    }
};

template <auto C>
constexpr auto lit_c = _lit<lexy::_detail::type_char<C>>{};

#if LEXY_HAS_NTTP
/// Matches the literal string.
template <lexy::_detail::string_literal Str>
constexpr auto lit = _lit<lexy::_detail::type_string<Str>>;
#endif

#define LEXY_LIT(Str)                                                                              \
    ::lexyd::_lit<LEXY_NTTP_STRING(Str)> {}
} // namespace lexyd

#endif // LEXY_DSL_LITERAL_HPP_INCLUDED
