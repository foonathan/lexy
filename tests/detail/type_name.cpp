// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/_detail/type_name.hpp>

#include <doctest.h>

namespace
{
struct test_type
{
    static constexpr const char* name = "some type";
};
} // namespace

namespace ns
{
struct test_type
{};
class test_class
{};

namespace inner
{
    struct test_type
    {};
} // namespace inner
} // namespace ns

TEST_CASE("_detail::type_name")
{
    SUBCASE("function")
    {
        struct type
        {
            static LEXY_CONSTEVAL const char* name()
            {
                return "some type";
            }
        };
        CHECK(lexy::_detail::type_name<type>() == "some type");
    }
    SUBCASE("variable")
    {
        CHECK(lexy::_detail::type_name<test_type>() == "some type");
    }
    SUBCASE("default")
    {
        CHECK(lexy::_detail::type_name<int>(0) == "int");

        CHECK(lexy::_detail::type_name<ns::test_type>(0) == "ns::test_type");
        CHECK(lexy::_detail::type_name<ns::test_class>(0) == "ns::test_class");
        CHECK(lexy::_detail::type_name<ns::inner::test_type>(0) == "ns::inner::test_type");

        CHECK(lexy::_detail::type_name<ns::test_type>() == "test_type");
        CHECK(lexy::_detail::type_name<ns::test_class>() == "test_class");
        CHECK(lexy::_detail::type_name<ns::inner::test_type>() == "inner::test_type");

        CHECK(lexy::_detail::type_name<ns::test_type>(2) == "test_type");
        CHECK(lexy::_detail::type_name<ns::test_class>(2) == "test_class");
        CHECK(lexy::_detail::type_name<ns::inner::test_type>(2) == "test_type");
    }
}

