// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_CODE_POINT_HPP_INCLUDED
#define LEXY_ENGINE_CODE_POINT_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/code_point.hpp>
#include <lexy/engine/base.hpp>

namespace lexy
{
enum class _cp_error_code
{
    eof = 1,
    leads_with_trailing,
    missing_trailing,
    surrogate,
    overlong_sequence,
    out_of_range,
};

/// Parses an ASCII code point.
struct engine_cp_ascii : engine_matcher_base, engine_parser_base
{
    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        static_assert(std::is_same_v<typename Reader::encoding, ascii_encoding>);

        if (reader.eof())
        {
            ec = error_code::eof;
            return code_point();
        }

        auto cur = reader.peek();
        reader.bump();

        auto cp = code_point(static_cast<char32_t>(cur));
        if (!cp.is_ascii())
            ec = error_code::out_of_range;
        return cp;
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};
        parse(result, reader);
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader&, error_code ec)
    {
        switch (ec)
        {
        case error_code::eof:
            // We cannot recover after EOF.
            return false;
        case error_code::out_of_range:
            // Invalid code unit, but already consumed.
            return true;

        case error_code::leads_with_trailing:
        case error_code::missing_trailing:
        case error_code::surrogate:
        case error_code::overlong_sequence:
            LEXY_ASSERT(false, "unreachable");
        }
        return false;
    }
};

/// Matches a UTF-8 code point.
struct engine_cp_utf8 : engine_matcher_base, engine_parser_base
{
    static constexpr auto payload_lead1 = 0b0111'1111;
    static constexpr auto payload_lead2 = 0b0001'1111;
    static constexpr auto payload_lead3 = 0b0000'1111;
    static constexpr auto payload_lead4 = 0b0000'0111;
    static constexpr auto payload_cont  = 0b0011'1111;

    static constexpr auto pattern_lead1 = 0b0 << 7;
    static constexpr auto pattern_lead2 = 0b110 << 5;
    static constexpr auto pattern_lead3 = 0b1110 << 4;
    static constexpr auto pattern_lead4 = 0b11110 << 3;
    static constexpr auto pattern_cont  = 0b10 << 6;

    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        static_assert(std::is_same_v<typename Reader::encoding, utf8_encoding>);

        auto first = reader.peek();
        if ((first & ~payload_lead1) == pattern_lead1)
        {
            // ASCII character.
            reader.bump();
            return code_point(first);
        }
        else if ((first & ~payload_cont) == pattern_cont)
        {
            ec = error_code::leads_with_trailing;
            return code_point();
        }
        else if ((first & ~payload_lead2) == pattern_lead2)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto result = char32_t(first & payload_lead2);
            result <<= 6;
            result |= char32_t(second & payload_cont);

            // C0 and C1 are overlong ASCII.
            if (first == 0xC0 || first == 0xC1)
                ec = error_code::overlong_sequence;

            return code_point(result);
        }
        else if ((first & ~payload_lead3) == pattern_lead3)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto third = reader.peek();
            if ((third & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto result = char32_t(first & payload_lead3);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);

            auto cp = code_point(result);
            if (cp.is_surrogate())
                ec = error_code::surrogate;
            else if (first == 0xE0 && second < 0xA0)
                ec = error_code::overlong_sequence;
            return cp;
        }
        else if ((first & ~payload_lead4) == pattern_lead4)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto third = reader.peek();
            if ((third & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto fourth = reader.peek();
            if ((fourth & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto result = char32_t(first & payload_lead4);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);
            result <<= 6;
            result |= char32_t(fourth & payload_cont);

            auto cp = code_point(result);
            if (!cp.is_valid())
                ec = error_code::out_of_range;
            else if (first == 0xF0 && second < 0x90)
                ec = error_code::overlong_sequence;
            return cp;
        }
        else // FE or FF
        {
            ec = error_code::eof;
            return code_point();
        }
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};
        parse(result, reader);
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code ec)
    {
        switch (ec)
        {
        case error_code::eof:
            // We cannot recover after EOF.
            return false;
        case error_code::leads_with_trailing:
            // Invalid code unit, consume to recover.
            reader.bump();
            return true;
        case error_code::missing_trailing:
        case error_code::surrogate:
        case error_code::out_of_range:
        case error_code::overlong_sequence:
            // We've already consumed the invalid unit, recovered.
            return true;
        }

        return false; // unreachable
    }
};

