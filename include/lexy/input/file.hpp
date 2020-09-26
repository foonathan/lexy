// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_FILE_HPP_INCLUDED
#define LEXY_INPUT_FILE_HPP_INCLUDED

#include <cstdio>
#include <lexy/_detail/buffer_builder.hpp>
#include <lexy/input/base.hpp>
#include <lexy/input/buffer.hpp>
#include <lexy/result.hpp>

namespace lexy
{
class _file_handle
{
public:
    explicit _file_handle(std::FILE* file) noexcept : _file(file) {}

    _file_handle(const _file_handle&) = delete;
    _file_handle& operator=(const _file_handle&) = delete;

    ~_file_handle() noexcept
    {
        if (_file)
            std::fclose(_file);
    }

    operator std::FILE*() const noexcept
    {
        return _file;
    }

private:
    std::FILE* _file;
};

/// Reads the file at the specified path into a buffer.
template <typename Encoding       = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
optional_value<buffer<Encoding, MemoryResource>> read_file(
    const char* path, MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
{
    _file_handle file(std::fopen(path, "rb"));
    if (!file)
        // File not found.
        return {};

    using char_type = typename Encoding::char_type;
    _detail::buffer_builder<char_type> buffer;
    while (true)
    {
        const auto buffer_size = buffer.write_size();
        LEXY_ASSERT(buffer_size > 0, "buffer empty?!");

        // Read into the entire write area of the buffer from the file,
        // commiting what we've just read.
        const auto read = std::fread(buffer.write_data(), sizeof(char_type), buffer_size, file);
        buffer.commit(read);

        // Check whether we have exhausted the file.
        if (read < buffer_size)
        {
            if (std::ferror(file))
                // We have a read error.
                return {};

            // We should have reached the end of the file.
            LEXY_ASSERT(std::feof(file), "why did fread() not read enough?");
            break;
        }

        // We've filled the entire buffer and need more space.
        // This grow might be unnecessary if we're just so happen to reach EOF with the next
        // input, but checking this requires reading more input.
        buffer.grow();
    }

    return {lexy::result_value, buffer.read_data(), buffer.read_size(), resource};
}
} // namespace lexy

// Technically not allowed, but I'm willing to take the risk.
namespace std::filesystem
{
class path;
} // namespace std::filesystem

namespace lexy
{
template <typename Encoding       = default_encoding, typename Path,
          typename MemoryResource = _detail::default_memory_resource,
          typename                = std::enable_if_t<std::is_same_v<Path, std::filesystem::path>>>
auto read_file(const Path&     path,
               MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
{
    return read_file(path.c_str(), resource);
}
} // namespace lexy

#endif // LEXY_INPUT_FILE_HPP_INCLUDED

