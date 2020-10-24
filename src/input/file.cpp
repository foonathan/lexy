// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/file.hpp>

#include <cerrno>
#include <cstdio>
#include <lexy/_detail/buffer_builder.hpp>

namespace
{
class file_handle
{
public:
    explicit file_handle(std::FILE* file) noexcept : _file(file) {}

    file_handle(const file_handle&) = delete;
    file_handle& operator=(const file_handle&) = delete;

    ~file_handle() noexcept
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

lexy::file_error get_file_error() noexcept
{
    switch (errno)
    {
    case ENOENT:
    case ENOTDIR:
    case ELOOP:
        return lexy::file_error::file_not_found;

    case EACCES:
    case EPERM:
        return lexy::file_error::permission_denied;

    default:
        return lexy::file_error::os_error;
    }
}
} // namespace

lexy::file_error lexy::_detail::read_file(const char* path, file_callback cb, void* user_data)
{
    file_handle file(std::fopen(path, "rb"));
    if (!file)
        return get_file_error();

    _detail::buffer_builder<char> buffer;
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
                return file_error::os_error;

            // We should have reached the end of the file.
            LEXY_ASSERT(std::feof(file), "why did fread() not read enough?");
            break;
        }

        // We've filled the entire buffer and need more space.
        // This grow might be unnecessary if we're just so happen to reach EOF with the next
        // input, but checking this requires reading more input.
        buffer.grow();
    }

    cb(user_data, buffer.read_data(), buffer.read_size());
    return file_error::_success;
}

