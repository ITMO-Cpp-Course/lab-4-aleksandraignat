#include "resource_core/resource_manager.hpp"

namespace lab4::resource
{

ResourceManager& ResourceManager::instance()
{
    static ResourceManager manager;
    return manager;
}

std::shared_ptr<FileHandle> ResourceManager::get_file(const std::string& filename, const std::string& mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    CacheKey key{filename, mode};
    auto it = cache_.find(key);
    
    if (it != cache_.end())
    {
        auto shared = it->second.lock();
        
        if (shared)
        {
            return shared;
        }
        else
        {
            cache_.erase(it);
        }
    }
    
    auto file = std::make_shared<FileHandle>(filename, mode);
    cache_[key] = file;
    
    return file;
}

void ResourceManager::evict(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto it = cache_.begin(); it != cache_.end(); )
    {
        if (it->first.filename == filename)
        {
            it = cache_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ResourceManager::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
}

}
