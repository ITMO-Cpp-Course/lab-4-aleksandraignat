#pragma once

#include "resource_core/resource_error.hpp"
#include <cstdio>
#include <memory>
#include <string>

namespace lab4::resource
{
class FileHandle
{
  public:
    FileHandle() noexcept = default;

    explicit FileHandle(const std::string& filename, const std::string& mode);

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    FileHandle(FileHandle&& other) noexcept;

    FileHandle& operator=(FileHandle&& other) noexcept;

    ~FileHandle();

    void open(const std::string& filename, const std::string& mode);

    void close();

    std::FILE* get() const noexcept
    {
        return file_;
    }

    bool is_open() const noexcept
    {
        return file_ != nullptr;
    }

    explicit operator bool() const noexcept
    {
        return is_open();
    }

    std::string read_line();

    void write_line(const std::string& data);

  private:
    std::FILE* file_ = nullptr;
    std::string filename_;

    void check_open() const;
};

} // namespace lab4::resource
