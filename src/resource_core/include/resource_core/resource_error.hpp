#pragma once

#include <stdexcept>
#include <string>

namespace lab4::resource
{

class ResourceError : public std::runtime_error
{
  public:
    explicit ResourceError(const std::string& message) : std::runtime_error(message) {}

    explicit ResourceError(const char* message) : std::runtime_error(message) {}
};

} // namespace lab4::resource
