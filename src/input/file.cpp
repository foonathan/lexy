// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/file.hpp>

#include <cerrno>
#include <lexy_ext/cfile.hpp>

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
    auto                          error = lexy_ext::_read_cfile(file, buffer);
    if (error != file_error::_success)
        return error;

    cb(user_data, buffer.read_data(), buffer.read_size());
    return file_error::_success;
}

