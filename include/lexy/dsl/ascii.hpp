// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ASCII_HPP_INCLUDED
#define LEXY_DSL_ASCII_HPP_INCLUDED

#include <lexy/_detail/ascii_table.hpp>
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd::ascii
{
template <typename Derived>
struct _ascii : token_base<Derived>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            if (!Derived::template ascii_match<typename Reader::encoding>(reader.peek()))
                return false;

            reader.bump();
            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(),
                                                                      Derived::ascii_name());
            context.on(_ev::error{}, err);
        }
    };
};

//=== control ===//
struct _control : _ascii<_control>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.control";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::control>(i);
    }
};
inline constexpr auto control = _control{};

//=== whitespace ===//
struct _blank : _ascii<_blank>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.blank";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return i == lexy::_char_to_int_type<Encoding>(' ')
               || i == lexy::_char_to_int_type<Encoding>('\t');
    }
};
inline constexpr auto blank = _blank{};

struct _newline : _ascii<_newline>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.newline";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return i == lexy::_char_to_int_type<Encoding>('\n')
               || i == lexy::_char_to_int_type<Encoding>('\r');
    }
};
inline constexpr auto newline = _newline{};

struct _other_space : _ascii<_other_space>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.other-space";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return i == lexy::_char_to_int_type<Encoding>('\f')
               || i == lexy::_char_to_int_type<Encoding>('\v');
    }
};
inline constexpr auto other_space = _other_space{};

struct _space : _ascii<_space>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.space";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::space>(i);
    }
};
inline constexpr auto space = _space{};

//=== alpha ===//
struct _lower : _ascii<_lower>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.lower";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('a') <= i
               && i <= lexy::_char_to_int_type<Encoding>('z');
    }
};
inline constexpr auto lower = _lower{};

struct _upper : _ascii<_upper>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.upper";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('A') <= i
               && i <= lexy::_char_to_int_type<Encoding>('Z');
    }
};
inline constexpr auto upper = _upper{};

struct _alpha : _ascii<_alpha>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.alpha";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::alpha>(i);
    }
};
inline constexpr auto alpha = _alpha{};

struct _alphau : _ascii<_alphau>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.alpha-underscore";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::alpha_underscore>(i);
    }
};
inline constexpr auto alpha_underscore = _alphau{};

//=== digit ===//
struct _digit : _ascii<_digit>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.digit";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::digit>(i);
    }
};
inline constexpr auto digit = _digit{};

struct _alnum : _ascii<_alnum>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.alpha-digit";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::alpha, ascii_table_t::digit>(i);
    }
};
inline constexpr auto alnum       = _alnum{};
inline constexpr auto alpha_digit = _alnum{};

struct _word : _ascii<_word>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.word";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table
            .contains<Encoding, ascii_table_t::alpha_underscore, ascii_table_t::digit>(i);
    }
};
inline constexpr auto word                   = _word{};
inline constexpr auto alpha_digit_underscore = _word{};

//=== punct ===//
struct _punct : _ascii<_punct>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.punct";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::punct>(i);
    }
};
inline constexpr auto punct = _punct{};

//=== categories ===//
struct _graph : _ascii<_graph>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.graph";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('\x21') <= i
               && i <= lexy::_char_to_int_type<Encoding>('\x7E');
    }
};
inline constexpr auto graph = _graph{};

struct _print : _ascii<_print>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.print";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('\x20') <= i
               && i <= lexy::_char_to_int_type<Encoding>('\x7E');
    }
};
inline constexpr auto print = _print{};

struct _char : _ascii<_char>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('\x00') <= i
               && i <= lexy::_char_to_int_type<Encoding>('\x7F');
    }
};
inline constexpr auto character = _char{};
} // namespace lexyd::ascii

namespace lexyd::ascii
{
template <char... C>
struct _alt : token_base<_alt<C...>>
{
    static_assert(sizeof...(C) > 0);

    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            auto cur = reader.peek();
            if (((cur != lexy::_char_to_int_type<typename Reader::encoding>(C)) && ...))
                return false;

            reader.bump();
            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            constexpr auto str = lexy::_detail::type_string<char, C...>::template c_str<char>;

            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), str);
            context.on(_ev::error{}, err);
        }
    };
};

template <typename CharT, CharT... C>
struct _one_of
{
    static_assert((std::is_same_v<CharT, char> && ... && lexy::_is_ascii(C)),
                  "only ASCII characters are supported");

    using rule = _alt<C...>;
};

#if LEXY_HAS_NTTP
/// Matches one of the ASCII characters.
template <lexy::_detail::string_literal Str>
constexpr auto one_of = typename lexy::_detail::to_type_string<_one_of, Str>::rule{};
#endif

#define LEXY_ASCII_ONE_OF(Str)                                                                     \
    LEXY_NTTP_STRING(::lexyd::ascii::_one_of, Str)::rule {}
} // namespace lexyd::ascii

#endif // LEXY_DSL_ASCII_HPP_INCLUDED

