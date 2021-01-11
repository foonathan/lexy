// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include "../../examples/json.cpp"

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>
#include <lexy/match.hpp>

// We copy the conformance tests from https://github.com/miloyip/nativejson-benchmark.

//=== pass/fail validation ===//
namespace
{
void fail(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    REQUIRE(!lexy::match<grammar::json>(input));
}

void pass(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    REQUIRE(lexy::match<grammar::json>(input));
}
} // namespace

TEST_CASE("fail02")
{
    fail(R"(["Unclosed array")");
}

TEST_CASE("fail03")
{
    fail(R"({unquoted_key: "keys must be quoted"})");
}

TEST_CASE("fail04")
{
    fail(R"(["extra comma",])");
}

TEST_CASE("fail05")
{
    fail(R"(["double extra comma",,])");
}

TEST_CASE("fail06")
{
    fail(R"([   , "<-- missing value"])");
}

TEST_CASE("fail07")
{
    fail(R"(["Comma after the close"],)");
}

TEST_CASE("fail08")
{
    fail(R"(["Extra close"]])");
}

TEST_CASE("fail09")
{
    fail(R"({"Extra comma": true,})");
}

TEST_CASE("fail10")
{
    fail(R"({"Extra value after close": true} "misplaced quoted value")");
}

TEST_CASE("fail11")
{
    fail(R"({"Illegal expression": 1 + 2})");
}

TEST_CASE("fail12")
{
    fail(R"({"Illegal invocation": alert()})");
}

TEST_CASE("fail13")
{
    fail(R"({"Numbers cannot have leading zeroes": 013})");
}

TEST_CASE("fail14")
{
    fail(R"({"Numbers cannot be hex": 0x14})");
}

TEST_CASE("fail15")
{
    fail(R"(["Illegal backslash escape: \x15"])");
}

TEST_CASE("fail16")
{
    fail(R"([\naked])");
}

TEST_CASE("fail17")
{
    fail(R"(["Illegal backslash escape: \017"])");
}

TEST_CASE("fail19")
{
    fail(R"({"Missing colon" null})");
}

TEST_CASE("fail20")
{
    fail(R"({"Double colon":: null})");
}

TEST_CASE("fail21")
{
    fail(R"({"Comma instead of colon", null})");
}

TEST_CASE("fail22")
{
    fail(R"(["Colon instead of comma": false])");
}

TEST_CASE("fail23")
{
    fail(R"(["Bad value", truth])");
}

TEST_CASE("fail24")
{
    fail(R"(['single quote'])");
}

TEST_CASE("fail25")
{
    fail("[\"\ttab\tcharacter\tin\tstring\t\"]");
}

TEST_CASE("fail27")
{
    fail("[\"\\\ttab\\\tcharacter\\\tin\\\tstring\\\t\"]");
}

TEST_CASE("fail27")
{
    fail(R"(["line
break"])");
}

TEST_CASE("fail28")
{
    fail(R"(["line\
break"])");
}

TEST_CASE("fail29")
{
    fail(R"([0e])");
}

TEST_CASE("fail30")
{
    fail(R"([0e+])");
}

TEST_CASE("fail31")
{
    fail(R"([0e+-1])");
}

TEST_CASE("fail32")
{
    fail(R"({"Comma instead if closing brace": true,)");
}

TEST_CASE("fail33")
{
    fail(R"(["mismatch"})");
}

TEST_CASE("pass01")
{
    pass(R"([
    "JSON Test Pattern pass1",
    {"object with 1 member":["array with 1 element"]},
    {},
    [],
    -42,
    true,
    false,
    null,
    {
        "integer": 1234567890,
        "real": -9876.543210,
        "e": 0.123456789e-12,
        "E": 1.234567890E+34,
        "":  23456789012E66,
        "zero": 0,
        "one": 1,
        "space": " ",
        "quote": "\"",
        "backslash": "\\",
        "controls": "\b\f\n\r\t",
        "slash": "/ & \/",
        "alpha": "abcdefghijklmnopqrstuvwyz",
        "ALPHA": "ABCDEFGHIJKLMNOPQRSTUVWYZ",
        "digit": "0123456789",
        "0123456789": "digit",
        "special": "`1~!@#$%^&*()_+-={':[,]}|;.</>?",
        "hex": "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A",
        "true": true,
        "false": false,
        "null": null,
        "array":[  ],
        "object":{  },
        "address": "50 St. James Street",
        "url": "http://www.JSON.org/",
        "comment": "// /* <!-- --",
        "# -- --> */": " ",
        " s p a c e d " :[1,2 , 3

,

4 , 5        ,          6           ,7        ],"compact":[1,2,3,4,5,6,7],
        "jsontext": "{\"object with 1 member\":[\"array with 1 element\"]}",
        "quotes": "&#34; \u0022 %22 0x22 034 &#x22;",
        "\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"
: "A key can be any string"
    },
    0.5 ,98.6
,
99.44
,

1066,
1e1,
0.1e1,
1e-1,
1e00,2e+00,2e-00
,"rosebud"])");
}

TEST_CASE("pass02")
{
    pass(R"([[[[[[[[[[[[[[[[[[["Not too deep"]]]]]]]]]]]]]]]]]]])");
}

TEST_CASE("pass03")
{
    // Note: outdated comment.
    pass(R"({
    "JSON Test Pattern pass3": {
        "The outermost value": "must be an object or array.",
        "In this test": "It is an object."
    }
})");
}

//=== parse double ===//
// Not necessary, we don't create doubles.

