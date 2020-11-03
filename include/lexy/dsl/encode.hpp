// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ENCODE_HPP_INCLUDED
#define LEXY_DSL_ENCODE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/bom.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexyd
{
template <typename Reader, typename Encoding, lexy::encoding_endianness Endianness>
struct _encoded_reader // Single byte encodings
{
    static_assert(sizeof(typename Encoding::char_type) == 1);

    using encoding         = Encoding;
    using char_type        = typename encoding::char_type;
    using iterator         = typename Reader::iterator;
    using canonical_reader = _encoded_reader<Reader, Encoding, Endianness>;

    constexpr bool eof() const
    {
        return _reader.eof();
    }

    constexpr auto peek() const
    {
        if (_reader.eof())
            return encoding::eof();
        else
            return encoding::to_int_type(static_cast<char_type>(*_reader.cur()));
    }

    constexpr void bump()
    {
        _reader.bump();
    }

    constexpr iterator cur() const
    {
        return _reader.cur();
    }

    Reader& _reader;
};
template <typename Reader, lexy::encoding_endianness Endianness>
struct _encoded_reader<Reader, lexy::utf16_encoding, Endianness>
{
    using encoding         = lexy::utf16_encoding;
    using char_type        = typename encoding::char_type;
    using iterator         = typename Reader::iterator;
    using canonical_reader = _encoded_reader<Reader, lexy::utf16_encoding, Endianness>;

    constexpr bool eof() const
    {
        return peek() == eof();
    }

    constexpr auto peek() const
    {
        auto copy = _reader;
        if (copy.eof())
            return encoding::eof();
        auto first = static_cast<char_type>(*copy.cur());

        copy.bump();
        if (copy.eof())
            return encoding::eof();
        auto second = static_cast<char_type>(*copy.cur());

        if constexpr (Endianness == lexy::encoding_endianness::little)
            return encoding::to_int_type(static_cast<char_type>((second << 8) | first));
        else
            return encoding::to_int_type(static_cast<char_type>((first << 8) | second));
    }

    constexpr void bump()
    {
        _reader.bump();
        _reader.bump();
    }

    constexpr iterator cur() const
    {
        return _reader.cur();
    }

    Reader& _reader;
};
template <typename Reader, lexy::encoding_endianness Endianness>
struct _encoded_reader<Reader, lexy::utf32_encoding, Endianness>
{
    using encoding         = lexy::utf32_encoding;
    using char_type        = typename encoding::char_type;
    using iterator         = typename Reader::iterator;
    using canonical_reader = _encoded_reader<Reader, lexy::utf32_encoding, Endianness>;

    constexpr bool eof() const
    {
        return peek() == eof();
    }

    constexpr auto peek() const
    {
        auto copy = _reader;
        if (copy.eof())
            return encoding::eof();
        auto first = static_cast<char_type>(*copy.cur());

        copy.bump();
        if (copy.eof())
            return encoding::eof();
        auto second = static_cast<char_type>(*copy.cur());

        copy.bump();
        if (copy.eof())
            return encoding::eof();
        auto third = static_cast<char_type>(*copy.cur());

        copy.bump();
        if (copy.eof())
            return encoding::eof();
        auto fourth = static_cast<char_type>(*copy.cur());

        if constexpr (Endianness == lexy::encoding_endianness::little)
        {
            auto c = (fourth << 24) | (third << 16) | (second << 8) | first;
            return encoding::to_int_type(static_cast<char_type>(c));
        }
        else
        {
            auto c = (first << 24) | (second << 16) | (third << 8) | fourth;
            return encoding::to_int_type(static_cast<char_type>(c));
        }
    }

    constexpr void bump()
    {
        _reader.bump();
        _reader.bump();
        _reader.bump();
        _reader.bump();
    }

    constexpr iterator cur() const
    {
        return _reader.cur();
    }

    Reader& _reader;
};

template <typename Encoding, lexy::encoding_endianness Endianness>
struct _encode_begin : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            using old_encoding = typename Reader::encoding;
            static_assert((std::is_same_v<old_encoding, lexy::default_encoding>)
                              || (std::is_same_v<old_encoding, lexy::raw_encoding>),
                          "cannot re-encode input");

            auto encoded_reader = _encoded_reader<Reader, Encoding, Endianness>{reader};
            return NextParser::parse(handler, encoded_reader, LEXY_FWD(args)...);
        }
    };
};

struct _encode_end : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename Encoding,
                  lexy::encoding_endianness Endianness, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler&                                       handler,
                                 _encoded_reader<Reader, Encoding, Endianness>& reader,
                                 Args&&... args) -> typename Handler::result_type
        {
            return NextParser::parse(handler, reader._reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Encoding, lexy::encoding_endianness Endianness>
struct _encode
{
    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule rule) const
    {
        if constexpr (Endianness == lexy::encoding_endianness::bom)
        {
            if constexpr (sizeof(typename Encoding::char_type) == 1)
            {
                // The Endianness doesn't matter, just parse a BOM.
                return if_(bom<Encoding, lexy::encoding_endianness::little>)
                       + _encode<Encoding, lexy::encoding_endianness::little>{}(rule);
            }
            else
            {
                auto encode_little = _encode<Encoding, lexy::encoding_endianness::little>{}(rule);
                auto encode_big    = _encode<Encoding, lexy::encoding_endianness::big>{}(rule);

                auto little   = bom<Encoding, lexy::encoding_endianness::little> >> encode_little;
                auto big      = bom<Encoding, lexy::encoding_endianness::big> >> encode_big;
                auto fallback = else_ >> encode_big;

                return little | big | fallback;
            }
        }
        else
            return _encode_begin<Encoding, Endianness>{} + rule + _encode_end{};
    }
};

/// Matches the rule using the specified encoding.
template <typename Encoding, lexy::encoding_endianness Endianness = lexy::encoding_endianness::bom>
constexpr auto encode = _encode<Encoding, Endianness>{};
} // namespace lexyd

#endif // LEXY_DSL_ENCODE_HPP_INCLUDED

