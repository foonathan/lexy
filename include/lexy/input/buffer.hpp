// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_BUFFER_HPP_INCLUDED
#define LEXY_INPUT_BUFFER_HPP_INCLUDED

#include <cstring>
#include <lexy/_detail/memory_resource.hpp>
#include <lexy/error.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
/// Stores the input that will be parsed.
/// For encodings with spare code points, it can append an EOF sentinel.
/// This allows branch-less detection of EOF.
template <typename Encoding       = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
class buffer
{
    static constexpr auto _has_sentinel
        = std::is_same_v<typename Encoding::char_type, typename Encoding::int_type>;

public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;

    //=== constructors ===//
    constexpr buffer() noexcept : buffer(_detail::get_memory_resource<MemoryResource>()) {}

    constexpr buffer(MemoryResource* resource) noexcept
    : _resource(resource), _data(nullptr), _size(0)
    {}

    explicit buffer(const char_type* data, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : _resource(resource), _size(size)
    {
        _data = allocate(size);
        std::memcpy(_data, data, size * sizeof(char_type));
    }
    explicit buffer(const char_type* begin, const char_type* end,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(begin, std::size_t(end - begin), resource)
    {}

    template <typename CharT, typename = _require_secondary_char_type<encoding, CharT>>
    explicit buffer(const CharT* data, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(reinterpret_cast<const char_type*>(data), size, resource)
    {
        static_assert(sizeof(CharT) == sizeof(char_type));
    }
    template <typename CharT, typename = _require_secondary_char_type<encoding, CharT>>
    explicit buffer(const CharT* begin, const CharT* end,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(reinterpret_cast<const char_type*>(begin), reinterpret_cast<const char_type*>(end),
             resource)
    {
        static_assert(sizeof(CharT) == sizeof(char_type));
    }

    template <typename View, typename = decltype(LEXY_DECLVAL(View).data())>
    explicit buffer(const View&     view,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(view.data(), view.size(), resource)
    {}

    buffer(const buffer& other) : buffer(other.data(), other.size(), other._resource.get()) {}
    buffer(const buffer& other, MemoryResource* resource)
    : buffer(other.data(), other.size(), resource)
    {}

    buffer(buffer&& other) noexcept
    : _resource(other._resource), _data(other._data), _size(other._size)
    {
        other._data = nullptr;
        other._size = 0;
    }

    ~buffer() noexcept
    {
        if (!_data)
            return;

        if constexpr (_has_sentinel)
            _resource->deallocate(_data, (_size + 1) * sizeof(char_type), alignof(char_type));
        else
            _resource->deallocate(_data, _size * sizeof(char_type), alignof(char_type));
    }

    buffer& operator=(const buffer& other)
    {
        // Create a temporary buffer that owns the same memory as other but with our resource.
        // We then move assign it to *this.
        return *this = buffer(other, _resource.get());
    }

    buffer& operator=(buffer&& other) noexcept(std::is_empty_v<MemoryResource>)
    {
        if (*_resource == *other._resource)
        {
            // We have the same resource; we can just steal other's memory.
            // We do that by swapping - when other is destroyed it will free our memory.
            _detail::swap(_data, other._data);
            _detail::swap(_size, other._size);
            return *this;
        }
        else
        {
            LEXY_PRECONDITION(!std::is_empty_v<MemoryResource>);

            // We create a copy using the right resource and swap the ownership.
            buffer copy(other, _resource.get());
            _detail::swap(_data, copy._data);
            _detail::swap(_size, copy._size);
            return *this;
        }
    }

    //=== access ===//
    const char_type* begin() const noexcept
    {
        return _data;
    }
    const char_type* end() const noexcept
    {
        return _data + _size;
    }

    const char_type* data() const noexcept
    {
        return _data;
    }

    bool empty() const noexcept
    {
        return _size == 0;
    }

    std::size_t size() const noexcept
    {
        return _size;
    }
    std::size_t length() const noexcept
    {
        return _size;
    }

    //=== input ===//
    auto reader() const& noexcept
    {
        if constexpr (_has_sentinel)
        {
            class reader_type
            {
            public:
                using encoding  = Encoding;
                using char_type = typename encoding::char_type;

                using iterator = const char_type*;

                auto peek() const noexcept
                {
                    // The last one will be EOF.
                    return *_cur;
                }

                void bump() noexcept
                {
                    ++_cur;
                }

                iterator cur() const noexcept
                {
                    return _cur;
                }

            private:
                explicit reader_type(iterator begin) noexcept : _cur(begin) {}

                iterator _cur;
                friend buffer;
            };

            return reader_type(_data);
        }
        else
            return _detail::range_reader<encoding, const char_type*>(_data, _data + _size);
    }

private:
    char_type* allocate(std::size_t size) const
    {
        if constexpr (_has_sentinel)
            ++size;

        auto memory = static_cast<char_type*>(
            _resource->allocate(size * sizeof(char_type), alignof(char_type)));
        if constexpr (_has_sentinel)
            memory[size - 1] = encoding::eof();
        return memory;
    }

    LEXY_EMPTY_MEMBER _detail::memory_resource_ptr<MemoryResource> _resource;
    char_type*                                                     _data;
    std::size_t                                                    _size;
};

template <typename CharT>
buffer(const CharT*, const CharT*) -> buffer<deduce_encoding<CharT>>;
template <typename CharT>
buffer(const CharT*, std::size_t) -> buffer<deduce_encoding<CharT>>;
template <typename View>
buffer(const View&) -> buffer<deduce_encoding<std::decay_t<decltype(*LEXY_DECLVAL(View).data())>>>;

template <typename CharT, typename MemoryResource>
buffer(const CharT*, const CharT*, MemoryResource*)
    -> buffer<deduce_encoding<CharT>, MemoryResource>;
template <typename CharT, typename MemoryResource>
buffer(const CharT*, std::size_t, MemoryResource*)
    -> buffer<deduce_encoding<CharT>, MemoryResource>;
template <typename View, typename MemoryResource>
buffer(const View&, MemoryResource*)
    -> buffer<deduce_encoding<std::decay_t<decltype(*LEXY_DECLVAL(View).data())>>, MemoryResource>;

//=== convenience typedefs ===//
template <typename Encoding       = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
using buffer_lexeme = lexeme_for<buffer<Encoding, MemoryResource>>;

template <typename Tag, typename Encoding = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
using buffer_error = error_for<buffer<Encoding, MemoryResource>, Tag>;

template <typename Production, typename Encoding = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
using buffer_error_context = error_context<Production, buffer<Encoding, MemoryResource>>;
} // namespace lexy

#endif // LEXY_INPUT_BUFFER_HPP_INCLUDED

