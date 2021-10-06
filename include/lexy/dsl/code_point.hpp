// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED

#include <lexy/code_point.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexy::_detail
{
enum class cp_error
{
    success,
    eof,
    leads_with_trailing,
    missing_trailing,
    surrogate,
    overlong_sequence,
    out_of_range,
    predicate_failure,
};

template <typename Reader>
constexpr lexy::code_point parse_code_point(cp_error& ec, Reader& reader)
{
    if constexpr (std::is_same_v<typename Reader::encoding, lexy::ascii_encoding>)
    {
        if (reader.peek() == Reader::encoding::eof())
        {
            ec = cp_error::eof;
            return {};
        }

        auto cur = reader.peek();
        reader.bump();

        auto cp = lexy::code_point(static_cast<char32_t>(cur));
        if (!cp.is_ascii())
            ec = cp_error::out_of_range;
        return cp;
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf8_encoding>)
    {
        constexpr auto payload_lead1 = 0b0111'1111;
        constexpr auto payload_lead2 = 0b0001'1111;
        constexpr auto payload_lead3 = 0b0000'1111;
        constexpr auto payload_lead4 = 0b0000'0111;
        constexpr auto payload_cont  = 0b0011'1111;

        constexpr auto pattern_lead1 = 0b0 << 7;
        constexpr auto pattern_lead2 = 0b110 << 5;
        constexpr auto pattern_lead3 = 0b1110 << 4;
        constexpr auto pattern_lead4 = 0b11110 << 3;
        constexpr auto pattern_cont  = 0b10 << 6;

        auto first = reader.peek();
        if ((first & ~payload_lead1) == pattern_lead1)
        {
            // ASCII character.
            reader.bump();
            return lexy::code_point(first);
        }
        else if ((first & ~payload_cont) == pattern_cont)
        {
            ec = cp_error::leads_with_trailing;
            return {};
        }
        else if ((first & ~payload_lead2) == pattern_lead2)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
            {
                ec = cp_error::missing_trailing;
                return {};
            }
            reader.bump();

            auto result = char32_t(first & payload_lead2);
            result <<= 6;
            result |= char32_t(second & payload_cont);

            // C0 and C1 are overlong ASCII.
            if (first == 0xC0 || first == 0xC1)
                ec = cp_error::overlong_sequence;

            return lexy::code_point(result);
        }
        else if ((first & ~payload_lead3) == pattern_lead3)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
            {
                ec = cp_error::missing_trailing;
                return {};
            }
            reader.bump();

            auto third = reader.peek();
            if ((third & ~payload_cont) != pattern_cont)
            {
                ec = cp_error::missing_trailing;
                return {};
            }
            reader.bump();

            auto result = char32_t(first & payload_lead3);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);

            auto cp = lexy::code_point(result);
            if (cp.is_surrogate())
                ec = cp_error::surrogate;
            else if (first == 0xE0 && second < 0xA0)
                ec = cp_error::overlong_sequence;
            return cp;
        }
        else if ((first & ~payload_lead4) == pattern_lead4)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
            {
                ec = cp_error::missing_trailing;
                return {};
            }
            reader.bump();

            auto third = reader.peek();
            if ((third & ~payload_cont) != pattern_cont)
            {
                ec = cp_error::missing_trailing;
                return {};
            }
            reader.bump();

            auto fourth = reader.peek();
            if ((fourth & ~payload_cont) != pattern_cont)
            {
                ec = cp_error::missing_trailing;
                return {};
            }
            reader.bump();

            auto result = char32_t(first & payload_lead4);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);
            result <<= 6;
            result |= char32_t(fourth & payload_cont);

            auto cp = lexy::code_point(result);
            if (!cp.is_valid())
                ec = cp_error::out_of_range;
            else if (first == 0xF0 && second < 0x90)
                ec = cp_error::overlong_sequence;
            return cp;
        }
        else // FE or FF
        {
            ec = cp_error::eof;
            return {};
        }
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf16_encoding>)
    {
        constexpr auto payload1 = 0b0000'0011'1111'1111;
        constexpr auto payload2 = payload1;

        constexpr auto pattern1 = 0b110110 << 10;
        constexpr auto pattern2 = 0b110111 << 10;

        if (reader.peek() == Reader::encoding::eof())
        {
            ec = cp_error::eof;
            return {};
        }

        auto first = char16_t(reader.peek());
        if ((first & ~payload1) == pattern1)
        {
            reader.bump();
            if (reader.peek() == Reader::encoding::eof())
            {
                ec = cp_error::missing_trailing;
                return {};
            }

            auto second = char16_t(reader.peek());
            if ((second & ~payload2) != pattern2)
            {
                ec = cp_error::missing_trailing;
                return {};
            }
            reader.bump();

            // We've got a valid code point.
            auto result = char32_t(first & payload1);
            result <<= 10;
            result |= char32_t(second & payload2);
            result |= 0x10000;
            return lexy::code_point(result);
        }
        else if ((first & ~payload2) == pattern2)
        {
            ec = cp_error::leads_with_trailing;
            return {};
        }
        else
        {
            // Single code unit code point; always valid.
            reader.bump();
            return lexy::code_point(first);
        }
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf32_encoding>)
    {
        if (reader.peek() == Reader::encoding::eof())
        {
            ec = cp_error::eof;
            return {};
        }

        auto cur = reader.peek();
        reader.bump();

        auto cp = lexy::code_point(cur);
        if (!cp.is_valid())
            ec = cp_error::out_of_range;
        else if (cp.is_surrogate())
            ec = cp_error::surrogate;

        return cp;
    }
    else
    {
        static_assert(lexy::_detail::error<typename Reader::encoding>,
                      "no known code point for this encoding");
        return {};
    }
}

