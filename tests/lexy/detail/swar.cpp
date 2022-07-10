// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/swar.hpp>

#include <doctest/doctest.h>

TEST_CASE("swar_fill")
{
    REQUIRE(sizeof(lexy::_detail::swar_int) == 8);
    CHECK(lexy::_detail::swar_fill(char(0)) == 0);
    CHECK(lexy::_detail::swar_fill(char(0x30)) == 0x30303030'30303030);
    CHECK(lexy::_detail::swar_fill(char(0x80)) == 0x80808080'80808080);
    CHECK(lexy::_detail::swar_fill(char(0xAA)) == 0xAAAAAAAA'AAAAAAAA);
    CHECK(lexy::_detail::swar_fill(char16_t(0x30)) == 0x00300030'00300030);
    CHECK(lexy::_detail::swar_fill(char16_t(0xAA)) == 0x00AA00AA'00AA00AA);
}

