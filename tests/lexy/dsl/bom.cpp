// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/bom.hpp>

#include "verify.hpp"

TEST_CASE("dsl::bom")
{
    SUBCASE("no bom")
    {
        constexpr auto atom
            = lexy::dsl::bom<lexy::default_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_pattern<decltype(atom)>);

        constexpr auto empty = atom_matches(atom, "");
        CHECK(empty);
        CHECK(empty.count == 0);
    }
    SUBCASE("UTF-8")
    {
        constexpr auto atom
            = lexy::dsl::bom<lexy::utf8_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_pattern<decltype(atom)>);

        constexpr auto empty = atom_matches(atom, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        // CHECK(empty.error.character_class() == "BOM.UTF-8");

        static constexpr char str[] = {char(0xEF), char(0xBB), char(0xBF)};
        constexpr auto        bom   = atom_matches(atom, str, 3);
        CHECK(bom);
        CHECK(bom.count == 3);
    }
    SUBCASE("UTF-16 little")
    {
        constexpr auto atom
            = lexy::dsl::bom<lexy::utf16_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_pattern<decltype(atom)>);

        constexpr auto empty = atom_matches(atom, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        // CHECK(empty.error.character_class() == "BOM.UTF-16-LE");

        static constexpr char str[] = {char(0xFF), char(0xFE)};
        constexpr auto        bom   = atom_matches(atom, str, 2);
        CHECK(bom);
        CHECK(bom.count == 2);
    }
    SUBCASE("UTF-16 big")
    {
        constexpr auto atom = lexy::dsl::bom<lexy::utf16_encoding, lexy::encoding_endianness::big>;
        CHECK(lexy::is_pattern<decltype(atom)>);

        constexpr auto empty = atom_matches(atom, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        // CHECK(empty.error.character_class() == "BOM.UTF-16-BE");

        static constexpr char str[] = {char(0xFE), char(0xFF)};
        constexpr auto        bom   = atom_matches(atom, str, 2);
        CHECK(bom);
        CHECK(bom.count == 2);
    }
    SUBCASE("UTF-32 little")
    {
        constexpr auto atom
            = lexy::dsl::bom<lexy::utf32_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_pattern<decltype(atom)>);

        constexpr auto empty = atom_matches(atom, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        // CHECK(empty.error.character_class() == "BOM.UTF-32-LE");

        static constexpr char str[] = {char(0xFF), char(0xFE), 0, 0};
        constexpr auto        bom   = atom_matches(atom, str, 4);
        CHECK(bom);
        CHECK(bom.count == 4);
    }
    SUBCASE("UTF-32 big")
    {
        constexpr auto atom = lexy::dsl::bom<lexy::utf32_encoding, lexy::encoding_endianness::big>;
        CHECK(lexy::is_pattern<decltype(atom)>);

        constexpr auto empty = atom_matches(atom, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        // CHECK(empty.error.character_class() == "BOM.UTF-32-BE");

        static constexpr char str[] = {0, 0, char(0xFE), char(0xFF)};
        constexpr auto        bom   = atom_matches(atom, str, 4);
        CHECK(bom);
        CHECK(bom.count == 4);
    }
}

