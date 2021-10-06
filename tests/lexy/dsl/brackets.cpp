// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/brackets.hpp>

#include "verify.hpp"
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/option.hpp>

TEST_CASE("dsl::brackets()")
{
    constexpr auto brackets = dsl::brackets(dsl::lit_c<'('>, dsl::lit_c<')'>);

    CHECK(equivalent_rules(brackets.open(), dsl::lit_c<'('>));
    CHECK(equivalent_rules(brackets.close(), dsl::lit_c<')'>));
    CHECK(equivalent_rules(brackets.as_terminator(), dsl::terminator(dsl::lit_c<')'>)));
    CHECK(equivalent_rules(brackets.recovery_rule(), dsl::recover(dsl::lit_c<')'>)));

    CHECK(equivalent_rules(brackets(LEXY_LIT("abc")),
                           brackets.open() >> brackets.as_terminator()(LEXY_LIT("abc"))));
    CHECK(equivalent_rules(brackets.try_(LEXY_LIT("abc")),
                           brackets.open() >> brackets.as_terminator().try_(LEXY_LIT("abc"))));
    CHECK(equivalent_rules(brackets.opt(LEXY_LIT("abc")),
                           brackets.open() >> brackets.as_terminator().opt(LEXY_LIT("abc"))));
    CHECK(equivalent_rules(brackets.list(LEXY_LIT("abc")),
                           brackets.open() >> brackets.as_terminator().list(LEXY_LIT("abc"))));
    CHECK(equivalent_rules(brackets.list(LEXY_LIT("abc"), dsl::sep(LEXY_LIT(","))),
                           brackets.open()
                               >> brackets.as_terminator().list(LEXY_LIT("abc"),
                                                                dsl::sep(LEXY_LIT(",")))));
    CHECK(equivalent_rules(brackets.opt_list(LEXY_LIT("abc")),
                           brackets.open() >> brackets.as_terminator().opt_list(LEXY_LIT("abc"))));
    CHECK(equivalent_rules(brackets.opt_list(LEXY_LIT("abc"), dsl::sep(LEXY_LIT(","))),
                           brackets.open()
                               >> brackets.as_terminator().opt_list(LEXY_LIT("abc"),
                                                                    dsl::sep(LEXY_LIT(",")))));

    CHECK(equivalent_rules(brackets.limit(dsl::lit_c<'!'>).recovery_rule(),
                           dsl::recover(dsl::lit_c<')'>).limit(dsl::lit_c<'!'>)));
    CHECK(equivalent_rules(brackets.limit(dsl::lit_c<'!'>).limit(dsl::lit_c<'.'>),
                           brackets.limit(dsl::lit_c<'!'>, dsl::lit_c<'.'>)));

    CHECK(equivalent_rules(dsl::round_bracketed, brackets));
    CHECK(equivalent_rules(dsl::square_bracketed, dsl::brackets(dsl::lit_c<'['>, dsl::lit_c<']'>)));
    CHECK(equivalent_rules(dsl::curly_bracketed, dsl::brackets(dsl::lit_c<'{'>, dsl::lit_c<'}'>)));
    CHECK(equivalent_rules(dsl::angle_bracketed, dsl::brackets(dsl::lit_c<'<'>, dsl::lit_c<'>'>)));
    CHECK(equivalent_rules(dsl::parenthesized, brackets));
}

