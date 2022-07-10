// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_SWAR_HPP_INCLUDED
#define LEXY_DETAIL_SWAR_HPP_INCLUDED

#include <climits>
#include <cstdint>
#include <cstring>
#include <lexy/_detail/config.hpp>
#include <lexy/input/base.hpp>

namespace lexy::_detail
{
using swar_int = std::uintmax_t;

// The number of chars that can fit into one SWAR.
template <typename CharT>
constexpr auto swar_length = [] {
    static_assert(sizeof(CharT) < sizeof(swar_int) && sizeof(swar_int) % sizeof(CharT) == 0);
    return sizeof(swar_int) / sizeof(CharT);
}();

// Returns a swar_int filled with the specific char.
template <typename CharT>
constexpr swar_int swar_fill(CharT _c)
{
    auto c = std::make_unsigned_t<CharT>(_c);

    auto result = swar_int(0);
    for (auto i = 0u; i != swar_length<CharT>; ++i)
    {
        result <<= sizeof(CharT) * CHAR_BIT;
        result |= c;
    }
    return result;
}
} // namespace lexy::_detail

namespace lexy::_detail
{
struct _swar_base
{};
template <typename Reader>
constexpr auto is_swar_reader = std::is_base_of_v<_swar_base, Reader>;

template <typename Derived>
class swar_reader_base : _swar_base
{
public:
    swar_int peek_swar() const
    {
        auto ptr = static_cast<const Derived&>(*this).position();

        swar_int result;
#if LEXY_IS_LITTLE_ENDIAN
        std::memcpy(&result, ptr, sizeof(swar_int));
#else
        using char_type = typename Derived::encoding::char_type;
        auto dst        = reinterpret_cast<char*>(&result);
        auto length     = sizeof(swar_int) / sizeof(char_type);
        for (auto i = 0u; i != length; ++i)
        {
            std::memcpy(dst + i, ptr + length - i - 1, sizeof(char_type));
        }
#endif
        return result;
    }

    void bump_swar()
    {
        auto ptr = static_cast<Derived&>(*this).position();
        ptr += sizeof(swar_int);
        static_cast<Derived&>(*this).set_position(ptr);
    }
};

constexpr std::size_t round_size_for_swar(std::size_t size_in_bytes)
{
    // We round up to the next multiple.
    if (auto remainder = size_in_bytes % sizeof(swar_int); remainder > 0)
        size_in_bytes += sizeof(swar_int) - remainder;
    // Then add one extra space of padding on top.
    size_in_bytes += sizeof(swar_int);
    return size_in_bytes;
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_SWAR_HPP_INCLUDED

