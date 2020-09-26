// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/buffer.hpp>

#include <doctest.h>
#include <memory_resource>

TEST_CASE("buffer")
{
    static const char str[] = {'a', 'b', 'c'};
    struct view_type
    {
        auto data() const
        {
            return str;
        }

        std::size_t size() const
        {
            return 3;
        }
    };

    auto verify = [](const auto& buffer) {
        CHECK(!buffer.empty());

        CHECK(buffer.begin() == buffer.data());
        CHECK(buffer.end() == buffer.data() + 3);
        CHECK(buffer.size() == 3);
        CHECK(buffer.length() == 3);

        CHECK(buffer.data()[0] == 'a');
        CHECK(buffer.data()[1] == 'b');
        CHECK(buffer.data()[2] == 'c');
    };

    SUBCASE("constructor, default encoding, default resource")
    {
        const lexy::buffer ptr_size(str, 3);
        verify(ptr_size);

        const lexy::buffer ptr_ptr(str, str + 3);
        verify(ptr_ptr);

        const lexy::buffer view(view_type{});
        verify(view);

        if constexpr (LEXY_HAS_EMPTY_MEMBER)
            CHECK(sizeof(ptr_size) == 2 * sizeof(void*));
    }
    SUBCASE("constructor, default encoding, custom resource")
    {
        const lexy::buffer ptr_size(str, 3, std::pmr::new_delete_resource());
        verify(ptr_size);

        const lexy::buffer ptr_ptr(str, str + 3, std::pmr::new_delete_resource());
        verify(ptr_ptr);

        const lexy::buffer view(view_type{}, std::pmr::new_delete_resource());
        verify(view);

        CHECK(sizeof(ptr_size) == 3 * sizeof(char*));
    }
    SUBCASE("constructor, custom encoding, default resource")
    {
        static const auto ustr = reinterpret_cast<const unsigned char*>(str);
        struct uview_type
        {
            auto data() const
            {
                return ustr;
            }

            std::size_t size() const
            {
                return 3;
            }
        };

        const lexy::buffer ptr_size(ustr, 3);
        CHECK(std::is_same_v<decltype(ptr_size)::encoding, lexy::raw_encoding>);
        verify(ptr_size);

        const lexy::buffer ptr_ptr(ustr, ustr + 3);
        CHECK(std::is_same_v<decltype(ptr_ptr)::encoding, lexy::raw_encoding>);
        verify(ptr_ptr);

        const lexy::buffer view(uview_type{});
        CHECK(std::is_same_v<decltype(view)::encoding, lexy::raw_encoding>);
        verify(view);

        const lexy::buffer<lexy::raw_encoding> ptr_size_conv(str, 3);
        verify(ptr_size_conv);

        const lexy::buffer<lexy::raw_encoding> ptr_ptr_conv(str, str + 3);
        verify(ptr_ptr);

        const lexy::buffer<lexy::raw_encoding> view_conv(view_type{});
        verify(view_conv);
    }
    SUBCASE("constructor, custom encoding, custom resource")
    {
        static const auto ustr = reinterpret_cast<const unsigned char*>(str);
        struct uview_type
        {
            auto data() const
            {
                return ustr;
            }

            std::size_t size() const
            {
                return 3;
            }
        };

        const lexy::buffer ptr_size(ustr, 3, std::pmr::new_delete_resource());
        CHECK(std::is_same_v<decltype(ptr_size)::encoding, lexy::raw_encoding>);
        verify(ptr_size);

        const lexy::buffer ptr_ptr(ustr, ustr + 3, std::pmr::new_delete_resource());
        CHECK(std::is_same_v<decltype(ptr_ptr)::encoding, lexy::raw_encoding>);
        verify(ptr_ptr);

        const lexy::buffer view(uview_type{}, std::pmr::new_delete_resource());
        CHECK(std::is_same_v<decltype(view)::encoding, lexy::raw_encoding>);
        verify(view);

        using buffer_type = lexy::buffer<lexy::raw_encoding, std::pmr::memory_resource>;
        const buffer_type ptr_size_conv(str, 3, std::pmr::new_delete_resource());
        verify(ptr_size_conv);

        const buffer_type ptr_ptr_conv(str, str + 3, std::pmr::new_delete_resource());
        verify(ptr_ptr);

        const buffer_type view_conv(view_type{}, std::pmr::new_delete_resource());
        verify(view_conv);
    }

    SUBCASE("copy constructor")
    {
        const lexy::buffer original(str, str + 3);

        const lexy::buffer copy(original);
        verify(copy);

        lexy::_detail::default_memory_resource other_resource;
        const lexy::buffer                     copy_resource(original, &other_resource);
        verify(copy_resource);
    }
    SUBCASE("move constructor")
    {
        lexy::buffer original(str, str + 3);

        lexy::buffer move(LEXY_MOV(original));
        verify(move);
        CHECK(original.empty());
    }
    SUBCASE("copy assignment")
    {
        const lexy::buffer other(str, str + 3);

        lexy::buffer buffer{};
        CHECK(buffer.empty());
        buffer = other;
        verify(buffer);
    }
    SUBCASE("move assignment")
    {
        lexy::buffer other(str, str + 3);

        lexy::buffer buffer{};
        CHECK(buffer.empty());
        buffer = LEXY_MOV(other);
        verify(buffer);
        CHECK(other.empty());
    }

    SUBCASE("reader, no sentinel")
    {
        const lexy::buffer buffer(str, 3);

        auto reader = buffer.reader();
        CHECK(reader.cur() == buffer.data());
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.cur() == buffer.data() + 1);
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.cur() == buffer.data() + 2);
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.cur() == buffer.data() + 3);
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("reader, sentinel")
    {
        const lexy::buffer<lexy::ascii_encoding> buffer(str, 3);

        auto reader = buffer.reader();
        CHECK(reader.cur() == buffer.data());
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.cur() == buffer.data() + 1);
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.cur() == buffer.data() + 2);
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.cur() == buffer.data() + 3);
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
}

