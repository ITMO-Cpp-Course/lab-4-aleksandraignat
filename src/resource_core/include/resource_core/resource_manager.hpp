#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "resource_core/file_handle.hpp"

namespace lab4::resource
{

class ResourceManager
{
  public:
    static ResourceManager& instance();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    std::shared_ptr<FileHandle> get_file(const std::string& filename, const std::string& mode = "r");

    void evict(const std::string& filename);

    void clear();

  private:
    ResourceManager() = default;

    std::unordered_map<std::string, std::weak_ptr<FileHandle>> cache_;

    std::mutex mutex_;
};

} // namespace lab4::resource