/// Matches a UTF-16 code point.
struct engine_cp_utf16 : engine_matcher_base, engine_parser_base
{
    static constexpr auto payload1 = 0b0000'0011'1111'1111;
    static constexpr auto payload2 = payload1;

    static constexpr auto pattern1 = 0b110110 << 10;
    static constexpr auto pattern2 = 0b110111 << 10;

    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        static_assert(std::is_same_v<typename Reader::encoding, utf16_encoding>);

        if (reader.eof())
        {
            ec = error_code::eof;
            return code_point();
        }

        auto first = char16_t(reader.peek());
        if ((first & ~payload1) == pattern1)
        {
            reader.bump();
            if (reader.eof())
            {
                ec = error_code::missing_trailing;
                return code_point();
            }

            auto second = char16_t(reader.peek());
            if ((second & ~payload2) != pattern2)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            // We've got a valid code point.
            auto result = char32_t(first & payload1);
            result <<= 10;
            result |= char32_t(second & payload2);
            result |= 0x10000;
            return code_point(result);
        }
        else if ((first & ~payload2) == pattern2)
        {
            ec = error_code::leads_with_trailing;
            return code_point();
        }
        else
        {
            // Single code unit code point; always valid.
            reader.bump();
            return code_point(first);
        }
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};
        parse(result, reader);
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code ec)
    {
        switch (ec)
        {
        case error_code::eof:
            // We cannot recover after EOF.
            return false;
        case error_code::leads_with_trailing:
            // Invalid code unit, consume to recover.
            reader.bump();
            return true;
        case error_code::missing_trailing:
            // We've already consumed the invalid unit, recovered.
            return true;

        case error_code::surrogate:
        case error_code::overlong_sequence:
        case error_code::out_of_range:
            LEXY_ASSERT(false, "unreachable");
        }
        return false;
    }
};

/// Matches a UTF-32 code point.
struct engine_cp_utf32 : engine_matcher_base, engine_parser_base
{
    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        static_assert(std::is_same_v<typename Reader::encoding, utf32_encoding>);

        if (reader.eof())
        {
            ec = error_code::eof;
            return code_point();
        }

        auto cur = reader.peek();
        reader.bump();

        auto cp = code_point(cur);
        if (!cp.is_valid())
            ec = error_code::out_of_range;
        else if (cp.is_surrogate())
            ec = error_code::surrogate;

        return cp;
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};
        parse(result, reader);
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader&, error_code ec)
    {
        switch (ec)
        {
        case error_code::eof:
            // We cannot recover after EOF.
            return false;
        case error_code::surrogate:
        case error_code::out_of_range:
            // Invalid code unit, but already consumed.
            return true;

        case error_code::leads_with_trailing:
        case error_code::missing_trailing:
        case error_code::overlong_sequence:
            LEXY_ASSERT(false, "unreachable");
        }
        return false;
    }
};

/// Matches a code point according to the inputs encoding.
struct engine_cp_auto : lexy::engine_matcher_base, lexy::engine_parser_base
{
    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        using encoding = typename Reader::encoding;
        if constexpr (std::is_same_v<encoding, lexy::ascii_encoding>)
            return engine_cp_ascii::parse(ec, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>)
            return engine_cp_utf8::parse(ec, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
            return engine_cp_utf16::parse(ec, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
            return engine_cp_utf32::parse(ec, reader);
        else
            static_assert(lexy::_detail::error<encoding>,
                          "no code point defined for this encoding");
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};

        using encoding = typename Reader::encoding;
        if constexpr (std::is_same_v<encoding, lexy::ascii_encoding>)
            engine_cp_ascii::parse(result, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>)
            engine_cp_utf8::parse(result, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
            engine_cp_utf16::parse(result, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
            engine_cp_utf32::parse(result, reader);
        else
            static_assert(lexy::_detail::error<encoding>,
                          "no code point defined for this encoding");

        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code ec)
    {
        using encoding = typename Reader::encoding;
        if constexpr (std::is_same_v<encoding, lexy::ascii_encoding>)
            return engine_cp_ascii::recover(reader, ec);
        else if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>)
            return engine_cp_utf8::recover(reader, ec);
        else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
            return engine_cp_utf16::recover(reader, ec);
        else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
            return engine_cp_utf32::recover(reader, ec);
        else
            static_assert(lexy::_detail::error<encoding>,
                          "no code point defined for this encoding");
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_CODE_POINT_HPP_INCLUDED

