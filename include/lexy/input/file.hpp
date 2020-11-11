// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_FILE_HPP_INCLUDED
#define LEXY_INPUT_FILE_HPP_INCLUDED

#include <lexy/_detail/std.hpp>
#include <lexy/input/base.hpp>
#include <lexy/input/buffer.hpp>
#include <lexy/result.hpp>

namespace lexy
{
/// Errors that might occur while reading the file.
enum class file_error
{
    _success,
    /// An internal OS error, such as failure to read from the file.
    os_error,
    /// The file was not found.
    file_not_found,
    /// The file cannot be opened.
    permission_denied,
};
} // namespace lexy

namespace lexy::_detail
{
using file_callback = void (*)(void* user_data, const char* memory, std::size_t size);

// Reads the entire contents of the specified file into memory.
// On success, invokes the callback before freeing the memory.
// On error, returns the error without invoking the callback.
//
// Do not change ABI, especially with different build configurations!
file_error read_file(const char* path, file_callback cb, void* user_data);
} // namespace lexy::_detail

namespace lexy
{
/// Reads the file at the specified path into a buffer.
template <typename Encoding          = default_encoding,
          encoding_endianness Endian = encoding_endianness::bom,
          typename MemoryResource    = _detail::default_memory_resource>
auto read_file(const char*     path,
               MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    -> result<buffer<Encoding, MemoryResource>, file_error>
{
    using buffer_type = buffer<Encoding, MemoryResource>;

    struct user_data_t
    {
        buffer_type     buffer;
        MemoryResource* resource;
    } user_data{buffer_type(resource), resource};

    auto error = _detail::read_file(
        path,
        [](void* _user_data, const char* memory, std::size_t size) {
            auto user_data = static_cast<user_data_t*>(_user_data);

            user_data->buffer
                = lexy::make_buffer<Encoding, Endian>(memory, size, user_data->resource);
        },
        &user_data);

    if (error == file_error::_success)
        return {lexy::result_value, LEXY_MOV(user_data.buffer)};
    else
        return {lexy::result_error, error};
}
} // namespace lexy

#endif // LEXY_INPUT_FILE_HPP_INCLUDED

