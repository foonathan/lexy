// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_ASSERT_HPP_INCLUDED
#define LEXY_DETAIL_ASSERT_HPP_INCLUDED

#include <cassert>
#include <cstdlib>

#define LEXY_PRECONDITION(Expr) ((Expr) ? (void)0 : assert(0))
#define LEXY_ASSERT(Expr, Msg) ((Expr) ? (void)0 : assert(false && Msg))

#ifdef NDEBUG

namespace lexy::_detail
{
[[noreturn]] inline void unreachable()
{
    std::abort();
}
} // namespace lexy::_detail

#    define LEXY_UNREACHABLE() ::lexy::_detail::unreachable()

#else

#    define LEXY_UNREACHABLE() assert(false)

#endif

#endif // LEXY_DETAIL_ASSERT_HPP_INCLUDED

