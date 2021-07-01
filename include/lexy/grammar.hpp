// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_GRAMMAR_HPP_INCLUDED
#define LEXY_GRAMMAR_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/type_name.hpp>

//=== rule ===//
#if 0
struct Rule : rule_base
{
    static constexpr auto is_branch = false;
    static constexpr auto is_unconditional_branch = false; // Only set to true if is_branch is also true.

    template <typename NextParser>
    struct parser
    {
        // Only if `is_branch == true` and `is_unconditional_branch == false`.
        template <typename Context, typename Reader, typename ... Args>
        LEXY_DSL_FUNC rule_try_parse_result try_parse(Context& context, Reader& reader, Args&&... args)
        {
            if (/* check whether we would match without consuming */)
            {
                if (/* matched and consumed */)
                {
                    auto result = NextParser::parse(context, reader, LEXY_FWD(args)..., /* rule arguments */);
                    return static_cast<rule_try_parse_result>(result);
                }
                else
                {
                    context.error(/* error */);
                    return rule_try_parse_result::canceled;
                }
            }
            else
            {
                return rule_try_parse_result::backtracked;
            }
        }

        template <typename Context, typename Reader, typename ... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (/* matched and consumed */)
                return NextParser::parse(context, reader, LEXY_FWD(args)..., /* rule arguments */);
            else
            {
                context.error(/* error */);
                return false;
            }
        }
    };
};
#endif

// We use a shorthand namespace to decrease symbol size.
namespace lexyd
{
struct rule_base
{
    static constexpr auto is_branch               = false;
    static constexpr auto is_unconditional_branch = false;
};

struct _token_base : rule_base
{};
} // namespace lexyd

namespace lexy
{
namespace dsl = lexyd;

enum class rule_try_parse_result
{
    ok          = true,
    canceled    = false,
    backtracked = 2,
};

template <typename T>
constexpr bool is_rule = std::is_base_of_v<dsl::rule_base, T>;
template <typename T>
constexpr bool is_token_rule = std::is_base_of_v<dsl::_token_base, T>;

template <typename T>
constexpr bool is_branch_rule = [] {
    if constexpr (is_rule<T>)
        return T::is_branch;
    else
        return false;
}();

template <typename Rule, typename NextParser>
using rule_parser = typename Rule::template parser<NextParser>;
} // namespace lexy

//=== production ===//
namespace lexy
{
template <typename Production>
using production_rule = std::decay_t<decltype(Production::rule)>;

template <typename Production>
constexpr bool is_production = _detail::is_detected<production_rule, Production>;

/// Base class to indicate that this production is conceptually a token.
/// This inhibits whitespace skipping inside the production.
///
/// When generating a parse tree, it will also merge tokens of the same kind into the same node.
struct token_production
{};

template <typename Production>
constexpr bool is_token_production = std::is_base_of_v<token_production, Production>;

/// Base class to indicate that this production is transparent for the parse tree generation.
/// It will not create a node in the tree, all children will be added to the its parent.
/// If parse tree generation is not used, it has no effect.
struct transparent_production
{};

template <typename Production>
constexpr bool is_transparent_production = std::is_base_of_v<transparent_production, Production>;

template <typename Production>
LEXY_CONSTEVAL const char* production_name()
{
    return _detail::type_name<Production>();
}
} // namespace lexy

namespace lexy
{
template <typename Production>
using _detect_whitespace = decltype(Production::whitespace);

template <typename Production, typename Root>
auto _production_whitespace()
{
    if constexpr (is_token_production<Production>)
        return; // void
    else if constexpr (lexy::_detail::is_detected<_detect_whitespace, Production>)
        return Production::whitespace;
    else if constexpr (lexy::_detail::is_detected<_detect_whitespace, Root>)
        return Root::whitespace;
    else
        return; // void
}
template <typename Production, typename Root>
using production_whitespace = decltype(_production_whitespace<Production, Root>());
} // namespace lexy

namespace lexy
{
template <typename Production>
struct production_value
{
    static constexpr auto get = Production::value;
    using type                = std::decay_t<decltype(get)>;
};
} // namespace lexy

#endif // LEXY_GRAMMAR_HPP_INCLUDED

