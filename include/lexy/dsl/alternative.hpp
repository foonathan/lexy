// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/_detail/iterator.hpp>
#include <lexy/_detail/trie.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>

namespace lexy
{
struct exhausted_alternatives
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted alternatives";
    }
};
} // namespace lexy

namespace lexy::_detail
{
template <typename... Tokens>
struct build_token_trie;

template <>
struct build_token_trie<>
{
    template <typename... Strings>
    static constexpr auto trie()
    {
        if constexpr (sizeof...(Strings) == 0)
            return lexy::_detail::trie<char>;
        else
            return lexy::_detail::trie<std::common_type_t<typename Strings::char_type...>,
                                       Strings...>;
    }

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens = Templ<Tokens...>;
};

template <typename H, typename... T>
struct build_token_trie<H, T...>
{
    template <typename... Strings>
    static constexpr auto trie()
    {
        return build_token_trie<T...>::template trie<Strings...>();
    }

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens =
        typename build_token_trie<T...>::template other_tokens<Templ, Tokens..., H>;
};
template <typename CharT, CharT... C, typename... T>
struct build_token_trie<lexyd::_lit<CharT, C...>, T...>
{
    template <typename... Strings>
    static constexpr auto trie()
    {
        return build_token_trie<T...>::template trie<Strings..., type_string<CharT, C...>>();
    }

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens = typename build_token_trie<T...>::template other_tokens<Templ, Tokens...>;
};
} // namespace lexy::_detail

namespace lexyd
{
template <typename... Tokens>
struct _malt : token_base<_malt<Tokens...>>
{
    template <typename Reader,
              typename Indices = lexy::_detail::make_index_sequence<sizeof...(Tokens)>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::iterator end;

        constexpr auto try_parse([[maybe_unused]] const Reader& reader)
        {
            if constexpr (sizeof...(Tokens) == 0)
                return std::false_type{};
            else
            {
                auto result = false;
                auto impl   = [&](auto token, Reader local_reader) {
                    // Try to match the current token.
                    if (!lexy::try_match_token(token, local_reader))
                        return;

                    // Update end to longest match.
                    end    = lexy::_detail::max_range_end(reader.position(), this->end,
                                                       local_reader.position());
                    result = true;
                };

                // Need to try everything.
                end = reader.position();
                (impl(Tokens{}, reader), ...);
                return result;
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::exhausted_alternatives>(reader.position());
            context.on(_ev::error{}, err);
        }
    };
};

template <typename... Tokens>
struct _alt : token_base<_alt<Tokens...>>
{
    using _builder              = lexy::_detail::build_token_trie<typename Tokens::token_type...>;
    static constexpr auto _trie = _builder::trie();

    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr bool try_parse(const Reader& reader)
        {
            lexy::token_parser_for<typename _builder::template other_tokens<_malt>, Reader>
                manual_parser{};

            // We check the trie as a baseline.
            // This gives us a first end position.
            if (auto trie_reader = reader;
                lexy::_detail::trie_parser<_trie, Reader>::try_match(trie_reader))
            {
                end = trie_reader.position();

                if (trie_reader.peek() == Reader::encoding::eof())
                    // Exit early, there can't be a longer match.
                    return true;

                // Check the remaining tokens to see if we have a longer match.
                if (manual_parser.try_parse(reader))
                    end = lexy::_detail::max_range_end(reader.position(), end, manual_parser.end);

                return true;
            }
            else
            {
                // Check the remaining tokens only.
                auto result = manual_parser.try_parse(reader);
                end         = manual_parser.end;
                return result;
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::exhausted_alternatives>(reader.position());
            context.on(_ev::error{}, err);
        }
    };
};

template <typename R, typename S>
constexpr auto operator/(R, S)
{
    static_assert(lexy::is_token_rule<R> && lexy::is_token_rule<S>);
    return _alt<R, S>{};
}

template <typename... R, typename S>
constexpr auto operator/(_alt<R...>, S)
{
    static_assert(lexy::is_token_rule<S>);
    return _alt<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator/(R, _alt<S...>)
{
    static_assert(lexy::is_token_rule<R>);
    return _alt<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator/(_alt<R...>, _alt<S...>)
{
    return _alt<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

