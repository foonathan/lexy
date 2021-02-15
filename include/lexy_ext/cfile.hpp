// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_CFILE_HPP_INCLUDED
#define LEXY_EXT_CFILE_HPP_INCLUDED

#include <cstdio>
#include <lexy/_detail/buffer_builder.hpp>
#include <lexy/input/file.hpp>

namespace lexy_ext
{
inline lexy::file_error _read_cfile(std::FILE* file, lexy::_detail::buffer_builder<char>& buffer)
{
    while (true)
    {
        const auto buffer_size = buffer.write_size();
        LEXY_ASSERT(buffer_size > 0, "buffer empty?!");

        // Read into the entire write area of the buffer from the file,
        // commiting what we've just read.
        const auto read = std::fread(buffer.write_data(), sizeof(char), buffer_size, file);
        buffer.commit(read);

        // Check whether we have exhausted the file.
        if (read < buffer_size)
        {
            if (std::ferror(file))
                // We have a read error.
                return lexy::file_error::os_error;

            // We should have reached the end of the file.
            LEXY_ASSERT(std::feof(file), "why did fread() not read enough?");
            break;
        }

        // We've filled the entire buffer and need more space.
        // This grow might be unnecessary if we're just so happen to reach EOF with the next
        // input, but checking this requires reading more input.
        buffer.grow();
    }

    return lexy::file_error::_success;
}

/// Reads from a FILE as opposed to a path.
template <typename Encoding                = lexy::default_encoding,
          lexy::encoding_endianness Endian = lexy::encoding_endianness::bom,
          typename MemoryResource          = lexy::_detail::default_memory_resource>
auto read_file(std::FILE*      file,
               MemoryResource* resource = lexy::_detail::get_memory_resource<MemoryResource>())
    -> lexy::result<lexy::buffer<Encoding, MemoryResource>, lexy::file_error>
{
    if (!file)
        return {lexy::result_error, lexy::file_error::file_not_found};
    else if (std::ferror(file))
        return {lexy::result_error, lexy::file_error::os_error};

    lexy::_detail::buffer_builder<char> builder;
    auto                                error = _read_cfile(file, builder);
    if (error != lexy::file_error::_success)
        return {lexy::result_error, error};

    auto buffer
        = lexy::make_buffer<Encoding, Endian>(builder.read_data(), builder.read_size(), resource);
    return {lexy::result_value, LEXY_MOV(buffer)};
}
} // namespace lexy_ext

#endif // LEXY_EXT_CFILE_HPP_INCLUDED