template <typename Reader>
constexpr void recover_code_point(Reader& reader, typename Reader::iterator end, cp_error ec)
{
    switch (ec)
    {
    case cp_error::success:
        LEXY_PRECONDITION(false);
        break;
    case cp_error::eof:
        // We don't need to do anything to "recover" from EOF.
        break;

    case cp_error::leads_with_trailing:
        // Invalid code unit, consume to recover.
        LEXY_PRECONDITION(end == reader.position());
        reader.bump();
        break;

    case cp_error::missing_trailing:
    case cp_error::surrogate:
    case cp_error::out_of_range:
    case cp_error::overlong_sequence:
    case cp_error::predicate_failure:
        // Consume all the invalid code units to recover.
        reader.set_position(end);
        break;
    }
}
} // namespace lexy::_detail

namespace lexyd
{
template <typename Predicate>
struct _cp : token_base<_cp<Predicate>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;
        lexy::_detail::cp_error   ec;

        constexpr explicit tp(const Reader& reader)
        : end(reader.position()), ec(lexy::_detail::cp_error::success)
        {}

        constexpr bool try_parse(Reader reader)
        {
            using lexy::_detail::cp_error;

            // Parse one code point.
            [[maybe_unused]] auto cp = lexy::_detail::parse_code_point(ec, reader);
            end                      = reader.position();
            if (ec != cp_error::success)
                return false;

            // Check whether it matches the predicate.
            if constexpr (!std::is_void_v<Predicate>)
            {
                if (!Predicate()(cp))
                {
                    ec = cp_error::predicate_failure;
                    return false;
                }
            }
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            using lexy::_detail::cp_error;

            if (ec == cp_error::predicate_failure)
            {
                constexpr auto name = lexy::_detail::type_name<Predicate>();

                auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), name);
                context.on(_ev::error{}, err);
            }
            else
            {
                constexpr auto name = [] {
                    using encoding = typename Reader::encoding;
                    if constexpr (std::is_same_v<encoding, lexy::ascii_encoding>)
                        return "ASCII.code-point";
                    else if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>)
                        return "UTF-8.code-point";
                    else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
                        return "UTF-16.code-point";
                    else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
                        return "UTF-32.code-point";
                    else
                        return "code-point";
                }();

                auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), name);
                context.on(_ev::error{}, err);
            }
        }
    };

    template <typename P>
    constexpr auto if_() const
    {
        static_assert(std::is_void_v<Predicate>);
        return _cp<P>{};
    }
};

/// Matches a single unicode code point in the current unicode encoding.
constexpr auto code_point = _cp<void>{};
} // namespace lexyd

#endif // LEXY_DSL_CODE_POINT_HPP_INCLUDED

