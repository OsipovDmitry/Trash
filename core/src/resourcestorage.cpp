#include <assert.h>

#include "resourcestorage.h"

void ResourceStorage::store(const std::string& key, std::shared_ptr<ResourceStorage::Object> value)
{
    auto iter = m_storage.find(key);

    if (iter != m_storage.end())
    {
        auto weak = iter->second;
        if (!weak.expired())
            assert(weak.lock() == value);
    }

    m_storage[key] = value;
}

std::shared_ptr<ResourceStorage::Object> ResourceStorage::get(const std::string& key) const
{
    auto iter = m_storage.find(key);

    if (iter == m_storage.end())
        return nullptr;

    auto weak = iter->second;
    return weak.expired() ? nullptr : weak.lock();
}

ResourceStorage::ResourceStorage()
{
}
