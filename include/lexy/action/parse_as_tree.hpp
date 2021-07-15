// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED
#define LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/action/validate.hpp>
#include <lexy/parse_tree.hpp>

namespace lexy
{
template <typename Tree, typename Input, typename Callback>
class parse_tree_handler
{
    using iterator = typename lexy::input_reader<Input>::iterator;

public:
    explicit parse_tree_handler(Tree& tree, const Input& input, const Callback& cb)
    : _tree(&tree), _depth(0), _validate(input, cb)
    {}

    constexpr auto get_result(bool did_recover) &&
    {
        return LEXY_MOV(_validate).get_result(did_recover);
    }

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    constexpr auto get_result_value() && noexcept
    {
        return LEXY_MOV(_validate).template get_result_value<Production>();
    }
    template <typename Production>
    constexpr auto get_result_empty() && noexcept
    {
        return LEXY_MOV(_validate).template get_result_empty<Production>();
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {
        typename Tree::builder::marker                                                builder;
        typename lexy::validate_handler<Input, Callback>::template marker<Production> validate;
    };

    template <typename Production>
    constexpr auto on(parse_events::production_start<Production>, iterator pos)
    {
        if (_depth++ == 0)
        {
            _builder.emplace(LEXY_MOV(*_tree), Production{});
            return marker<Production>{{}, {pos}};
        }
        else
        {
            return marker<Production>{_builder->start_production(Production{}), {pos}};
        }
    }

    template <typename Production, typename Iterator>
    constexpr auto on(marker<Production>, parse_events::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename TokenKind>
    constexpr void on(const marker<Production>&, parse_events::token, TokenKind kind,
                      iterator begin, iterator end)
    {
        _builder->token(kind, begin, end);
    }

    template <typename Production, typename Error>
    constexpr void on(marker<Production> m, parse_events::error, Error&& error)
    {
        _validate.on(m.validate, parse_events::error{}, LEXY_FWD(error));
    }

    template <typename Production, typename... Args>
    constexpr void on(marker<Production>&& m, parse_events::production_finish<Production>, iterator,
                      Args&&...)
    {
        if (--_depth == 0)
            // Finish tree instead of production.
            *_tree = LEXY_MOV(*_builder).finish();
        else
            _builder->finish_production(LEXY_MOV(m.builder));
    }
    template <typename Production>
    constexpr void on(marker<Production>&& m, parse_events::production_cancel<Production>, iterator)
    {
        if (--_depth == 0)
            // Clear tree instead of finishing production.
            _tree->clear();
        else
            _builder->cancel_production(LEXY_MOV(m.builder));
    }

    template <typename... Args>
    constexpr void on(const Args&...)
    {}

private:
    lexy::_detail::lazy_init<typename Tree::builder> _builder;
    Tree*                                            _tree;
    int                                              _depth;

    lexy::validate_handler<Input, Callback> _validate;
};

template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    auto handler = parse_tree_handler(tree, input, LEXY_MOV(callback));
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), reader);
}
} // namespace lexy

#endif // LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED

