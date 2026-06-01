#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "resource_core/file_handle.hpp"

namespace lab4::resource
{

struct CacheKey {
    std::string filename;
    std::string mode;
    
    bool operator==(const CacheKey& other) const {
        return filename == other.filename && mode == other.mode;
    }
};

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

    std::unordered_map<CacheKey, std::weak_ptr<FileHandle>> cache_;

    std::mutex mutex_;
};

}

namespace std
{
    template<>
    struct hash<lab4::resource::CacheKey> {
        size_t operator()(const lab4::resource::CacheKey& key) const {
            return hash<string>()(key.filename) ^ (hash<string>()(key.mode) << 1);
        }
    };
}
