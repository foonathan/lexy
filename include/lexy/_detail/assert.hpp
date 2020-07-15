// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_ASSERT_HPP_INCLUDED
#define LEXY_DETAIL_ASSERT_HPP_INCLUDED

#include <cassert>

#define LEXY_PRECONDITION(Expr) ((Expr) ? (void)0 : assert(0))
#define LEXY_ASSERT(Expr, Msg) ((Expr) ? (void)0 : assert(false && Msg))

#endif // LEXY_DETAIL_ASSERT_HPP_INCLUDED

