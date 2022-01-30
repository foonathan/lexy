// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_CFILE_HPP_INCLUDED
#define LEXY_EXT_CFILE_HPP_INCLUDED

#include <cstdio>
#include <lexy/_detail/buffer_builder.hpp>
#include <lexy/input/file.hpp>

namespace lexy_ext
{
/// Reads from a FILE as opposed to a path.
template <typename Encoding                = lexy::default_encoding,
          lexy::encoding_endianness Endian = lexy::encoding_endianness::bom,
          typename MemoryResource          = lexy::_detail::default_memory_resource>
[[deprecated("use lexy::read_stdin() for reading stdin; lexy::read_file() for other files")]] auto
    read_file(std::FILE*      file,
              MemoryResource* resource = lexy::_detail::get_memory_resource<MemoryResource>())
        -> lexy::read_file_result<Encoding, MemoryResource>
{
    using result_type = lexy::read_file_result<Encoding, MemoryResource>;

    if (!file)
        return result_type(lexy::file_error::file_not_found, resource);
    else if (std::ferror(file))
        return result_type(lexy::file_error::os_error, resource);

    // We can't use ftell() to get file size, as the file might not be open in binary mode or is
    // stdin. So instead use a conservative loop.
    lexy::_detail::buffer_builder<char> builder;
    while (true)
    {
        const auto buffer_size = builder.write_size();
        LEXY_ASSERT(buffer_size > 0, "buffer empty?!");

        // Read into the entire write area of the buffer from the file,
        // commiting what we've just read.
        const auto read = std::fread(builder.write_data(), sizeof(char), buffer_size, file);
        builder.commit(read);

        // Check whether we have exhausted the file.
        if (read < buffer_size)
        {
            if (std::ferror(file))
                // We have a read error.
                return result_type(lexy::file_error::os_error, resource);

            // We should have reached the end of the file.
            LEXY_ASSERT(std::feof(file), "why did fread() not read enough?");
            break;
        }

        // We've filled the entire buffer and need more space.
        // This grow might be unnecessary if we're just so happen to reach EOF with the next
        // input, but checking this requires reading more input.
        builder.grow();
    }

    auto buffer = lexy::make_buffer_from_raw<Encoding, Endian>(builder.read_data(),
                                                               builder.read_size(), resource);
    return result_type(lexy::file_error::_success, LEXY_MOV(buffer));
}
} // namespace lexy_ext

#endif // LEXY_EXT_CFILE_HPP_INCLUDED

