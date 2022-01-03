// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/_detail/iterator.hpp>
#include <lexy/_detail/trie.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/char_class.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>

namespace lexy::_detail
{
template <typename... Tokens>
struct partition_alt;

template <>
struct partition_alt<>
{
    template <typename Encoding, template <typename...> typename Templ, typename... Strings>
    using trie_tokens = Templ<Strings...>;

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens = Templ<Tokens...>;
};

template <typename H, typename... T>
struct partition_alt<H, T...>
{
    using _tail = partition_alt<T...>;

    template <typename Encoding, template <typename...> typename Templ, typename... Strings>
    using trie_tokens = typename _tail::template trie_tokens<Encoding, Templ, Strings...>;

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens = typename _tail::template other_tokens<Templ, Tokens..., H>;
};
template <typename CharT, CharT... C, typename... T>
struct partition_alt<lexyd::_lit<CharT, C...>, T...>
{
    using _tail = partition_alt<T...>;

    template <typename Encoding, template <typename...> typename Templ, typename... Strings>
    using trie_tokens =
        typename _tail::template trie_tokens<Encoding, Templ, Strings..., type_string<CharT, C...>>;

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens = typename _tail::template other_tokens<Templ, Tokens...>;
};
template <char32_t Cp, typename... T>
struct partition_alt<lexyd::_lcp<Cp>, T...>
{
    using _tail = partition_alt<T...>;

    template <typename Encoding>
    using _string = typename lexyd::_lcp<Cp>::template _string<Encoding>;
    template <typename Encoding, template <typename...> typename Templ, typename... Strings>
    using trie_tokens =
        typename _tail::template trie_tokens<Encoding, Templ, Strings..., _string<Encoding>>;

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens = typename partition_alt<T...>::template other_tokens<Templ, Tokens...>;
};
} // namespace lexy::_detail

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

namespace lexyd
{
template <typename Partition, typename Encoding>
struct _token_trie
{
    template <typename... Strings>
    struct _impl
    {
        static constexpr auto get()
        {
            return lexy::_detail::trie<Encoding, Strings...>;
        }
    };

    static constexpr auto get = Partition::template trie_tokens<Encoding, _impl>::get();
    using parser              = lexy::_detail::trie_parser<get>;
};

template <typename Partition>
struct _malt
{
    template <typename... Tokens>
    struct _impl
    {
        template <typename Reader>
        static constexpr bool try_match(Reader& reader)
        {
            auto result = false;

            auto                  begin   = reader.position();
            auto                  end     = begin;
            [[maybe_unused]] auto process = [&](auto token, Reader local_reader) {
                // Try to match the current token.
                if (!lexy::try_match_token(token, local_reader))
                    return;

                // Update end to longest match.
                end    = lexy::_detail::max_range_end(begin, end, local_reader.position());
                result = true;
            };
            (process(Tokens{}, reader), ...);

            reader.set_position(end);
            return result;
        }
    };

    using parser = typename Partition::template other_tokens<_impl>;
};

template <typename... Tokens>
struct _alt : token_base<_alt<Tokens...>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(const Reader& reader)
        {
            using encoding = typename Reader::encoding;

            using partition     = lexy::_detail::partition_alt<typename Tokens::token_type...>;
            using trie_parser   = typename _token_trie<partition, encoding>::parser;
            using manual_parser = typename _malt<partition>::parser;

            // We check the trie as a baseline.
            // This gives us a first end position.
            if (auto trie_reader = reader; trie_parser::try_match(trie_reader))
            {
                end = trie_reader.position();

                if (trie_reader.peek() == encoding::eof())
                    // Exit early, there can't be a longer match.
                    return true;

                // Check the remaining tokens to see if we have a longer match.
                if (auto manual_reader = reader; manual_parser::try_match(manual_reader))
                    end = lexy::_detail::max_range_end(reader.position(), end,
                                                       manual_reader.position());

                return true;
            }
            else
            {
                // Check the remaining tokens only.
                auto manual_reader = reader;
                auto result        = manual_parser::try_match(manual_reader);
                end                = manual_reader.position();
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

// The generic operator/ overload is in char_class.hpp.

template <typename... R, typename S>
constexpr auto operator/(_alt<R...>, S)
{
    static_assert(lexy::is_token_rule<S>);
    if constexpr ((lexy::is_char_class_rule<S> && ... && _is_convertible_char_class<R>))
        // If we add a char class, we attempt to turn it into a character class alternative.
        return _calt<decltype(_make_char_class(R{}))..., decltype(_make_char_class(S{}))>{};
    else
        return _alt<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator/(R, _alt<S...>)
{
    static_assert(lexy::is_token_rule<R>);
    if constexpr ((lexy::is_char_class_rule<R> && ... && _is_convertible_char_class<S>))
        // If we add a char class, we attempt to turn it into a character class alternative.
        return _calt<decltype(_make_char_class(R{})), decltype(_make_char_class(S{}))...>{};
    else
        return _alt<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator/(_alt<R...>, _alt<S...>)
{
    return _alt<R..., S...>{};
}
} // namespace lexyd

namespace lexy
{
template <typename H, typename... T>
constexpr auto token_kind_of<lexy::dsl::_alt<H, T...>> = [] {
    constexpr auto is_equal = [](auto a, auto b) {
        if constexpr (std::is_same_v<decltype(a), decltype(b)>)
            return a == b;
        else
            return false;
    };

    constexpr auto kind = lexy::token_kind_of<H>;
    if constexpr ((is_equal(kind, lexy::token_kind_of<T>) && ...))
        return kind;
    else
        return lexy::unknown_token_kind;
}();
} // namespace lexy

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