//=== parse string ===//
namespace
{
auto parse_string(const char* str)
{
    auto input  = lexy::zstring_input<lexy::utf8_encoding>(str);
    auto result = lexy::parse<grammar::json>(input, lexy::noop);
    REQUIRE(result);
    return std::get<ast::json_string>(result.value().v);
}
} // namespace

TEST_CASE("parse_string")
{
    CHECK(parse_string(R"("")") == "");
    CHECK(parse_string(R"("Hello")") == "Hello");
    CHECK(parse_string(R"("Hello\nWorld")") == "Hello\nWorld");
    CHECK(parse_string(R"("Hello\u0000World")") == std::string("Hello\0World", 11));
    CHECK(parse_string(R"("\"\\\/\b\f\n\r\t")") == "\"\\/\b\f\n\r\t");
    CHECK(parse_string(R"("\u0024")") == "\u0024");
    CHECK(parse_string(R"("\u00A2")") == "\u00A2");
    CHECK(parse_string(R"("\u20AC")") == "\u20AC");

    // I'm not sure how the final string test is supposed to work.
}

//=== roundtrip ===//
namespace
{
auto parse(const char* str)
{
    auto input  = lexy::zstring_input<lexy::utf8_encoding>(str);
    auto result = lexy::parse<grammar::json>(input, lexy::noop);
    REQUIRE(result);
    return result.value();
}

template <typename T>
auto& get(const ast::json_value& value)
{
    auto ptr = std::get_if<T>(&value.v);
    REQUIRE(ptr);
    return *ptr;
}

auto get_number(const ast::json_value& value)
{
    auto number = get<ast::json_number>(value);
    auto result = std::to_string(number.integer);
    if (number.fraction)
        result += "." + *number.fraction;
    if (number.exponent)
        result += "e" + std::to_string(*number.exponent);
    return result;
}

using namespace ast;
} // namespace

TEST_CASE("roundtrip01")
{
    auto result = parse(R"([null])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    get<json_null>(array[0]);
}

TEST_CASE("roundtrip02")
{
    auto result = parse(R"([true])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get<json_bool>(array[0]) == true);
}

TEST_CASE("roundtrip03")
{
    auto result = parse(R"([false])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get<json_bool>(array[0]) == false);
}

TEST_CASE("roundtrip04")
{
    auto result = parse(R"([0])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "0");
}

TEST_CASE("roundtrip05")
{
    auto result = parse(R"(["foo"])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get<json_string>(array[0]) == "foo");
}

TEST_CASE("roundtrip06")
{
    auto result = parse(R"([])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 0);
}

TEST_CASE("roundtrip07")
{
    auto result = parse(R"({})");
    auto object = get<json_object>(result);
    REQUIRE(object.size() == 0);
}

TEST_CASE("roundtrip08")
{
    auto result = parse(R"([0, 1])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 2);
    REQUIRE(get_number(array[0]) == "0");
    REQUIRE(get_number(array[1]) == "1");
}

TEST_CASE("roundtrip09")
{
    auto result = parse(R"({"foo":"bar"})");
    auto object = get<json_object>(result);
    REQUIRE(object.size() == 1);
    REQUIRE(get<json_string>(object.at("foo")) == "bar");
}

TEST_CASE("roundtrip10")
{
    auto result = parse(R"({"a":null,"foo":"bar"})");
    auto object = get<json_object>(result);
    REQUIRE(object.size() == 2);
    get<json_null>(object.at("a"));
    REQUIRE(get<json_string>(object.at("foo")) == "bar");
}

TEST_CASE("roundtrip11")
{
    auto result = parse(R"([-1])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "-1");
}

TEST_CASE("roundtrip12")
{
    auto result = parse(R"([-2147483648])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "-2147483648");
}

TEST_CASE("roundtrip13")
{
    auto result = parse(R"([-1234567890123456789])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "-1234567890123456789");
}

// 14 has an integer overflow.

TEST_CASE("roundtrip15")
{
    auto result = parse(R"([1])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "1");
}

TEST_CASE("roundtrip16")
{
    auto result = parse(R"([2147483647])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "2147483647");
}

TEST_CASE("roundtrip17")
{
    auto result = parse(R"([4294967295])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "4294967295");
}

TEST_CASE("roundtrip18")
{
    auto result = parse(R"([1234567890123456789])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "1234567890123456789");
}

TEST_CASE("roundtrip19")
{
    auto result = parse(R"([9223372036854775807])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "9223372036854775807");
}

TEST_CASE("roundtrip20")
{
    auto result = parse(R"([0.0])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "0.0");
}

TEST_CASE("roundtrip21")
{
    // We're losing the sign of negative zero here, but that's okay.

    auto result = parse(R"([-0.0])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "0.0");
}

TEST_CASE("roundtrip22")
{
    auto result = parse(R"([1.2345])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "1.2345");
}

TEST_CASE("roundtrip23")
{
    auto result = parse(R"([-1.2345])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "-1.2345");
}

TEST_CASE("roundtrip24")
{
    auto result = parse(R"([5e-324])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "5e-324");
}

TEST_CASE("roundtrip25")
{
    auto result = parse(R"([2.225073858507201e-308])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "2.225073858507201e-308");
}

TEST_CASE("roundtrip26")
{
    auto result = parse(R"([2.2250738585072014e-308])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "2.2250738585072014e-308");
}

TEST_CASE("roundtrip27")
{
    auto result = parse(R"([1.7976931348623157e308])");
    auto array  = get<json_array>(result);
    REQUIRE(array.size() == 1);
    REQUIRE(get_number(array[0]) == "1.7976931348623157e308");
}

