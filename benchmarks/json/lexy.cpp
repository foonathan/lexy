// Copyright (C) 2020-2023 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/action/validate.hpp>
#include <lexy/input/file.hpp>
#include <lexy/input/string_input.hpp>

#define LEXY_TEST
#include "../../examples/json.cpp"

namespace
{
template <typename Encoding>
class swar_disabled_reader
{
public:
    using encoding = Encoding;
    using iterator = const typename Encoding::char_type*;

    explicit swar_disabled_reader(iterator begin) noexcept : _cur(begin) {}

    auto peek() const noexcept
    {
        // The last one will be EOF.
        return *_cur;
    }

    void bump() noexcept
    {
        ++_cur;
    }

    iterator position() const noexcept
    {
        return _cur;
    }

    void set_position(iterator new_pos) noexcept
    {
        _cur = new_pos;
    }

private:
    iterator _cur;
};

template <typename Encoding>
struct swar_disabled_input
{
    const lexy::buffer<Encoding>* _input;

    auto reader() const&
    {
        return swar_disabled_reader<Encoding>(_input->reader().position());
    }
};
} // namespace

bool json_lexy(const lexy::buffer<lexy::utf8_encoding>& input)
{
    auto result = lexy::validate<grammar::json>(input, lexy::noop);
    if (!result)
        throw "buggy implementation";
    return result.is_success();
}

bool json_lexy_no_swar(const lexy::buffer<lexy::utf8_encoding>& input)
{
    auto result = lexy::validate<grammar::json>(swar_disabled_input<lexy::utf8_encoding>{&input},
                                                lexy::noop);
    if (!result)
        throw "buggy implementation";
    return result.is_success();
}

bool json_lexy_no_buffer(const lexy::buffer<lexy::utf8_encoding>& input)
{
    auto result
        = lexy::validate<grammar::json>(lexy::string_input(input.data(), input.size()), lexy::noop);
    if (!result)
        throw "buggy implementation";
    return result.is_success();
}

