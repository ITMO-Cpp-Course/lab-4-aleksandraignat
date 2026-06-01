#include "resource_core/file_handle.hpp"

#include <cerrno>
#include <cstring>

namespace lab4::resource
{

FileHandle::FileHandle(const std::string& filename, const std::string& mode)
    : file_(nullptr), filename_()
{
    open(filename, mode);
}

FileHandle::FileHandle(FileHandle&& other) noexcept
    : file_(other.file_)
    , filename_(std::move(other.filename_))
{
    other.file_ = nullptr;
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept
{
    if (this != &other)
    {
        close();
        file_ = other.file_;
        filename_ = std::move(other.filename_);
        other.file_ = nullptr;
    }
    return *this;
}

FileHandle::~FileHandle()
{
    close();
}

void FileHandle::open(const std::string& filename, const std::string& mode)
{
    if (is_open())
    {
        throw ResourceError("FileHandle already open: " + filename_);
    }

    std::FILE* new_file = std::fopen(filename.c_str(), mode.c_str());

    if (!new_file)
    {
        throw ResourceError("Failed to open file: " + filename + " (errno: " + std::to_string(errno) + ")");
    }

    file_ = new_file;
    filename_ = filename;
}

void FileHandle::close()
{
    if (file_)
    {
        if (std::fclose(file_) != 0)
        {
            throw ResourceError("Failed to close file: " + filename_);
        }
        file_ = nullptr;
        filename_.clear();
    }
}

void FileHandle::check_open() const
{
    if (!is_open())
    {
        throw ResourceError("FileHandle is not open");
    }
}

std::string FileHandle::read_line()
{
    check_open();

    char buffer[4096];

    if (std::fgets(buffer, sizeof(buffer), file_) == nullptr)
    {
        if (std::feof(file_))
        {
            return "";
        }
        throw ResourceError("Failed to read line from file: " + filename_);
    }

    size_t len = std::strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }

    return std::string(buffer);
}

void FileHandle::write_line(const std::string& data)
{
    check_open();

    if (std::fprintf(file_, "%s\n", data.c_str()) < 0)
    {
        throw ResourceError("Failed to write to file: " + filename_);
    }

    std::fflush(file_);
}

}
