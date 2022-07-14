// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/swar.hpp>

#include <doctest/doctest.h>

using namespace lexy::_detail;

TEST_CASE("swar_fill")
{
    REQUIRE(sizeof(swar_int) == 8);
    CHECK(swar_fill(char(0)) == 0);
    CHECK(swar_fill(char(0x30)) == 0x30303030'30303030);
    CHECK(swar_fill(char(0x80)) == 0x80808080'80808080);
    CHECK(swar_fill(char(0xAA)) == 0xAAAAAAAA'AAAAAAAA);
    CHECK(swar_fill(char16_t(0x30)) == 0x00300030'00300030);
    CHECK(swar_fill(char16_t(0xAA)) == 0x00AA00AA'00AA00AA);
}

TEST_CASE("swar_pack")
{
    REQUIRE(sizeof(swar_int) == 8);

    constexpr auto single_char = swar_pack(char(0x11));
    CHECK(single_char.value == 0x11);
    CHECK(single_char.mask == 0xFF);
    CHECK(single_char.count == 1);

    constexpr auto multiple_chars
        = swar_pack(char(0x00), char(0x11), char(0x22), char(0x33), char(0x44));
    CHECK(multiple_chars.value == 0x4433221100);
    CHECK(multiple_chars.mask == 0xFFFFFFFFFF);
    CHECK(multiple_chars.count == 5);

    constexpr auto full = swar_pack(char32_t(0x11), char32_t(0x22));
    CHECK(full.value == 0x00000022'00000011);
    CHECK(full.mask == 0xFFFFFFFF'FFFFFFFF);
    CHECK(full.count == 2);

    constexpr auto overflow = swar_pack(char32_t(0x11), char32_t(0x22), char32_t(0x33));
    CHECK(overflow.value == 0x00000022'00000011);
    CHECK(overflow.mask == 0xFFFFFFFF'FFFFFFFF);
    CHECK(overflow.count == 2);

    constexpr auto offset
        = swar_pack<2>(char(0x00), char(0x11), char(0x22), char(0x33), char(0x44));
    CHECK(offset.value == 0x443322);
    CHECK(offset.mask == 0xFFFFFF);
    CHECK(offset.count == 3);

    constexpr auto overflow_offset = swar_pack<1>(char32_t(0x11), char32_t(0x22), char32_t(0x33));
    CHECK(overflow_offset.value == 0x00000033'00000022);
    CHECK(overflow_offset.mask == 0xFFFFFFFF'FFFFFFFF);
    CHECK(overflow_offset.count == 2);
}

TEST_CASE("swar_find_difference")
{
    REQUIRE(sizeof(swar_int) == 8);
    constexpr auto a   = swar_pack(char('a')).value;
    constexpr auto A   = swar_pack(char('A')).value;
    constexpr auto abc = swar_pack(char('a'), char('b'), char('c')).value;
    constexpr auto aBc = swar_pack(char('a'), char('B'), char('c')).value;

    CHECK(swar_find_difference<char>(a, a) == 8);
    CHECK(swar_find_difference<char>(a, A) == 0);
    CHECK(swar_find_difference<char>(abc, aBc) == 1);
}

